#include "am_rx.hpp"

void am_rx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_xpams_rx, //output
    axis_t &axis_net, //input
    dataMoverCommand_t &axis_s2mmCommand, //output
    axis_t &axis_s2mm, //output
    dataMoverStatus_t &axis_s2mmStatus, //input
    uint_1_t &release //output
){
    #pragma HLS INTERFACE axis port=axis_xpams_rx
    #pragma HLS INTERFACE axis port=axis_net
    #pragma HLS INTERFACE axis port=axis_s2mmCommand
	#pragma HLS INTERFACE axis port=axis_s2mm
	#pragma HLS INTERFACE axis port=axis_s2mmStatus
    #ifdef DEBUG
    #pragma HLS INTERFACE ap_none port=dbg_currentState
    #endif
    #pragma HLS INTERFACE ap_none port=release
    // #pragma HLS INTERFACE ap_none port=nextState
    // #pragma HLS INTERFACE ap_none port=nextVectorCount
	#pragma HLS INTERFACE ap_ctrl_none port=return

    #pragma HLS PIPELINE

    // axis_word_t axis_word;
    // axis_word_8a_t axis_word_s2mmStatus;
    // axis_wordDest_t axis_wordDest;

    static gc_AMsrc_t AMsrc;
    static gc_AMdst_t AMdst;
    static gc_AMToken_t AMToken;
    static gc_AMtype_t AMtype;
    static gc_AMargs_t AMargs;
    static gc_AMhandler_t AMhandler;
    static gc_AMdest_t AMdest;

    static gc_payloadSize_t AMpayloadSize;
    static gc_dstVectorNum_t AMdstVectorNum;

    static gc_destination_t AMdestination;
    static gc_strideBlockSize_t AMstrideBlockSize;
    static gc_strideBlockNum_t AMstrideBlockNum;
    static gc_stride_t AMstride;

    static gc_vectorSize_t AMvectorSize[MAX_VECTOR_NUM];
    // static gc_destination_t AMvectorDest[MAX_VECTOR_NUM];

    static uint_1_t bufferRelease = 1;
    static gc_strideBlockNum_t status_count = 0;
    static gc_strideBlockNum_t status_counter = 0;
    static gc_strideBlockNum_t strideCount = 1;
    static gc_destination_t strideDest = -1;
    static gc_payloadSize_t writeCount = 0;
    static gc_dstVectorNum_t vectorCount = 0;
    static gc_AMargs_t argCounter = 1;
    static gc_dstVectorNum_t dstVectorNum_counter = 1;

    // static state_t nextState = st_header;
    static state_t currentState = st_header;
    // #pragma HLS DEPENDENCE variable=currentState inter false
    // #pragma HLS DEPENDENCE variable=vectorCount inter false
    switch(currentState){
        case st_header:{
            #pragma HLS PIPELINE
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            AMsrc = axis_word.data(AM_SRC);
            AMdst = axis_word.data(AM_DST);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize = axis_word.data(AM_PAYLOAD_SIZE) - GC_DATA_BYTES;
            #else
            AMpayloadSize = axis_word.data(AM_PAYLOAD_SIZE);
            #endif
            AMhandler = axis_word.data(AM_HANDLER);
            AMtype = axis_word.data(AM_TYPE);
            AMargs = axis_word.data(AM_HANDLER_ARGS);
            if (isReplyAM(AMtype) && (!isShortAM(AMtype))){
                // // forward all read words straight to xpams_rx for data requests
                // do {
                //     #pragma HLS loop_tripcount min=2 max=289 avg=10
                //     #pragma HLS PIPELINE
                    axis_xpams_rx.write(axis_word);
                //     axis_net.read(axis_word);
                // } while(axis_word.last != 1);
                // axis_xpams_rx.write(axis_word);
                // currentState = st_header;
                currentState = st_AMforward;
            }
            else{
                bufferRelease = !isLongxAM(AMtype);
                axis_xpams_rx.write(axis_word);
                if(isLongStridedAM(AMtype)){
                    currentState = st_AMLongStride;
                }
                else if(isLongVectoredAM(AMtype)){
                    currentState = st_AMLongVector;
                }
                else{
                    currentState = st_AMToken;
                }
            }
            break;
        }
        case st_AMforward:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false

            axis_net.read(axis_word);
            axis_xpams_rx.write(axis_word);
            
            if(axis_word.last == 1){
                currentState = st_header;
            } else {
                currentState = st_AMforward;
            }
            break;
        }
        case st_AMToken:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            AMToken = axis_word.data(AM_TOKEN);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            if(isShortAM(AMtype)){
                currentState = AMargs == 0 ? st_header : st_AMHandlerArgs;
            }
            else if(isMediumAM(AMtype)){
                currentState = AMargs == 0 ? st_AMpayloadMedium : st_AMHandlerArgs;
            }
            else{
                currentState = st_AMdestination;
            }

            if((isShortAM(AMtype) || isLongxAM(AMtype)) && AMargs == 0){
                axis_word.last = 1;
            }
            axis_xpams_rx.write(axis_word);
            break;
        }
        case st_AMHandlerArgs:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            bool last_write = argCounter == AMargs;
            bool assert_last = isShortAM(AMtype) || isLongxAM(AMtype);
            
            axis_net.read(axis_word);
            axis_word.last = last_write && assert_last;
            axis_xpams_rx.write(axis_word);
            argCounter++;
            if(last_write){
                if(isShortAM(AMtype)){
                    currentState = st_header;
                } else if(isMediumAM(AMtype)){
                    currentState = st_AMpayloadMedium;
                } else if(isLongAM(AMtype)){
                    currentState = st_AMpayloadLong;
                } else if(isLongStridedAM(AMtype)){
                    currentState = st_AMpayloadStride;
                } else {
                    currentState = st_AMpayloadVector;
                }
            }
            break;
        }
        // case st_AMHandlerArgs:{
        //     // axis_word_t axis_word;
        //     // gc_AMargs_t argCount;
        //     // for(argCount = 0; argCount < AMargs - 1; argCount++){
        //     //     #pragma HLS loop_tripcount min=1 max=254 avg=1
        //     //     axis_net.read(axis_word);
        //     //     axis_xpams_rx.write(axis_word);
        //     //     #ifdef USE_ABS_PAYLOAD
        //     //     AMpayloadSize-=GC_DATA_BYTES;
        //     //     #endif
        //     // }
        //     // axis_net.read(axis_word);
        //     // if(isShortAM(AMtype) || isLongxAM(AMtype)){
        //     //     axis_word.last = 1;
        //     // }
        //     // axis_xpams_rx.write(axis_word);
        //     // #ifdef USE_ABS_PAYLOAD
        //     // AMpayloadSize-=GC_DATA_BYTES;
        //     // #endif
        //     // currentState = isShortAM(AMtype) ? st_header : st_AMpayload;
        //     if(isShortAM(AMtype)){
        //         currentState = st_header;
        //     } else if(isMediumAM(AMtype)){
        //         currentState = st_AMpayloadMedium;
        //     } else if(isLongAM(AMtype)){
        //         currentState = st_AMpayloadLong;
        //     } else if(isLongStridedAM(AMtype)){
        //         currentState = st_AMpayloadStride;
        //     } else {
        //         currentState = st_AMpayloadVector;
        //     }
        //     break;
        // }
        case st_AMdestination:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            AMdestination = axis_word.data;
            strideDest = axis_word.data + AMstride;
            // axis_xpams_rx.write(axis_word);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            gc_strideBlockSize_t payload = isLongAM(AMtype) ? (gc_strideBlockSize_t)(AMpayloadSize - AMargs*GC_DATA_BYTES) : (gc_strideBlockSize_t)(AMstrideBlockSize);
            #else
            gc_strideBlockSize_t payload = isLongAM(AMtype) ? AMpayloadSize : AMstrideBlockSize;
            #endif
            addr_word_t address = axis_word.data(GC_ADDR_WIDTH-1,0);
            dataMoverWriteCommand(axis_s2mmCommand, 0, 0, address,
                address(1,0) != 0, 1, address(1,0), 1, payload);
            // currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
            if(AMargs != 0){
                currentState = st_AMHandlerArgs;
            } else if(isMediumAM(AMtype)){
                currentState = st_AMpayloadMedium;
            } else if(isLongAM(AMtype)){
                currentState = st_AMpayloadLong;
            } else if(isLongStridedAM(AMtype)){
                currentState = st_AMpayloadStride;
            } else {
                currentState = st_AMpayloadVector;
            }
            break;
        }
        case st_AMLongStride:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            axis_word.last = AMargs == 0;
            axis_xpams_rx.write(axis_word);
            AMstride = axis_word.data(AM_STRIDE_SIZE);
            AMstrideBlockSize = axis_word.data(AM_STRIDE_BLK_SIZE);
            AMstrideBlockNum = axis_word.data(AM_STRIDE_BLK_NUM);
            AMToken = axis_word.data(AM_TOKEN);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            currentState = st_AMdestination;
            break;
        }
        case st_AMLongVector:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            AMdstVectorNum = axis_word.data(AM_DST_VECTOR_NUM);
            AMvectorSize[0] = axis_word.data(AM_DST_VECTOR_SIZE_HEAD);
            AMToken = axis_word.data(AM_TOKEN);
            status_count = axis_word.data(AM_DST_VECTOR_NUM);
            axis_word.last = AMargs == 0;
            axis_xpams_rx.write(axis_word);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            currentState = st_AMLongVector_2;
            break;

        }
        case st_AMLongVector_2:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            gc_destination_t AMvectorDest;
            axis_net.read(axis_word);
            AMvectorDest = axis_word.data;
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            dataMoverWriteCommand(axis_s2mmCommand, 0, 0,
                AMvectorDest(GC_ADDR_WIDTH-1,0),
                AMvectorDest(1,0) != 0, 1,
                AMvectorDest(1,0), 1, AMvectorSize[0]);

            // gc_dstVectorNum_t i;
            // for(i = 1; i < AMdstVectorNum; i++){
            //     #pragma HLS loop_tripcount min=0 max=16 avg=2
            //     axis_net.read(axis_word);
            //     AMvectorSize[i] = axis_word.data(AM_DST_VECTOR_SIZE_BODY);
            //     #ifdef USE_ABS_PAYLOAD
            //     AMpayloadSize-=GC_DATA_BYTES;
            //     #endif

            //     axis_net.read(axis_word);
            //     AMvectorDest[i] = axis_word.data;
            //     #ifdef USE_ABS_PAYLOAD
            //     AMpayloadSize-=GC_DATA_BYTES;
            //     #endif

            //     dataMoverWriteCommand(axis_s2mmCommand, 0, 0,
            //         AMvectorDest[i](GC_ADDR_WIDTH-1,0),
            //         AMvectorDest[i](1,0) != 0, 1,
            //         AMvectorDest[i](1,0), 1, AMvectorSize[i]);
            // }
            if(AMdstVectorNum > 1){
                currentState = st_AMLongVectorRead;
            } else {
                currentState = AMargs == 0 ? st_AMpayloadVector : st_AMHandlerArgs;
            }
            break;
        }
        case st_AMLongVectorRead:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            AMvectorSize[dstVectorNum_counter] = axis_word.data(AM_DST_VECTOR_SIZE_BODY);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            currentState = st_AMLongVectorRead_2;
            break;
        }
        case st_AMLongVectorRead_2:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            gc_destination_t AMvectorDest;
            axis_net.read(axis_word);
            AMvectorDest = axis_word.data;
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif

            dataMoverWriteCommand(axis_s2mmCommand, 0, 0,
                AMvectorDest(GC_ADDR_WIDTH-1,0),
                AMvectorDest(1,0) != 0, 1,
                AMvectorDest(1,0), 1, AMvectorSize[dstVectorNum_counter]);
            
            dstVectorNum_counter++;
            if(dstVectorNum_counter == AMdstVectorNum){
                currentState = AMargs == 0 ? st_AMpayloadVector : st_AMHandlerArgs;
            }
            break;
        }
        case st_AMpayloadMedium:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            axis_xpams_rx.write(axis_word);
            if(axis_word.last){
                currentState = st_header;
            }
            break;
        }
        case st_AMpayloadLong:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            axis_s2mm.write(axis_word);
            if(axis_word.last){
                currentState = st_done;
            }
            status_count = 1;
            break;
        }
        case st_AMpayloadStride:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            bool is_last = axis_word.last;
            writeCount += GC_DATA_BYTES;
            if (strideCount < AMstrideBlockNum){
                dataMoverWriteCommand(axis_s2mmCommand, 0, 0,
                    strideDest, strideDest(1,0) != 0, 1,
                    strideDest(1,0), 1, AMstrideBlockSize);
                strideDest += AMstride;
                strideCount++;
            }
            if(writeCount < AMstrideBlockSize){
                axis_word.last = 0;
                axis_s2mm.write(axis_word);
            }
            else if(writeCount == AMstrideBlockSize){
                axis_word.last = 1;
                axis_s2mm.write(axis_word);
            }
            else{
                // ap_uint<GC_DATA_BYTES> keep = writeCount - AMstrideBlockSize;
                // switch (keep){
                // case 1:
                //     keep = power<2, 1>()-1;
                //     break;
                // case 2:
                //     keep = power<2, 2>()-1;
                //     break;
                // case 3:
                //     keep = power<2, 3>()-1;
                //     break;
                // case 4:
                //     keep = power<2, 4>()-1;
                //     break;
                // case 5:
                //     keep = power<2, 5>()-1;
                //     break;
                // case 6:
                //     keep = power<2, 6>()-1;
                //     break;
                // case 7:
                //     keep = power<2, 7>()-1;
                //     break;
                // default:
                //     keep = GC_DATA_TKEEP;
                //     break;
                // }
                    axis_word.last = 1;
                    axis_word.keep = GC_DATA_TKEEP;
                    axis_s2mm.write(axis_word);
                // }
                // writeCount = 1;
                writeCount = 0;
            }
            status_count = AMstrideBlockNum;
            if(is_last){
                currentState = st_done;
            }
            break;
        }
        case st_AMpayloadVector:{
            axis_word_t axis_word;
            // #pragma HLS DEPENDENCE variable=axis_word inter false
            axis_net.read(axis_word);
            bool is_last = axis_word.last;
            writeCount += GC_DATA_BYTES;
            if(writeCount < AMvectorSize[vectorCount]){
                axis_word.last = 0;
                axis_s2mm.write(axis_word);
            }
            else{
                axis_word.last = 1;
                axis_s2mm.write(axis_word);
                vectorCount++;
                writeCount = 0;
            }
            if(is_last){
                currentState = st_done;
            }
            break;
        }
        // case st_AMpayload:{
        //     axis_word_t axis_word;
        //     gc_payloadSize_t i = 0;
        //     gc_payloadSize_t writeCount = 0;
        //     gc_destination_t strideDest = AMdestination + AMstride;
        //     gc_strideBlockNum_t strideCount = 1;
        //     gc_dstVectorNum_t vectorCount = 0;
        //     AMpayloadSize = AMpayloadSize % GC_DATA_BYTES == 0 ? AMpayloadSize / GC_DATA_BYTES : (gc_payloadSize_t)((AMpayloadSize / GC_DATA_BYTES) + 1);
        //     while(i < AMpayloadSize){
        //         #pragma HLS loop_tripcount min=1 max=65536 avg=2
        //         axis_net.read(axis_word);
        //         writeCount+=GC_DATA_BYTES;
        //         i++;
        //         if(isMediumAM(AMtype)){
        //             axis_xpams_rx.write(axis_word);
        //         }
        //         else{
        //             if(isLongAM(AMtype)){
        //                 axis_s2mm.write(axis_word);
        //             }
        //             else if(isLongStridedAM(AMtype)){
        //                 if (strideCount < AMstrideBlockNum){
        //                     dataMoverWriteCommand(axis_s2mmCommand, 0, 0,
        //                         strideDest, strideDest(1,0) != 0, 1,
        //                         strideDest(1,0), 1, AMstrideBlockSize);
        //                     strideDest += AMstride;
        //                     strideCount++;
        //                 }
        //                 if(writeCount < AMstrideBlockSize){
        //                     // if(!axis_s2mm.full()){
        //                         axis_word.last = 0;
        //                         axis_s2mm.write(axis_word);
        //                     // }
        //                 }
        //                 else if(writeCount == AMstrideBlockSize){
        //                     // if(!axis_s2mm.full()){
        //                         axis_word.last = 1;
        //                         axis_s2mm.write(axis_word);
        //                     // }
        //                     // if(i < AMpayloadSize){

        //                     // }
        //                 }
        //                 else{
        //                     // dataMoverWriteCommand(axis_s2mmCommand, 0, 0,
        //                     //     strideDest, strideDest(1,0) != 0, 1,
        //                     //     strideDest(1,0), 1, AMstrideBlockSize * GC_DATA_BYTES);
        //                     // strideDest += AMstride;
        //                     // if(!axis_s2mm.full()){
        //                     ap_uint<GC_DATA_BYTES> keep = writeCount - AMstrideBlockSize;
        //                     switch (keep){
        //                     case 1:
        //                         keep = power<2, 1>()-1;
        //                         break;
        //                     case 2:
        //                         keep = power<2, 2>()-1;
        //                         break;
        //                     case 3:
        //                         keep = power<2, 3>()-1;
        //                         break;
        //                     case 4:
        //                         keep = power<2, 4>()-1;
        //                         break;
        //                     case 5:
        //                         keep = power<2, 5>()-1;
        //                         break;
        //                     case 6:
        //                         keep = power<2, 6>()-1;
        //                         break;
        //                     case 7:
        //                         keep = power<2, 7>()-1;
        //                         break;
        //                     default:
        //                         keep = GC_DATA_TKEEP;
        //                         break;
        //                     }
        //                     // if (AMstrideBlockSize == 1){
        //                         axis_word.last = 1;
        //                     // }
        //                     // else{
        //                     //     axis_word.last = 0;
        //                     // }
        //                         axis_word.keep = keep;
        //                         axis_s2mm.write(axis_word);
        //                     // }
        //                     // writeCount = 1;
        //                     writeCount = 0;
        //                 }
        //             }
        //             else{
        //                 if(writeCount < AMvectorSize[vectorCount]){
        //                     // if(!axis_s2mm.full()){
        //                         axis_word.last = 0;
        //                         axis_s2mm.write(axis_word);
        //                     // }
        //                 }
        //                 // else if(writeCount == AMvectorSize[vectorCount]){
        //                 else{
        //                     // if(!axis_s2mm.full()){
        //                         axis_word.last = 1;
        //                         axis_s2mm.write(axis_word);
        //                         vectorCount++;
        //                         writeCount = 0;
        //                     // }
        //                 }
        //                 // else{
        //                 //     vectorCount++;
        //                 //     // dataMoverWriteCommand(axis_s2mmCommand, 0, 0,
        //                 //     //     AMvectorDest[vectorCount], //address
        //                 //     //     AMvectorDest[vectorCount](1,0) != 0, //ddr
        //                 //     //     1, //eof
        //                 //     //     AMvectorDest[vectorCount](1,0), 1, //dsa, type
        //                 //     //     AMvectorSize[vectorCount]*GC_DATA_BYTES);
        //                 //     // if(!axis_s2mm.full()){
        //                 //         axis_word.last = 0;
        //                 //         axis_s2mm.write(axis_word);
        //                 //     // }
        //                 //     writeCount = 8;
        //                 // }
        //             }
        //         }
        //     }
        //     currentState = st_done;
        //     break;
        // }
        case st_done:{
            axis_word_8a_t axis_word_s2mmStatus;
            // if(isShortAM(AMtype) && AMargs == 0){
            //     currentState = st_header;
            // }
            // else if(isLongxAM(AMtype)){
                // if(axis_s2mmStatus.empty()){
                //     currentState = st_done;
                // }
                // else{
                // int j = 1;
                // if (isLongStridedAM(AMtype)){
                //     j = AMstrideBlockNum;
                // }
                // else if (isLongVectoredAM(AMtype)){
                //     j = AMdstVectorNum;
                // }
                // for(int i = 0; i < j; i++){
                //     #pragma HLS PIPELINE
                    //? s2mm doesn't seem to send a status in behav sim
                    //? to prevent hanging in simulation, use read_nb though it should 
                    //? be blocking
                    axis_s2mmStatus.read(axis_word_s2mmStatus);
                    status_counter++;
                // }
                if(status_counter >= status_count){
                    bufferRelease = 1;
                    currentState = st_header;
                }
                // }
            // }
            // else{
            //     currentState = st_header;
            // }
            break;
        }
    }

    // nextState = currentState;
    release = bufferRelease == 0 ? 0 : 1;

    // unconditionally read status to keep it empty
    // if(!axis_s2mmStatus.empty()){
    //     axis_s2mmStatus.read(axis_word_s2mmStatus);
    // }

    #ifdef DEBUG
    dbg_currentState = currentState;
    #endif

}

#ifdef DEBUG
std::string stateParse(int state){
    switch(state){
        CHECK_STATE("st_header", st_header, 0)
        CHECK_STATE("st_AMHandlerArgs", st_AMHandlerArgs, 1)
        CHECK_STATE("st_AMLongVector", st_AMLongVector, 2)
        CHECK_STATE("st_AMdestination", st_AMdestination, 3)
        CHECK_STATE("st_AMToken", st_AMToken, 4)
        CHECK_STATE("st_AMpayload", st_AMpayload, 5)
        CHECK_STATE("st_AMLongStride", st_AMLongStride, 6)
        CHECK_STATE("st_done", st_done, 7)
        default: return "Unknown State";
    }
}
#endif

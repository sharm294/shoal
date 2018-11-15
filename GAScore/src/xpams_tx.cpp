#include "xpams_tx.hpp"

void xpams_tx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_kernel_in, //input from kernel
    axis_dest_t &axis_kernel_out, //output to kernel
    axis_t &axis_tx, // output to am_tx (from kernel)

    axis_noKeep_t &axis_handler, //output to handler

    const gc_AMdest_t address_offset_low,
    const gc_AMdest_t address_offset_high
){
    #pragma HLS INTERFACE axis port=axis_kernel_in
    #pragma HLS INTERFACE axis port=axis_tx
    #pragma HLS INTERFACE axis port=axis_kernel_out
    #pragma HLS INTERFACE axis port=axis_handler
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_stable port=address_offset_high
    #pragma HLS INTERFACE ap_stable port=address_offset_low

    #ifdef DEBUG
    #pragma HLS INTERFACE ap_none port=dbg_currentState
    #endif
    
    axis_word_t axis_word;
    axis_wordDest_t axis_wordDest;
    axis_wordNoKeep_t axis_wordNoKeep;

    static gc_AMsrc_t AMsrc;
    static gc_AMdst_t AMdst;
    static gc_AMToken_t AMToken;

    static gc_payloadSize_t AMpayloadSize;

    static gc_AMargs_t AMargs;
    static gc_AMtype_t AMtype;
    static gc_AMhandler_t AMhandler;

    static bool loopback;

    switch(currentState){
        case st_AMheader:{
            if(!axis_kernel_in.empty()){
                axis_kernel_in.read(axis_word);
                AMsrc = axis_word.data(AM_SRC);
                AMhandler = axis_word.data(AM_HANDLER);
                AMargs = axis_word.data(AM_HANDLER_ARGS);
                AMdst = axis_word.data(AM_DST);
                AMtype = axis_word.data(AM_TYPE);
                AMpayloadSize = axis_word.data(AM_PAYLOAD_SIZE);
                loopback = AMdst < address_offset_high && AMdst >= address_offset_low;
                if (loopback){
                    if (AMhandler != H_EMPTY){
                        axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                        axis_handler.write(axis_wordNoKeep);
                    }
                    axis_kernel_in.read(axis_word); //read token
                    AMToken = axis_word.data(AM_TOKEN);
                    if (isMediumAM(AMtype)){
                        axis_wordDest = assignWord(axis_word);
                        axis_wordDest.dest = AMdst;
                        axis_kernel_out.write(axis_wordDest);
                        if (AMargs != 0){
                            gc_AMargs_t i;
                            for(i = 0; i < AMargs - 1; i++){
                                axis_kernel_in.read(axis_word);
                                axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                                axis_handler.write(axis_wordNoKeep);
                            }
                            axis_kernel_in.read(axis_word);
                            axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                            axis_wordNoKeep.last = 1;
                            axis_handler.write(axis_wordNoKeep);
                        }
                        if (AMpayloadSize > 0)
                            currentState = st_AMpayload;
                        else
                            currentState = st_reply;
                    }
                    else {
                        if (AMargs != 0){
                            gc_AMargs_t i;
                            for(i = 0; i < AMargs - 1; i++){
                                axis_kernel_in.read(axis_word);
                                axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                                axis_handler.write(axis_wordNoKeep);
                            }
                            axis_kernel_in.read(axis_word);
                            axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                            axis_wordNoKeep.last = 1;
                            axis_handler.write(axis_wordNoKeep);
                        }
                        currentState = st_reply;
                    }
                }
                else{
                    axis_tx.write(axis_word);
                    axis_kernel_in.read(axis_word); //read token
                    AMToken = axis_word.data(AM_TOKEN);
                    axis_tx.write(axis_word);
                    if (AMargs != 0){
                        gc_AMargs_t i;
                        for(i = 0; i < AMargs; i++){
                            axis_kernel_in.read(axis_word);
                            axis_tx.write(axis_word);
                        }
                    }
                    if (isMediumFIFOAM(AMtype))
                        currentState = st_AMsend;
                    else
                        currentState = st_AMheader;
                }
            }
            break;
        }
        case st_reply:{
            axis_word.data(AM_TYPE) = AM_SHORT + AM_REPLY;
            axis_word.data(AM_TOKEN) = AMToken;
            axis_word.keep = GC_DATA_TKEEP;
            axis_wordDest = assignWord(axis_word);
            axis_wordDest.dest = AMsrc;
            axis_kernel_out.write(axis_wordDest);
            currentState = st_AMheader;
            break;
        }
        case st_AMpayload:{
            gc_payloadSize_t i;
            for(i = 0; i < AMpayloadSize; i++){
                axis_kernel_in.read(axis_word);
                axis_wordDest = assignWord(axis_word);
                axis_wordDest.dest = AMdst;
                axis_kernel_out.write(axis_wordDest);
            }
            currentState = st_reply;
            break;
        }
        case st_AMsend:{
            axis_kernel_in.read(axis_word);
            while(axis_word.last != 1){
                axis_tx.write(axis_word);
                axis_kernel_in.read(axis_word);
            }
            currentState = st_AMheader;
            break;
        }
    }

    #ifdef DEBUG
    dbg_currentState = currentState;
    #endif
}

#ifdef DEBUG
std::string stateParse(int state){
    switch(state){
        CHECK_STATE("st_AMheader", st_AMheader, 0)
        CHECK_STATE("st_sendReplyHeader", st_reply, 1)
        CHECK_STATE("st_AMsend", st_AMsend, 2)
        CHECK_STATE("st_AMpayload", st_AMpayload, 3)
        default: return "Unknown State";
    }
}
#endif
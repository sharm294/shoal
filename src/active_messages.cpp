#include <cstddef>
#include "active_messages.hpp"

long long createHeader(
    int src,
    int dst,
    int payloadSize,
    int handlerID,
    int type,
    int handlerArgCount
){
    long long header = ((long long)src << AM_SRC_LOWER) + 
        ((long long)dst << AM_DST_LOWER) + 
        ((long long)payloadSize << AM_PAYLOAD_SIZE_LOWER) + 
        ((long long)handlerID << AM_HANDLER_LOWER) +
        ((long long)type << AM_TYPE_LOWER) + 
        ((long long)handlerArgCount << AM_HANDLER_ARGS_LOWER);
    return header;
}

long long createToken(
    int token
){
    long long word = (long long)token << AM_TOKEN_LOWER;
    return word;
}

long long createVectorToken(
    int srcVectorCount,
    int dstVectorCount,
    int srcSize1,
    int dstSize1,
    int token
){
    long long word = ((long long)srcVectorCount << AM_SRC_VECTOR_NUM_LOWER) +
        ((long long)dstVectorCount << AM_DST_VECTOR_NUM_LOWER) +
        ((long long)srcSize1 << AM_SRC_VECTOR_SIZE_HEAD_LOWER) +
        ((long long)dstSize1 << AM_DST_VECTOR_SIZE_HEAD_LOWER) +
        ((long long)token << AM_TOKEN_LOWER);
    return word;
}

long long createStridedToken(
    int stride,
    int blockSize,
    int blockNum,
    int token
){
    long long word = ((long long)stride << AM_STRIDE_SIZE_LOWER) +
        ((long long)blockSize << AM_STRIDE_BLK_SIZE_LOWER) +
        ((long long)blockNum << AM_STRIDE_BLK_NUM_LOWER) +
        ((long long)token << AM_TOKEN_LOWER);
    return word;
}

long long createStridedToken(
    int stride,
    int blockSize,
    int blockNum
){
    long long word = ((long long)stride << AM_STRIDE_SIZE_LOWER) +
        ((long long)blockSize << AM_STRIDE_BLK_SIZE_LOWER) +
        ((long long)blockNum << AM_STRIDE_BLK_NUM_LOWER);
    return word;
}

long long createStridedSrcBody(
    int size
){
    return ((long long)size << AM_SRC_VECTOR_SIZE_BODY_LOWER);
}

long long createStridedDstBody(
    int size
){
    return ((long long)size << AM_DST_VECTOR_SIZE_BODY_LOWER);
}

#if defined(__HLS__)

#include "../GAScore/include/utilities.hpp"

void writeWord(
    word_t data,
    bool last,
    bool keep,

){

}


inline axis_word_t createHeader(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    int type,
    gc_AMargs_t handlerArgCount
){
    axis_word_t axis_word;
    axis_word.data(AM_SRC) = src;
    axis_word.data(AM_DST) = dst;
    axis_word.data(AM_PAYLOAD_SIZE) = payloadSize;
    axis_word.data(AM_HANDLER) = handlerID;
    axis_word.data(AM_TYPE) = type;
    axis_word.data(AM_HANDLER_ARGS) = handlerArgCount;
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

inline axis_word_t createToken(
    gc_AMToken_t token,
    bool last
){
    axis_word_t axis_word;
    axis_word.data(AM_TOKEN) = token;
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

inline axis_word_t createStride(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
){
    axis_word_t axis_word;
    axis_word.data(AM_STRIDE_SIZE) = stride;
    axis_word.data(AM_STRIDE_BLK_SIZE) = stride_size;
    axis_word.data(AM_STRIDE_BLK_NUM) = stride_num;
    axis_word.data(AM_TOKEN) = token;
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

inline axis_word_t createStride(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
){
    axis_word_t axis_word;
    axis_word.data(AM_STRIDE_SIZE) = stride;
    axis_word.data(AM_STRIDE_BLK_SIZE) = stride_size;
    axis_word.data(AM_STRIDE_BLK_NUM) = stride_num;
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

inline void sendHandlerArgs(
    axis_t & axis_out,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount
){
    axis_word_t axis_word;
    for (int i = 0; i < handlerArgCount; i++){
        axis_word.data = *(handler_args+i);
        axis_word.last = i == handlerArgCount - 1;
        axis_word.keep = GC_DATA_TKEEP;
        axis_out.write(axis_word);;
    }
}

inline void sendPayloadArgs(
    axis_t & axis_out,
    word_t * payload_args,
    gc_payloadSize_t payloadArgCount
){
    axis_word_t axis_word;
    for (int i = 0; i < payloadArgCount; i++){
        axis_word.data = *(payload_args+i);
        axis_word.last = i == payloadArgCount - 1;
        axis_word.keep = GC_DATA_TKEEP;
        axis_out.write(axis_word);;
    }
}

void sendShortAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, 0, handlerID, AM_SHORT, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM|AM_FIFO, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
    sendPayloadArgs(axis_out, payload, payloadSize);
}

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = payloadSize;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

void sendlong longAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    word_t dst_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_long long|AM_FIFO, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
    sendPayloadArgs(axis_out, payload, payloadSize);
}

void sendlong longAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_long long, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

void sendlong longStridedAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_stride_t src_stride,
    gc_strideBlockSize_t src_blk_size,
    gc_strideBlockNum_t src_blk_num,
    word_t src_addr,
    gc_stride_t dst_stride,
    gc_strideBlockSize_t dst_blk_size,
    gc_strideBlockNum_t dst_blk_num,
    word_t dst_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_long long, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createStride(src_stride, src_blk_size, src_blk_num);
    axis_out.write(axis_word);
    axis_word.data = src_addr;
    axis_word.last = 0;
    axis_out.write(axis_word);
    axis_word = createStride(dst_stride, dst_blk_size, dst_blk_num, token);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

#elif defined(__x86_64__) || defined(__MICROBLAZE__)

#if defined(__MICROBLAZE__)
#include "fsl.h"

bool checkKernelEmpty(){
    long word_0;
    int return_val;
    getfslx(word_0, KERNEL_LINK, FSL_NONBLOCKING);
    fsl_isinvalid(return_val);
    return return_val;
}

#if defined(ENABLE_NETWORK)

bool checkNetEmpty(){
    long word_0;
    int return_val;
    getfslx(word_0, NET_LINK, FSL_NONBLOCKING);
    fsl_isinvalid(return_val);
    return return_val;
}

long long readNetwork(){
    long word_0, word_1;
    getfslx(word_0, NET_LINK, FSL_DEFAULT);
    getfslx(word_1, NET_LINK, FSL_DEFAULT);
    long long word = ((long long)word_1 << 32);
    word |= (long long)word_0;
    return word;
}

void writeNetwork (long long word, bool assertLast){
    int word_0 = word & 0xFFFFFFFF;
    int word_1 = (word >> 32) & 0xFFFFFFFF;
    putfslx(word_0, NET_LINK, FSL_DEFAULT);
    if (assertLast){
        putfslx(word_1, NET_LINK, FSL_CONTROL);
    } else {
        putfslx(word_1, NET_LINK, FSL_DEFAULT);
    }
}

#endif

long long readKernel(){
    long word_0, word_1;
    getfslx(word_0, KERNEL_LINK, FSL_DEFAULT);
    getfslx(word_1, KERNEL_LINK, FSL_DEFAULT);
    long long word = ((long long)word_1 << 32);
    word |= (long long)word_0;
    return word;
}

void writeKernel(long long word, bool assertLast){
    int word_0 = word & 0xFFFFFFFF;
    int word_1 = (word >> 32) & 0xFFFFFFFF;
    putfslx(word_0, KERNEL_LINK, FSL_DEFAULT);
    if (assertLast){
        putfslx(word_1, KERNEL_LINK, FSL_CONTROL);
    } else {
        putfslx(word_1, KERNEL_LINK, FSL_DEFAULT);
    }
}

void sendArgs(
    void * args,
    int argCount,
    bool assertLast
){
    long long word;
    int header_0;
    int header_1;
    int i;
    for (i = 0; i < argCount-1; i++){
        word = *((long long *)(args)+i);
        header_0 = word & 0xFFFFFFFF;
        header_1 = (word >> 32) & 0xFFFFFFFF;
        putfslx(header_0, KERNEL_LINK, FSL_DEFAULT);
        putfslx(header_1, KERNEL_LINK, FSL_DEFAULT);
    }
    word = *((long long *)(args)+i);
    header_0 = word & 0xFFFFFFFF;
    header_1 = (word >> 32) & 0xFFFFFFFF;
    putfslx(header_0, KERNEL_LINK, FSL_DEFAULT);
    if (assertLast){
        putfslx(header_1, KERNEL_LINK, FSL_CONTROL);
    } else {
        putfslx(header_1, KERNEL_LINK, FSL_DEFAULT);
    }
}

void writeReg(int address, int offset, int value){
    Xil_Out32(address + offset, value);
}

int readReg(int address, int offset){
    return Xil_In32(address + offset);
}

// #else // __x86_64__

// int AM_init(){
    
//     struct ifreq buffer;
//     int ifindex;

//     // open socket
//     if ((fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_PROTO))) < 0) {
//         printf("Error: could not open socket\n");
//         return -1;
//     }

//     // set local network interface index and MAC
//     memset(&buffer, 0x00, sizeof(buffer));
//     strncpy(buffer.ifr_name, ETH_NAME, IFNAMSIZ);
//     if (ioctl(fd, SIOCGIFINDEX, &buffer) < 0) {
//         printf("Error: could not get interface index\n");
//         close(fd);
//         return -1;
//     }
//     ifindex = buffer.ifr_ifindex;

//     if (ioctl(fd, SIOCGIFHWADDR, &buffer) < 0) {
//         printf("Error: could not get interface address\n");
//         close(fd);
//         return -1;
//     }

//     // save the interface's MAC address in global variable
//     memcpy((void*)source_mac, (void*)(buffer.ifr_hwaddr.sa_data), ETH_ALEN);

//     is_receiving = true;

//     eth_recv_thread = std::thread(eth_recv);
    
// }

// void eth_recv() {

//    	unsigned char buffer[ETH_FRAME_LEN];
// 	union AM_packet packet;

// 	int numbytes;

//    	while(is_receiving){
// 	    numbytes = eth_recv_packet(buffer,ETH_FRAME_LEN);

//    		if(numbytes != -1){
//    			std::lock_guard<std::mutex> guard(myMutex);
// 			// for(int i = 0; i < AM_WORD_SIZE; i++){
// 			// 	packet.field.am_header.buffer[i] = buffer[ETH_HLEN+i];
// 			// }
//             // for(int i = 0; i < AM_WORD_SIZE; i++){
// 			// 	packet.field.packet_id.buffer[i] = buffer[ETH_HLEN+AM_WORD_SIZE+i];
// 			// }

//             // if(numbytes > AM_PAYLOAD_SIZE)
//             //     numbytes = AM_PAYLOAD_SIZE;
//             // for(int i = 0; i < numbytes ;i++){
//             //     packet.field.payload[i] = buffer[ETH_HLEN + (2*AM_WORD_SIZE) + i]; 
//             // }
//             if(numbytes > ETH_FRAME_LEN){
//                 numbytes = ETH_FRAME_LEN;
//             }
//             for(int i = 0; i < numbytes ;i++){
//                 packet.buffer[i] = buffer[i]; 
//             }
//             data_list.push_back(packet);
// 		}
//    	}
// }

// int eth_recv_packet(unsigned char * buffer, int buffer_size){
//     struct ether_header *eh = (struct ether_header *) buffer;
//     int numbytes = recvfrom(fd, buffer, buffer_size, MSG_DONTWAIT, NULL, NULL);

//     if (eh->ether_dhost[0] == (source_mac[0] & 0xff) &&
// 		    eh->ether_dhost[1] == (source_mac[1] & 0xff) &&
// 		    eh->ether_dhost[2] == (source_mac[2] & 0xff) &&
// 		    eh->ether_dhost[3] == (source_mac[3] & 0xff) &&
// 		    eh->ether_dhost[4] == (source_mac[4] & 0xff) &&
// 		    eh->ether_dhost[5] == (source_mac[5] & 0xff)) {
//         return numbytes;
// 	} else {
//         return -1;
//     }
// }

// long long readKernel(){
//     long word_0, word_1;
//     getfslx(word_0, KERNEL_LINK, FSL_DEFAULT);
//     getfslx(word_1, KERNEL_LINK, FSL_DEFAULT);
//     long long word = ((long long)word_1 << 32);
//     word |= (long long)word_0;
//     return word;
// }

// void writeKernel(long long word, bool assertLast){
//     int word_0 = word & 0xFFFFFFFF;
//     int word_1 = (word >> 32) & 0xFFFFFFFF;
//     putfslx(word_0, KERNEL_LINK, FSL_DEFAULT);
//     if (assertLast){
//         putfslx(word_1, KERNEL_LINK, FSL_CONTROL);
//     } else {
//         putfslx(word_1, KERNEL_LINK, FSL_DEFAULT);
//     }
// }

// void sendArgs(
//     void * args,
//     int argCount,
//     bool assertLast
// ){
//     long long word;
//     int header_0;
//     int header_1;
//     int i;
//     for (i = 0; i < argCount-1; i++){
//         word = *((long long *)(args)+i);
//         header_0 = word & 0xFFFFFFFF;
//         header_1 = (word >> 32) & 0xFFFFFFFF;
//         putfslx(header_0, KERNEL_LINK, FSL_DEFAULT);
//         putfslx(header_1, KERNEL_LINK, FSL_DEFAULT);
//     }
//     word = *((long long *)(args)+i);
//     header_0 = word & 0xFFFFFFFF;
//     header_1 = (word >> 32) & 0xFFFFFFFF;
//     putfslx(header_0, KERNEL_LINK, FSL_DEFAULT);
//     if (assertLast){
//         putfslx(header_1, KERNEL_LINK, FSL_CONTROL);
//     } else {
//         putfslx(header_1, KERNEL_LINK, FSL_DEFAULT);
//     }
// }

// #endif

// void sendShortAM(
//     int src,
//     int dst,
//     int token,
//     int handlerID,
//     int handlerArgCount,
//     void * handler_args
// ){
//     long long header = createHeader(src, dst, 0, handlerID, AM_SHORT, handlerArgCount);
//     writeKernel(header, false);
//     header = createToken(token);
//     if (handlerArgCount  > 0){
//         writeKernel(header, false);
//         sendArgs(handler_args, handlerArgCount, true);
//     } else {
//         writeKernel(header, true);
//     }
// }

// void sendMediumAM(
//     int src,
//     int dst,
//     int token,
//     int handlerID,
//     int handlerArgCount,
//     void * handler_args,
//     int payloadSize,
//     void * payload
// ){
//     long long header = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM|AM_FIFO, handlerArgCount);
//     writeKernel(header, false);
//     header = createToken(token);
//     writeKernel(header, false);
//     if (handlerArgCount  > 0){
//         sendArgs(handler_args, handlerArgCount, false);
//     }
//     sendArgs(payload, payloadSize, true);
// }

// void sendMediumAM(
//     int src,
//     int dst,
//     int token,
//     int handlerID,
//     int handlerArgCount,
//     void * handler_args,
//     int payloadSize,
//     long long src_addr
// ){
//     long long header = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM, handlerArgCount);
//     writeKernel(header, false);
//     header = createToken(token);
//     writeKernel(header, false);
//     header = src_addr;
//     if (handlerArgCount > 0){
//         writeKernel(header, false);
//         sendArgs(handler_args, handlerArgCount, true);
//     } else {
//         writeKernel(header, true);
//     }
// }

// void sendLongAM(
//     int src,
//     int dst,
//     int token,
//     int handlerID,
//     int handlerArgCount,
//     void * handler_args,
//     int payloadSize,
//     void * payload,
//     long long dst_addr
// ){
//     long long word = createHeader(src, dst, payloadSize, handlerID, AM_LONG|AM_FIFO, handlerArgCount);
//     writeKernel(word, false);
//     word = createToken(token);
//     writeKernel(word, false);
//     writeKernel(dst_addr, false);
//     if (handlerArgCount > 0){
//         sendArgs(handler_args, handlerArgCount, false);
//     }
//     sendArgs(payload, payloadSize, true);
// }

// void sendLongAM(
//     int src,
//     int dst,
//     int token,
//     int handlerID,
//     int handlerArgCount,
//     void * handler_args,
//     int payloadSize,
//     long long src_addr,
//     long long dst_addr
// ){
//     long long word = createHeader(src, dst, payloadSize, handlerID, AM_LONG, handlerArgCount);
//     writeKernel(word, false);
//     word = createToken(token);
//     writeKernel(word, false);
//     writeKernel(src_addr, false);
//     if (handlerArgCount > 0){
//         writeKernel(dst_addr, false);
//         sendArgs(handler_args, handlerArgCount, true);
//     } else {
//         writeKernel(dst_addr, true);
//     }
// }

// void sendVectorAM(
//     int src,
//     int dst,
//     int token,
//     int handlerID,
//     int handlerArgCount,
//     void * handler_args,
//     int payloadSize,
//     int srcVectorCount,
//     int dstVectorCount,
//     int * srcSize,
//     int * dstSize,
//     long long * src_addr,
//     long long * dst_addr
// ){
//     long long word = createHeader(src, dst, payloadSize, handlerID, AM_VECTOR, handlerArgCount);
//     writeKernel(word, false);
//     word = createVectorToken(srcVectorCount, dstVectorCount, *srcSize, *dstSize, token);
//     writeKernel(word, false);
//     writeKernel(*src_addr, false);
//     writeKernel(*dst_addr, false);

//     if (srcVectorCount > 1){
//         for(int i = 1; i < srcVectorCount - 1; i++){
//             word = createStridedSrcBody(*(++srcSize));
//             writeKernel(word, false);
//             writeKernel(*(++src_addr), false);
//         }
//         word = createStridedSrcBody(*(++srcSize));
//         writeKernel(word, false);
//         if (dstVectorCount == 1 && handlerArgCount == 0){
//             writeKernel(*(++src_addr), true);
//         } else {
//             writeKernel(*(++src_addr), false);
//         }
//     }

//     if (dstVectorCount > 1){
//         for(int i = 1; i < dstVectorCount - 1; i++){
//             word = createStridedDstBody(*(++dstSize));
//             writeKernel(word, false);
//             writeKernel(*(++dst_addr), false);
//         }
//         word = createStridedSrcBody(*(++dstSize));
//         writeKernel(word, false);
//         if (handlerArgCount == 0){
//             writeKernel(*(++dst_addr), true);
//         } else {
//             writeKernel(*(++dst_addr), false);
//         }
//     }

//     if (handlerArgCount > 0){
//         sendArgs(handler_args, handlerArgCount, true);
//     }
// }

// void sendStridedAM(
//     int src,
//     int dst,
//     int token,
//     int handlerID,
//     int handlerArgCount,
//     void * handler_args,
//     int payloadSize,
//     int srcStride,
//     int srcBlockSize,
//     int srcBlockNum,
//     long long src_addr,
//     int dstStride,
//     int dstBlockSize,
//     int dstBlockNum,
//     long long dst_addr
// ){
//     long long word = createHeader(src, dst, payloadSize, handlerID, AM_STRIDE, handlerArgCount);
//     writeKernel(word, false);
//     word = createStridedToken(srcStride, srcBlockSize, srcBlockNum);
//     writeKernel(word, false);
//     writeKernel(src_addr, false);
//     word = createStridedToken(dstStride, dstBlockSize, dstBlockNum);
//     writeKernel(word, false);
//     if (handlerArgCount > 0){
//         writeKernel(dst_addr, false);
//         sendArgs(handler_args, handlerArgCount, true);
//     } else {
//         writeKernel(dst_addr, true);
//     }
// }

#endif // architecture

#endif

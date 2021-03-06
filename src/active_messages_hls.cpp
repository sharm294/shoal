#include "active_messages.hpp"
// #include "am_globals.hpp"

galapagos::stream_packet <word_t> createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createHeader(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createTokenBeat(
    gc_AMToken_t token,
    bool last
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createToken(token);
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createStrided(stride, stride_size, stride_num, token);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createStrided(stride, stride_size, stride_num);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createVectorBeat(
    gc_srcVectorNum_t srcVectorCount,
    gc_dstVectorNum_t dstVectorCount, 
    gc_vectorSize_t srcSize1,
    gc_vectorSize_t dstSize1,
    gc_AMToken_t token
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createVectorToken(srcVectorCount, dstVectorCount, srcSize1, dstSize1, token);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

// void printWord(const std::string& prefix, galapagos::stream_packet <word_t> axis_word){
//     std::stringstream ss;
//     ss << prefix <<
//     "Data: " << COLOR(Color::FG_GREEN, hex, axis_word.data) <<
//     " Keep: " << COLOR(Color::FG_GREEN, hex, axis_word.keep) <<
//     " Last: " << COLOR(Color::FG_GREEN, dec, axis_word.last) <<
//     " Dest: " << COLOR(Color::FG_GREEN, dec, axis_word.dest) << "\n";
//     std::cout << ss.str();
// }

void writeWord(
    galapagos::interface <word_t> & axis_out,
    galapagos::stream_packet <word_t> axis_word,
    gc_AMdst_t dst
){
    #pragma HLS INLINE
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    axis_out.write(axis_word);
}

void writeWord(
    galapagos::interface <word_t> & axis_out,
    word_t data,
    bool last,
    gc_AMdst_t dst,
    gc_keep_t keep
){
    #pragma HLS INLINE
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = data;
    axis_word.last = last;
    axis_word.keep = keep;
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    axis_out.write(axis_word);
}

void writeWord(
    galapagos::interface <word_t> & axis_out,
    word_t data,
    bool last,
    gc_AMdst_t dst
){
    #pragma HLS INLINE
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = data;
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    axis_out.write(axis_word);
}

void sendHandlerArgs(
    galapagos::interface <word_t> & axis_out,
    gc_AMdst_t dst,
    const word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
){
    #pragma HLS INLINE
    galapagos::stream_packet <word_t> axis_word;
    // axis_word.dest = dst;
    int i;
    for (i = 0; i < handlerArgCount-1; i++){
        // writeWord(axis_out, *(handler_args+i), 0, dst);
        writeWord(axis_out, handler_args[i], 0, dst);
        // axis_word.data = *(handler_args+i);
        // axis_word.last = 0;
        // axis_word.keep = GC_DATA_TKEEP;
        // axis_out.write(axis_word);
    }
    i++;
    writeWord(axis_out, handler_args[i], assertLast, dst);
    // axis_word.data = *(handler_args+i);
    // axis_word.last = assertLast;
    // axis_word.keep = GC_DATA_TKEEP;
    // axis_out.write(axis_word);;
}

void sendPayloadArgs(
    galapagos::interface <word_t> & axis_out,
    gc_AMdst_t dst,
    char * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.dest = dst;
    int i = GC_DATA_BYTES;
    for (i = GC_DATA_BYTES; i < payloadArgCount; i+=GC_DATA_BYTES){
        axis_word.data = *(payload_args+(i-GC_DATA_BYTES));
        axis_word.last = 0;
        axis_word.keep = GC_DATA_TKEEP;
        // printWord("   Sending - ", axis_word);
        axis_out.write(axis_word);
    }
    axis_word.data = *(word_t*)((payload_args+(i-GC_DATA_BYTES)));
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    // printWord("   Sending - ", axis_word);
    axis_out.write(axis_word);
}

void sendShortAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    galapagos::interface <word_t> & out
){
    // std::cout << "AM Short message from " << src << " to " << dst << "\n";
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, 0, handlerID, type, handlerArgCount);
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    out.write(axis_word);
    axis_word = createTokenBeat(token, handlerArgCount == 0);
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, true);
    }
}

void sendMediumAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    // const word_t * payload,
    galapagos::interface <word_t> & out
){
    // std::cout << "AM Medium message\n";
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, false);
    }
    // sendPayloadArgs(out, dst, (char*) payload, payloadSize, true);
}

void sendMediumAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    // const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    // const word_t * payload,
    galapagos::interface <word_t> & out
){
    // std::cout << "AM Medium message\n";
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    out.write(axis_word);
    // if (handlerArgCount > 0){
    //     sendHandlerArgs(out, dst, handler_args, handlerArgCount, false);
    // }
    // sendPayloadArgs(out, dst, (char*) payload, payloadSize, true);
}

void sendMediumAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    galapagos::interface <word_t> & out
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.dest = dst;
    out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_word.dest = dst;
    out.write(axis_word);
    axis_word.data = src_addr;
    axis_word.last = handlerArgCount == 0;
    axis_word.dest = dst;
    out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, true);
    }
}

void sendLongAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    // const word_t * payload,
    word_t dst_addr,
    galapagos::interface <word_t> & out
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.dest = dst;
    out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_word.dest = dst;
    out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = 0;
    axis_word.dest = dst;
    out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, false);
    }
    // sendPayloadArgs(out, dst, (char*) payload, payloadSize, true);
}

void sendLongAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr,
    galapagos::interface <word_t> & out
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.dest = dst;
    out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_word.dest = dst;
    out.write(axis_word);
    axis_word.data = src_addr;
    axis_word.dest = dst;
    out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_word.dest = dst;
    out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, true);
    }
}

void longStridedAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_stride_t src_stride,
    gc_strideBlockSize_t src_blk_size,
    gc_strideBlockNum_t src_blk_num,
    word_t src_addr,
    gc_stride_t dst_stride,
    gc_strideBlockSize_t dst_blk_size,
    gc_strideBlockNum_t dst_blk_num,
    word_t dst_addr,
    galapagos::interface <word_t> & out
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    writeWord(out, axis_word, dst);
    axis_word = createStridedBeat(src_stride, src_blk_size, src_blk_num);
    writeWord(out, axis_word, dst);
    writeWord(out, src_addr, 0, dst);
    axis_word = createStridedBeat(dst_stride, dst_blk_size, dst_blk_num, token);
    writeWord(out, axis_word, dst);
    writeWord(out, src_addr, handlerArgCount == 0, dst);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, true);
    }
}

void longVectorAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_srcVectorNum_t srcVectorCount,
    gc_dstVectorNum_t dstVectorCount,
    const gc_vectorSize_t * srcSize,
    const gc_vectorSize_t * dstSize,
    const word_t * src_addr,
    const word_t * dst_addr,
    galapagos::interface <word_t> & out
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    writeWord(out, axis_word, dst);
    
    axis_word = createVectorBeat(srcVectorCount, dstVectorCount, srcSize[0], dstSize[0], token);
    writeWord(out, axis_word, dst);

    writeWord(out, src_addr[0], 0, dst);
    writeWord(out, dst_addr[0], (srcVectorCount == 1) && (dstVectorCount == 1) && (handlerArgCount == 0), dst);

    int i;
    if (srcVectorCount > 1){
        for(i = 1; i < srcVectorCount-1; i++){
            writeWord(out, srcSize[i], 0, dst);
            writeWord(out, src_addr[i], 0, dst);
        }
        writeWord(out, srcSize[i], 0, dst);
        writeWord(out, src_addr[i], (dstVectorCount == 1) && (handlerArgCount == 0), dst);
    }

    if (dstVectorCount > 1){
        for(i = 1; i < dstVectorCount-1; i++){
            writeWord(out, dstSize[i], 0, dst);
            writeWord(out, dst_addr[i], 0, dst);
        }
        writeWord(out, dstSize[i], 0, dst);
        writeWord(out, dst_addr[i], handlerArgCount == 0, dst);
    }

    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, true);
    }
}
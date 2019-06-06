#if !defined(SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_)
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_

#include "config.hpp"
#include "hls_types.hpp"

#define CPU
#include "galapagos_node.hpp"

galapagos::stream_packet <word_t> createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
);

galapagos::stream_packet <word_t> createTokenBeat(
    gc_AMToken_t token,
    bool last
);

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
);

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
);

void printWord(const std::string& prefix, galapagos::stream_packet <word_t> axis_word);

inline void writeWord(
    galapagos::stream <word_t> & axis_out,
    galapagos::stream_packet <word_t> axis_word,
    gc_AMdst_t dst
);

inline void writeWord(
    galapagos::stream <word_t> & axis_out,
    word_t data,
    bool last, 
    gc_AMdst_t dst,
    gc_keep_t keep = GC_DATA_TKEEP
);

void sendHandlerArgs(
    galapagos::stream <word_t> & axis_out,
    gc_AMdst_t dst,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
);

void sendPayloadArgs(
    galapagos::stream <word_t> & axis_out,
    gc_AMdst_t dst,
    char * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
);

void sendShortAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    galapagos::stream <word_t> & out
);

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    galapagos::stream <word_t> & out
);

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    galapagos::stream <word_t> & out
);

void sendlongAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    word_t dst_addr,
    galapagos::stream <word_t> & out
);

void sendlongAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr,
    galapagos::stream <word_t> & out
);

void longStridedAM(
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
    galapagos::stream <word_t> & out
);

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_

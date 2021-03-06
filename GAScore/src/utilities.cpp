#include <cstddef> // needed to resolve ::max_align_t errors
#include "utilities.hpp"

bool isShortAM(gc_AMtype_t arg){
    return arg(2,0) == 0b001;
}

bool isMediumAM(gc_AMtype_t arg){
    return arg(2,0) == 0b010;
}

bool isLongxAM(gc_AMtype_t arg){
    return arg(2,2) == 1;
}

bool isLongAM(gc_AMtype_t arg){
    return arg(2,0) == 0b100;
}

bool isLongVectoredAM(gc_AMtype_t arg){
    return arg(2,0) == 0b110;
}

bool isLongStridedAM(gc_AMtype_t arg){
    return arg(2,0) == 0b101;
}

bool isDataFromFIFO(gc_AMtype_t arg){
    return arg[4] == 1;
}

bool isAsyncAM(gc_AMtype_t arg){
    return arg[5] == 1;
}

bool isReplyAM(gc_AMtype_t arg){
    return arg[6] == 1;
}

bool isMediumFIFOAM(gc_AMtype_t arg){
    return isMediumAM(arg) && isDataFromFIFO(arg);
}

bool isLongFIFOAM(gc_AMtype_t arg){
    return isLongAM(arg) && isDataFromFIFO(arg);
}

void dataMoverWriteCommand(
    dataMoverCommand_t &axis_command, //output
    uint_4_t reserved,
    uint_4_t tag,
    addr_word_t address,
    uint_1_t ddr,
    uint_1_t eof,
    uint_6_t dsa,
    uint_1_t type,
    btt_t btt
){
    #pragma HLS INLINE
    #define temp (32+GC_ADDR_WIDTH)
    dataMoverCommand_word_t axis_word_s2mmCommand;
    axis_word_s2mmCommand.data(temp+8-1,temp+4) = reserved;
    axis_word_s2mmCommand.data(temp+4-1,temp) = tag;
    axis_word_s2mmCommand.data(temp-1,32) = address;
    axis_word_s2mmCommand.data[31] = ddr;
    axis_word_s2mmCommand.data[30] = eof;
    axis_word_s2mmCommand.data(29,24) = dsa;
    axis_word_s2mmCommand.data[23] = type;
    axis_word_s2mmCommand.data(22,0) = btt;
    // axis_word_s2mmCommand.keep = GC_DATA_TKEEP;
    // if(!axis_command.full()){
        axis_command.write(axis_word_s2mmCommand);
    // }
    #undef temp
}

axis_wordNoKeep_t assignWordtoNoKeep(axis_word_t axis_word){
    #pragma HLS INLINE
    axis_wordNoKeep_t axis_wordDest;

    axis_wordDest.data = axis_word.data;
    axis_wordDest.last = axis_word.last;

    return axis_wordDest;
}

axis_wordDest_t assignWord(axis_word_t axis_word){
    #pragma HLS INLINE
    axis_wordDest_t axis_wordDest;

    axis_wordDest.data = axis_word.data;
    axis_wordDest.keep = axis_word.keep;
    axis_wordDest.last = axis_word.last;

    return axis_wordDest;
}

axis_wordNoKeep_t createHandlerHeader(gc_AMtype_t AMtype, gc_AMToken_t token, 
    gc_AMsrc_t dst, gc_payloadSize_t payload, gc_AMhandler_t handler, 
    gc_AMargs_t handler_args){
    #pragma HLS INLINE

    axis_wordNoKeep_t new_word;
    #pragma DEPENDENCE variable=new_word intra false

    new_word.data(AM_TYPE) = AMtype;
    new_word.data(AM_SRC) = token;
    new_word.data(AM_DST) = dst;
    new_word.data(AM_PAYLOAD_SIZE) = payload;
    new_word.data(AM_HANDLER) = handler;
    new_word.data(AM_HANDLER_ARGS) = handler_args;
    new_word.last = handler_args == 0;

    return new_word;
}

axis_wordDest_t createKernelHeader(gc_AMtype_t AMtype, gc_AMToken_t token, 
    gc_AMsrc_t src, gc_AMdst_t dst, gc_payloadSize_t payload, gc_AMhandler_t handler, 
    gc_AMargs_t handler_args){
    #pragma HLS INLINE

    axis_wordDest_t new_word;
    #pragma HLS DEPENDENCE variable=new_word intra false

    new_word.data(AM_TYPE) = AMtype;
    new_word.data(AM_SRC) = token;
    new_word.data(AM_DST) = src;
    new_word.data(AM_PAYLOAD_SIZE) = payload;
    new_word.data(AM_HANDLER) = handler;
    new_word.data(AM_HANDLER_ARGS) = handler_args;

    // new_word.last = 0;
    new_word.keep = GC_DATA_TKEEP;
    new_word.dest = dst;

    return new_word;
}

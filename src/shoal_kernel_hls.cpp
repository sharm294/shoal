#include <iostream>
#include <cstddef> // needed to resolve ::max_align_t errors

#ifndef __HLS__
#define __HLS__
#endif
#include "shoal_kernel.hpp"
#include "config.hpp"
#include "active_messages.hpp"
#include "handler_register_map.hpp"
#undef __HLS__

shoal::kernel::kernel(int id, int kernel_num, galapagos::interface<word_t> * in,
    galapagos::interface<word_t> * out, volatile int * handler_ctrl)
{
    this->id = id;
    this->kernel_num = kernel_num;
    this->in = in;
    this->out = out;
    this->handler_ctrl = handler_ctrl;
}

int shoal::kernel::get_id(){
    return this->id;
}

int shoal::kernel::init(){
    // SAFE_COUT("*** Initializing kernel ***\n");
	// mutex_nodedata = mutex_nodedata_global[this->id];
    return 0;
}

// void InternalBarrierUpdate(){
//     lock_guard_t lck(*mutex_nodedata);
// 	SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Updating barrier_cnt at " << nodedata << " to " << nodedata->barrier_cnt+1 << "\n"));
//     nodedata->barrier_cnt++;
// 	return;
// }

// void MemReadyBarrierUpdate(){
//     lock_guard_t lck(*mutex_nodedata);
// 	SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Updating mem_ready_barrier_cnt at " << nodedata << " to " << nodedata->mem_ready_barrier_cnt + 1 << "\n"));
//     nodedata->mem_ready_barrier_cnt++;
// 	return;
// }

// void counterUpdate(word_t arg){
//     lock_guard_t lck(*mutex_nodedata);
// 	SAFE_COUT("Updating counter at " << nodedata << " to " << nodedata->counter + arg << "\n");
//     nodedata->counter += arg;
// 	return;
// }

// void emptyHandler(){
// 	SAFE_COUT("Empty handler\n");
// 	return;
// }

// divide by 4 because int = 4 bytes
void shoal::kernel::wait_mem(unsigned int value){
    unsigned int read_value;
    do{
        read_value = *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_MEM_READY_OUT_V_DATA/4);
    } while(read_value < value);
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_ARG_V_DATA/4) = value;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA/4) = H_INCR_MEM | 0x10;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA/4) = H_INCR_MEM;
}

void shoal::kernel::wait_barrier(unsigned int value){
    unsigned int read_value;
    do{
        read_value = *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_BARRIER_OUT_V_DATA/4);
    } while(read_value < value);
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_ARG_V_DATA/4) = value;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA/4) = H_INCR_BAR | 0x10;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA/4) = H_INCR_BAR;
}

void shoal::kernel::sendShortAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args)
{
    // we have to add this here for some reason for the kernel to compile..?
    word_t tmp[16];
    int i = 0;
    for(i = 0; i < 16; i++){
        tmp[i] = handler_args[i];
    }
    sendShortAM(AM_SHORT, this->id, dst, token, handlerID, handlerArgCount,
        tmp, *(this->out));
}

void shoal::kernel::sendShortAM_async(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args)
{
    // we have to add this here for some reason for the kernel to compile..?
    word_t tmp[16];
    int i = 0;
    for(i = 0; i < 16; i++){
        tmp[i] = handler_args[i];
    }
    sendShortAM(AM_SHORT + AM_ASYNC, this->id, dst, token, handlerID,
        handlerArgCount, tmp, *(this->out));
}

void shoal::kernel::sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize)
{
    // we have to add this here for some reason for the kernel to compile..?
    word_t tmp[16];
    int i = 0;
    for(i = 0; i < 16; i++){
        tmp[i] = handler_args[i];
    }
    sendMediumAM(AM_MEDIUM|AM_FIFO, this->id, dst, token, handlerID, handlerArgCount,
        tmp, payloadSize, *(this->out));
}

void shoal::kernel::sendPayload(gc_AMdst_t dst, word_t payload, bool assertLast){
    #pragma HLS INLINE
    galapagos::stream_packet <word_t> axis_word;
    axis_word.dest = dst;
    axis_word.data = payload;
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    this->out->write(axis_word);
}

void shoal::kernel::sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t src_addr)
{
    // we have to add this here for some reason for the kernel to compile..?
    word_t tmp[16];
    int i = 0;
    for(i = 0; i < 16; i++){
        tmp[i] = handler_args[i];
    }
    sendMediumAM(AM_MEDIUM, this->id, dst, token, handlerID, handlerArgCount,
        tmp, payloadSize, src_addr, *(this->out));
}

void shoal::kernel::sendMediumAM_async(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize)
{
    // we have to add this here for some reason for the kernel to compile..?
    word_t tmp[16];
    int i = 0;
    for(i = 0; i < 16; i++){
        tmp[i] = handler_args[i];
    }
    sendMediumAM(AM_MEDIUM|AM_FIFO|AM_ASYNC, this->id, dst, token, handlerID, handlerArgCount,
        tmp, payloadSize, *(this->out));
}

void shoal::kernel::sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t dst_addr)
{
    // we have to add this here for some reason for the kernel to compile..?
    word_t tmp[16];
    int i = 0;
    for(i = 0; i < 16; i++){
        tmp[i] = handler_args[i];
    }
    sendLongAM(AM_LONG|AM_FIFO, this->id, dst, token, handlerID, handlerArgCount,
        tmp, payloadSize, dst_addr, *(this->out));
}

void shoal::kernel::sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t src_addr, word_t dst_addr)
{
    // we have to add this here for some reason for the kernel to compile..?
    word_t tmp[16];
    int i = 0;
    for(i = 0; i < 16; i++){
        tmp[i] = handler_args[i];
    }
    sendLongAM(AM_LONG, this->id, dst, token, handlerID, handlerArgCount,
        tmp, payloadSize, src_addr, dst_addr, *(this->out));
}

void shoal::kernel::sendMemUpdate(gc_AMdst_t dst){
    sendShortAM_normal(dst, 0, H_INCR_MEM, 0, nullptr);
}

void shoal::kernel::sendBarrierUpdate(gc_AMdst_t dst){
    sendShortAM_normal(dst, 0, H_INCR_BAR, 0, nullptr);
}

void shoal::kernel::barrier_wait(){
    this->wait_barrier(this->kernel_num-1);
    for(int i = 0; i < this->kernel_num; i++){
        if (i != this->id){
            ATOMIC_ACTION(this->sendBarrierUpdate(i));
        }
    }
    this->wait_reply(kernel_num-1);
}

void shoal::kernel::barrier_send(int id){
    #pragma HLS INLINE OFF
    ATOMIC_ACTION(this->sendBarrierUpdate(id));
    this->wait_barrier(1);
    this->wait_reply(1);
}

void shoal::kernel::wait_reply(unsigned int value){
    this->wait_mem(value);
}

void shoal::kernel::end(){
    // SAFE_COUT("Leaving kernel " << this->id << "\n");

    // free(gasnet_shared_mem);
    // *kernel_done[this->id] = true;
    // should free handlertable but it may segfault in the handler
}

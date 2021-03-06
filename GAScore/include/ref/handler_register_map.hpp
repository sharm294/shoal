// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2017.2
// Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
// 
// ==============================================================

// ctrl_bus
// 0x00 : reserved
// 0x04 : reserved
// 0x08 : reserved
// 0x0c : reserved
// 0x10 : Data signal of config_V
//        bit 1~0 - config_V[1:0] (Read/Write)
//        others  - reserved
// 0x14 : reserved
// 0x18 : Data signal of config_handler_V
//        bit 15~0 - config_handler_V[15:0] (Read/Write)
//        others   - reserved
// 0x1c : reserved
// 0x20 : Data signal of counter_threshold_V
//        bit 31~0 - counter_threshold_V[31:0] (Read/Write)
// 0x24 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/* 
 * This is an example register map file generated from Vivado HLS as part 
 * of making the handler IP (e.g. make hw-handler). It's included here to use if
 * you're not making the IP from scratch or if the default is broken. 
 * Since the names of variables may change between HLS versions, it cannot be 
 * generally pushed as part of this repository. This is provided as a reference.
 * 
 * To use, copy this file over to the parent include folder. The file with this 
 * name in the parent include folder is ignored in Git.
*/

#define XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA            0x10
#define XHANDLER_HANDLER_CTRL_BUS_BITS_CONFIG_V_DATA            2
#define XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_HANDLER_V_DATA    0x18
#define XHANDLER_HANDLER_CTRL_BUS_BITS_CONFIG_HANDLER_V_DATA    16
#define XHANDLER_HANDLER_CTRL_BUS_ADDR_COUNTER_THRESHOLD_V_DATA 0x20
#define XHANDLER_HANDLER_CTRL_BUS_BITS_COUNTER_THRESHOLD_V_DATA 32

#define XHANDLER_CONTROL_ADDR XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA
#define XHANDLER_HANDLER_ADDR XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_HANDLER_V_DATA
#define XHANDLER_THRESHOLD_ADDR XHANDLER_HANDLER_CTRL_BUS_ADDR_COUNTER_THRESHOLD_V_DATA

#define XHANDLER_LOCK 1
#define XHANDLER_RESET 2

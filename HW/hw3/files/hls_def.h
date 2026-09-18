#ifndef HLS_DEF_H_
#define HLS_DEF_H_

#include <systemc>
#include "grey_line_buffer.h"

// Grey Line Buffer (GLB)
#define GLB_IN     grey_line_buffer::in<>
#define GLB_OUT    grey_line_buffer::out<>
#define GLB_CHAN   grey_line_buffer::chan<>

// Testbench GLD in/out
#define GLB_TB_IN  grey_line_buffer::in<>
#define GLB_TB_OUT grey_line_buffer::out<>

// GLB API
#define GLB_GET(port, ws)   port.get(ws)
#define GLB_PUT(port, val)  port.put(val)
#define GLB_START(port)     port.start_tx()
#define GLB_NEXT_Y(port)    port.next_y()
#define GLB_END(port)       port.end_tx()
#define GLB_X_DONE(port)    port.x_done()
#define GLB_Y_DONE(port)    port.y_done()

#ifndef NATIVE_SYSTEMC
// HLS
#include "cynw_p2p.h"
#include "stratus_hls.h"

// DUT
#define HLS_IN(T) cynw_p2p<T>::in
#define HLS_OUT(T) cynw_p2p<T>::out
#define HLS_CHAN(T) cynw_p2p<T>::chan

// Testbench
#define HLS_TB_IN(T) cynw_p2p<T>::base_in
#define HLS_TB_OUT(T) cynw_p2p<T>::base_out

// read write 
#define HLS_READ(port, val) \
    { \
        HLS_DEFINE_PROTOCOL("input"); \
        val = port.get(); \
        wait(); \
    }

#define HLS_WRITE(port, val) \
    { \
        HLS_DEFINE_PROTOCOL("output"); \
        port.put(val); \
        wait(); \
    }

#define HLS_RESET_IN(port) port.reset()
#define HLS_RESET_OUT(port) port.reset()
#define HLS_RESET_BLOCK(stmts) \
    { \
        HLS_DEFINE_PROTOCOL("main_reset"); \
        stmts \
        wait(); \
    }

#define HLS_FLATTEN(arr) HLS_FLATTEN_ARRAY(arr)

#if defined(DPOPT_ALL)
    #define HLS_DPO(name) HLS_DPOPT_REGION(name)
#else
    #define HLS_DPO(name) 
#endif

#ifndef LAT
#define HLS_LAT_LOG(tag) HLS_CONSTRAIN_LATENCY(tag) // log only, no constraint
#else
#define HLS_LAT_LOG(tag) HLS_CONSTRAIN_LATENCY(0, LAT, tag)
#endif

#if defined(UNROLL)
    #define HLS_UNROLL(name) HLS_UNROLL_LOOP(ON, name)
#else
    #define HLS_UNROLL(name) 
#endif

#if defined(PIPELINE)
    #define HLS_PIPELINE(name) HLS_PIPELINE_LOOP(SOFT_STALL, II, name)
#else
    #define HLS_PIPELINE(name) 
#endif
#else
// SystemC

#define HLS_IN(T) sc_core::sc_fifo_in<T>
#define HLS_OUT(T) sc_core::sc_fifo_out<T>
#define HLS_TB_IN(T) sc_core::sc_fifo_in<T>
#define HLS_TB_OUT(T) sc_core::sc_fifo_out<T>
#define HLS_CHAN(T) sc_core::sc_fifo<T>

// read write 
#define HLS_READ(port, val) (val = port.read())
#define HLS_WRITE(port, val) (port.write(val))

// Reset 
#define HLS_RESET_IN(port) ((void)0)
#define HLS_RESET_OUT(port) ((void)0)
#define HLS_RESET_BLOCK(stmts) wait()

#define HLS_FLATTEN(arr) ((void)0)
#define HLS_DPO(name) ((void)0)
#define HLS_LAT_LOG(tag) ((void)0)
#define HLS_UNROLL(name) ((void)0)
#define HLS_PIPELINE(name) ((void)0) 

#endif

#endif // HLS_DEF_H_

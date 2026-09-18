#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

#ifndef SC_INCLUDE_FX
#define SC_INCLUDE_FX
#endif

#include <systemc>

typedef sc_dt::sc_fixed_fast<32, 16> ff_t;

const int SIGNAL_LENGTH = 500;

const int MOD_INPUT_ADDR = 0x00000000;
const int MOD_RESULT_ADDR = 0x00000004;

#endif // !FILTER_DEF_H_

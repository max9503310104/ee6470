#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <systemc>
#include "testbench.h"
#include "SimpleBus.h"
#ifndef NATIVE_SYSTEMC
#include "gauss_seidel_wrap.h"
#else
#include "gauss_seidel.h"
#endif

using namespace sc_core;

class System: public sc_module
{
public:
	SC_HAS_PROCESS(System);
	System(sc_module_name name);
	~System();
private:
    testbench tb;
	SimpleBus<1, 1> bus;
#ifndef NATIVE_SYSTEMC
	gauss_seidel_wrapper gauss_seidel_0;
#else
	gauss_seidel gauss_seidel_0;
#endif
	sc_clock clk;
	sc_signal<bool> rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<64> > a0;
    cynw_p2p< sc_dt::sc_uint<64> > a1;
    cynw_p2p< sc_dt::sc_uint<64> > a2;
    cynw_p2p< sc_dt::sc_uint<64> > a3;
	cynw_p2p< sc_dt::sc_uint<8> > n;
	cynw_p2p< sc_dt::sc_uint<64> > x;
#endif

};
#endif
#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <systemc>
#include "testbench.h"
#include "gauss_seidel_wrap.h"

using namespace sc_core;

class System: public sc_module
{
public:
	SC_HAS_PROCESS(System);
	System(sc_module_name name);
	~System();
private:
    testbench tb;
	gauss_seidel_wrapper gauss_seidel_0;

	sc_clock clk;
	sc_signal<bool> rst;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a0;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a1;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a2;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a3;
	cynw_p2p< sc_dt::sc_uint<8> > n;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > x;

};
#endif

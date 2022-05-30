#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <systemc>
#include "testbench.h"
#include "gauss_seidel_wrap.h"
#include "mac_wrap.h"

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
	mac_wrapper mac_0;

	sc_clock clk;
	sc_signal<bool> rst;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a0;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a1;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a2;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > a3;
	cynw_p2p< sc_dt::sc_uint<8> > n;
	cynw_p2p< sc_dt::sc_uint<1> > mode;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > x;
	
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > mul_a0;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > mul_a1;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > mul_a2;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > mul_a3;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> > mul_b;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 8> > mul_c;
};
#endif

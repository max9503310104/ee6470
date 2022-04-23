#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
#include <systemc>
#include <cynw_p2p.h>
#include "def.h"

using namespace std;
using namespace sc_core;

class testbench : public sc_module {
public:
	sc_in_clk i_clk;
	sc_out < bool >  o_rst;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::base_out o_a0;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::base_out o_a1;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::base_out o_a2;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::base_out o_a3;
	cynw_p2p< sc_dt::sc_uint<8> >::base_out o_n;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::base_in i_x;

	SC_HAS_PROCESS(testbench);
	testbench(sc_module_name n);
	~testbench();

private:
	void send_data();
	void receive_data();
	int dim;
	double x[128];
	double x_rcv[128];

	sc_time start_time;
};
#endif

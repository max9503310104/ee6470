#ifndef MAC_H_
#define MAC_H_

#include <systemc>
#include <cynw_p2p.h>
#include "def.h"

using namespace sc_core;

class mac: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool >  i_rst;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a0;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a1;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a2;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a3;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_b;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 8> >::out o_c;

	SC_HAS_PROCESS(mac);
	mac(sc_module_name n);
	~mac();
private:
	sc_dt::sc_biguint<(IL + FL) * 4> in_a0;
	sc_dt::sc_biguint<(IL + FL) * 4> in_a1;
	sc_dt::sc_biguint<(IL + FL) * 4> in_a2;
	sc_dt::sc_biguint<(IL + FL) * 4> in_a3;
	sc_dt::sc_biguint<(IL + FL) * 4> in_b;
	sc_dt::sc_biguint<(IL + FL) * 8> out_c;
	sc_dt::sc_int<IL + FL> a[4][4];
	sc_dt::sc_int<IL + FL> b[4];
	sc_dt::sc_int<(IL + FL) * 2> c[4];
	
	void compute();
};

#endif

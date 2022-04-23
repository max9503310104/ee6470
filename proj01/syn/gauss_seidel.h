#ifndef GAUSS_SEIDEL_H_
#define GAUSS_SEIDEL_H_

#include <systemc>
#include <cynw_p2p.h>
#include "def.h"

using namespace sc_core;

class gauss_seidel: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool >  i_rst;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a0;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a1;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a2;
    cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::in i_a3;
	cynw_p2p< sc_dt::sc_uint<8> >::in i_n;
	cynw_p2p< sc_dt::sc_biguint<(IL + FL) * 4> >::out o_x;

	SC_HAS_PROCESS(gauss_seidel);
	gauss_seidel(sc_module_name n);
	~gauss_seidel();
private:
	//sc_dt::sc_int<IL + FL> b[128], x[128];
	sc_dt::sc_int<IL + FL> b0[32], b1[32], b2[32], b3[32];
	sc_dt::sc_int<IL + FL> x0[32], x1[32], x2[32], x3[32];
	
	sc_dt::sc_biguint<(IL + FL) * 4> in[4];
    sc_dt::sc_int<IL + FL> in2[4][4];
    sc_dt::sc_uint<8> dim;
    sc_dt::sc_int<IL + FL> xp0[4];
    sc_dt::sc_int<(IL + FL) * 2> xp[4]; 
    sc_dt::sc_int<IL + FL> diag[4];
    sc_dt::sc_int<IL + FL> diag_x[4];
    sc_dt::sc_int<(IL + FL) * 2> row_result[4];
    sc_dt::sc_uint<8> col, row;
    sc_dt::sc_uint<8> iter;
	
	void compute();
};
#endif

#ifndef GAUSS_SEIDEL_H_
#define GAUSS_SEIDEL_H_

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include "def.h"
#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

using namespace sc_core;

class gauss_seidel: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool >  i_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<64> >::in i_a0;
    cynw_p2p< sc_dt::sc_uint<64> >::in i_a1;
    cynw_p2p< sc_dt::sc_uint<64> >::in i_a2;
    cynw_p2p< sc_dt::sc_uint<64> >::in i_a3;
	cynw_p2p< sc_dt::sc_uint<8> >::in i_n;
	cynw_p2p< sc_dt::sc_uint<64> >::out o_x;
#else
	sc_fifo< sc_dt::sc_uint<64> > i_a0;
    sc_fifo< sc_dt::sc_uint<64> > i_a1;
    sc_fifo< sc_dt::sc_uint<64> > i_a2;
    sc_fifo< sc_dt::sc_uint<64> > i_a3;
	sc_fifo< sc_dt::sc_uint<8> > i_n;
	sc_fifo< sc_dt::sc_uint<64> > o_x;
#endif

	tlm_utils::simple_target_socket<gauss_seidel> t_skt;

	SC_HAS_PROCESS(gauss_seidel);
	gauss_seidel(sc_module_name n);
	~gauss_seidel();
private:
	sc_dt::sc_int<IL + FL> b[128], x[128];
	void compute();
    void blocking_transport(tlm::tlm_generic_payload &payload,
                            sc_core::sc_time &delay);
};
#endif

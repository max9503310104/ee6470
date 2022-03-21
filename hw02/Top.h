#ifndef TOP_H
#define TOP_H
#include <systemc>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include "filter_def.h"

class Top : public sc_module {
public:
	tlm_utils::simple_target_socket<Top> t_skt;

	sc_fifo<unsigned char> i_r;
	sc_fifo<unsigned char> i_g;
	sc_fifo<unsigned char> i_b;
	sc_fifo<int> o_r;
	sc_fifo<int> o_g;
	sc_fifo<int> o_b;

	SC_HAS_PROCESS(Top);
	Top(sc_module_name name);

private:
	void do_filter();
	void do_buffer();
    void blocking_transport(tlm::tlm_generic_payload &payload,
	                          sc_core::sc_time &delay);
    
	unsigned int base_offset;

	int rowBuf1[3][256];
	int rowBuf2[3][256];
	int rowBuf3[3][256];

	int row, col;
	
	sc_event trig_filter;
};
#endif

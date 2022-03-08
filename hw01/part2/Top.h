#ifndef TOP_H
#define TOP_H
#include <systemc>
using namespace sc_core;

class Top : public sc_module {
public:
	//sc_in_clk i_clk;
	//sc_in<bool> i_rst;
	sc_fifo_in<unsigned char> i_r;
	sc_fifo_in<unsigned char> i_g;
	sc_fifo_in<unsigned char> i_b;
	sc_fifo_out<int> o_r;
	sc_fifo_out<int> o_g;
	sc_fifo_out<int> o_b;

	SC_HAS_PROCESS(Top);
	Top(sc_module_name name);

private:
	void do_filter();
	void do_buffer();

	int rowBuf1[3][256];
	int rowBuf2[3][256];
	int rowBuf3[3][256];

	int row, col;
	
	sc_event trig_filter;
};
#endif

#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <systemc>
using namespace sc_core;

#include "filter_def.h"

class Filter : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;
  sc_fifo_in<unsigned char> i_r;
  sc_fifo_in<unsigned char> i_g;
  sc_fifo_in<unsigned char> i_b;
  //sc_fifo_out<int> o_result;
  sc_fifo_out<int> r_flt;
  sc_fifo_out<int> g_flt;
  sc_fifo_out<int> b_flt;
  

  SC_HAS_PROCESS(Filter);
  Filter(sc_module_name n);
  ~Filter() = default;

private:
  void do_filter();
  //int val[MASK_N];
  int val[3];
};
#endif

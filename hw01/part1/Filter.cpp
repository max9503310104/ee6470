#include <cmath>

#include "Filter.h"

Filter::Filter(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

// sobel mask
const int mask[MASK_N][MASK_X][MASK_Y] = {{{1, 2, 1}, {2, 4, 2}, {1, 2, 1}}};

void Filter::do_filter() {
  while (true) {
    for (unsigned int i = 0; i < 3; ++i) {
      val[i] = 0;
    }
    for (unsigned int v = 0; v < MASK_Y; ++v) {
      for (unsigned int u = 0; u < MASK_X; ++u) {
        //unsigned char grey = (i_r.read() + i_g.read() + i_b.read()) / 3;
        //for (unsigned int i = 0; i != MASK_N; ++i) {
          val[0] += i_r.read() * mask[0][u][v];
          val[1] += i_g.read() * mask[0][u][v];
          val[2] += i_b.read() * mask[0][u][v];
        //}
      }
    }
    
	//for (unsigned int i = 0; i != MASK_N; ++i) {
    //  total += val[i] * val[i];
    //}
    //int result = (int)(std::sqrt(total));
    int temp;
	temp = (double)val[0] / 16;
	r_flt.write(temp);
	temp = (double)val[1] / 16;
	g_flt.write(temp);
	temp = (double)val[2] / 16;
	b_flt.write(temp);
    

	wait(10); //emulate module delay
  }
}

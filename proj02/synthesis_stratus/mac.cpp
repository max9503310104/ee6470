#include "stratus_hls.h"
#include "mac.h"

using namespace std;

mac::mac( sc_module_name n ): sc_module( n )
{
    SC_THREAD(compute);
	sensitive << i_clk.pos();
	dont_initialize();
    
    reset_signal_is(i_rst, false);
	i_a0.clk_rst(i_clk, i_rst);
	i_a1.clk_rst(i_clk, i_rst);
	i_a2.clk_rst(i_clk, i_rst);
	i_a3.clk_rst(i_clk, i_rst);
    i_b.clk_rst(i_clk, i_rst);
    o_c.clk_rst(i_clk, i_rst);
    
    HLS_FLATTEN_ARRAY(a);
    HLS_FLATTEN_ARRAY(b);
    HLS_FLATTEN_ARRAY(c);
}

mac::~mac() {}

void mac::compute() {   
    {
		HLS_DEFINE_PROTOCOL("main_reset");
		i_a0.reset();
		i_a1.reset();
		i_a2.reset();
		i_a3.reset();
        i_b.reset();
        o_c.reset();
		wait();
	}

	while (1) {
		{
		HLS_DEFINE_PROTOCOL("input");
		in_a0 = i_a0.get();
		in_a1 = i_a1.get();
		in_a2 = i_a2.get();
		in_a3 = i_a3.get();
		in_b = i_b.get();
		wait();
		}
		
		{HLS_CONSTRAIN_LATENCY(0, 0, "in");
		a[0][0] = in_a0.range(63, 48);
		a[0][1] = in_a0.range(47, 32);
		a[0][2] = in_a0.range(31, 16);
		a[0][3] = in_a0.range(15, 0);
		a[1][0] = in_a1.range(63, 48);
		a[1][1] = in_a1.range(47, 32);
		a[1][2] = in_a1.range(31, 16);
		a[1][3] = in_a1.range(15, 0);
		a[2][0] = in_a2.range(63, 48);
		a[2][1] = in_a2.range(47, 32);
		a[2][2] = in_a2.range(31, 16);
		a[2][3] = in_a2.range(15, 0);
		a[3][0] = in_a3.range(63, 48);
		a[3][1] = in_a3.range(47, 32);
		a[3][2] = in_a3.range(31, 16);
		a[3][3] = in_a3.range(15, 0);
		b[0] = in_b.range(63, 48);
		b[1] = in_b.range(47, 32);
		b[2] = in_b.range(31, 16);
		b[3] = in_b.range(15, 0);
		}
		
		for (int i = 0; i < 4; i++) {
			HLS_CONSTRAIN_LATENCY(0, 0, "mac");
			HLS_UNROLL_LOOP(ON);
			c[i] = a[i][0] * b[0] + a[i][1] * b[1] + a[i][2] * b[2] + a[i][3] * b[3];
		}
		
		{HLS_DEFINE_PROTOCOL("output");
		sc_dt::sc_biguint<(IL + FL) * 8> tmp_send;
		tmp_send.range(127, 96) = c[0];
		tmp_send.range(95, 64) = c[1];
		tmp_send.range(63, 32) = c[2];
		tmp_send.range(31, 0) = c[3];
		o_c.put(tmp_send);
		wait();
		}
    
    }
    
}

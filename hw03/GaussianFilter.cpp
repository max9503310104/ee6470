#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "GaussianFilter.h"

GaussianFilter::GaussianFilter( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_FLATTEN_ARRAY(val);
	HLS_FLATTEN_ARRAY(tmp);
	HLS_FLATTEN_ARRAY(fil);
	
#endif
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	i_rgb.clk_rst(i_clk, i_rst);
  o_result.clk_rst(i_clk, i_rst);
#endif
}

GaussianFilter::~GaussianFilter() {}																			

void GaussianFilter::do_filter() {
	sc_dt::sc_uint<9> row_in = 0, col_in = 0;
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
#endif	
		wait();
	}
	for (int i = 0; i < 256; i++) {		// 256 length			
		buf0[i] = 0;
		buf1[i] = 0;
		buf2[i] = 0;
	}		
	row_in = 0;
	col_in = 0;
	
	fil[0][0] = 1;
	fil[0][1] = 2;
	fil[0][2] = 1;
	fil[1][0] = 1;
	fil[1][1] = 2;
	fil[1][2] = 1;
	fil[2][0] = 2;
	fil[2][1] = 4;
	fil[2][2] = 2;
		
	while (true) {
		sc_dt::sc_uint<24> rgb;
		#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("input");
			rgb = i_rgb.get();
		}
		#else
			rgb = i_rgb.read();
		#endif
		
		// control index
		if (col_in == 257) {
			HLS_CONSTRAIN_LATENCY(0, 1, "index");
			col_in = 0;
			row_in++;
			
			sc_dt::sc_uint<3> tmp_swap0, tmp_swap1, tmp_swap2;
			tmp_swap0 = fil[2][0];
			tmp_swap1 = fil[2][1];
			tmp_swap2 = fil[2][2];
			for (int i = 2; i >= 1; i--) {
				HLS_UNROLL_LOOP(ON);
				for (int j = 0; j < 3; j++) {
					HLS_UNROLL_LOOP(ON);
					fil[i][j] = fil[i - 1][j];
				}
			}
			fil[0][0] = tmp_swap0;
			fil[0][1] = tmp_swap1;
			fil[0][2] = tmp_swap2;
			
		}
		if (col_in < 256) {
			HLS_CONSTRAIN_LATENCY(0, 1, "index");
			switch (row_in % 3) {
				case 0: buf0[col_in] = rgb; break;
				case 1: buf1[col_in] = rgb; break;
				case 2: buf2[col_in] = rgb; break;
			}
		}
				
		// prepare values for filter	
		if (col_in == 1) {
			HLS_CONSTRAIN_LATENCY(0, 2, "col0");
			tmp[0][0] = 0;
			tmp[1][0] = 0;
			tmp[2][0] = 0;
			for (int i = 1; i < 3; i++) {
				HLS_UNROLL_LOOP(ON);
				tmp[0][i] = buf0[col_in - 2 + i];
				tmp[1][i] = buf1[col_in - 2 + i];
				tmp[2][i] = buf2[col_in - 2 + i];
			}
		} else if (col_in == 256) {	
			HLS_CONSTRAIN_LATENCY(0, 1, "col1");
			for (int i = 0; i < 2; i++) {
				HLS_UNROLL_LOOP(ON);
				tmp[0][i] = tmp[0][i + 1];
				tmp[1][i] = tmp[1][i + 1];
				tmp[2][i] = tmp[2][i + 1];
			}
			tmp[0][2] = 0;
			tmp[1][2] = 0;
			tmp[2][2] = 0;
		} else {
			HLS_CONSTRAIN_LATENCY(0, 1, "col2");	
			for (int i = 0; i < 2; i++) {
				HLS_UNROLL_LOOP(ON);
				tmp[0][i] = tmp[0][i + 1];
				tmp[1][i] = tmp[1][i + 1];
				tmp[2][i] = tmp[2][i + 1];
			}
			tmp[0][2] = buf0[col_in];
			tmp[1][2] = buf1[col_in];
			tmp[2][2] = buf2[col_in];
		}
		
		// filter operation
		{
			HLS_PIPELINE_LOOP(SOFT_STALL, 1, "filter_pipeline");
			HLS_CONSTRAIN_LATENCY(0, 3, "latFilter");
			for (unsigned int i = 0; i < 3; ++i) {
				HLS_UNROLL_LOOP(ON);
				val[i] = 0;
			}	
			for (unsigned int v = 0; v<MASK_Y; ++v) {
				HLS_UNROLL_LOOP(ON);
				for (unsigned int u = 0; u<MASK_X; ++u) {		
					HLS_UNROLL_LOOP(ON);
					val[0] += tmp[v][u].range(7, 0) * fil[v][u];
					val[1] += tmp[v][u].range(15, 8) * fil[v][u];
					val[2] += tmp[v][u].range(23, 16) * fil[v][u];
				}
			}				
			for (unsigned int i = 0; i < 3; ++i) {
				HLS_UNROLL_LOOP(ON);
				val[i] = val[i] >> 4;		// div 16
			}	
		}
		
	
		if (row_in >= 1 && col_in >= 1) {
#ifndef NATIVE_SYSTEMC
			{
				HLS_DEFINE_PROTOCOL("output");
				sc_dt::sc_uint<32> total;
				total.range(7, 0) = val[0];
				total.range(15, 8) = val[1];
				total.range(23, 16) = val[2];
				o_result.put(total);
				wait();
			}
#else
			o_result.write(total);
#endif
		}
		col_in++;
	}
}

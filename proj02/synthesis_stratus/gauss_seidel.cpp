#include "stratus_hls.h"
#include "gauss_seidel.h"

using namespace std;

gauss_seidel::gauss_seidel( sc_module_name n ): sc_module( n )
{
    SC_THREAD(compute);
	sensitive << i_clk.pos();
	dont_initialize();
	
    
    reset_signal_is(i_rst, false);
	i_a0.clk_rst(i_clk, i_rst);
    i_a1.clk_rst(i_clk, i_rst);
    i_a2.clk_rst(i_clk, i_rst);
    i_a3.clk_rst(i_clk, i_rst);
    i_n.clk_rst(i_clk, i_rst);
    i_mode.clk_rst(i_clk, i_rst);
    o_x.clk_rst(i_clk, i_rst);
    
    o_mul_a0.clk_rst(i_clk, i_rst);
    o_mul_a1.clk_rst(i_clk, i_rst);
    o_mul_a2.clk_rst(i_clk, i_rst);
    o_mul_a3.clk_rst(i_clk, i_rst);
    o_mul_b.clk_rst(i_clk, i_rst);
    i_mul_c.clk_rst(i_clk, i_rst);
    
    HLS_FLATTEN_ARRAY(in);
    HLS_FLATTEN_ARRAY(in2);
    HLS_FLATTEN_ARRAY(xp0);
    HLS_FLATTEN_ARRAY(xp);
    HLS_FLATTEN_ARRAY(diag);
    HLS_FLATTEN_ARRAY(diag_x);
    HLS_FLATTEN_ARRAY(row_result);
    
    HLS_FLATTEN_ARRAY(tmp_c2);
}

gauss_seidel::~gauss_seidel() {}

void gauss_seidel::compute() {   
    {
		HLS_DEFINE_PROTOCOL("main_reset");
		i_a0.reset();
        i_a1.reset();
        i_a2.reset();
        i_a3.reset();
        i_n.reset();
        i_mode.reset();
		o_x.reset();
		
		i_mul_c.reset();
		o_mul_a0.reset();
		o_mul_a1.reset();
		o_mul_a2.reset();
		o_mul_a3.reset();
		o_mul_b.reset();
		wait();
	}

    // initialize
    for (int i = 0; i < 32; i++) {
    	HLS_CONSTRAIN_LATENCY(0, 1, "write_x_0");
        x0[i] = 0;
        x1[i] = 0;
        x2[i] = 0;
        x3[i] = 0;
    }
    row = 0;
    col = 0;
    iter = 0; 
    {
    HLS_DEFINE_PROTOCOL("input");
    dim = i_n.get();  
    mode = i_mode.get();
    wait();
    }
    if (mode == 0) {
    	iterMax = 4;
    } else {
    	iterMax = 1;
    }
    for (int i = 0; i < 8; i++) {
    	{
    	HLS_DEFINE_PROTOCOL("input");
        in[0] = i_a0.get();
        in[1] = i_a1.get();
        in[2] = i_a2.get();
        in[3] = i_a3.get();
        wait();
        }
        for (int j = 0; j < 4; j++) {
        	HLS_CONSTRAIN_LATENCY(0, 1, "write_b_0");
        	b0[i * 4 + j] = in[j].range((IL + FL) * 1 - 1, 0);
        	b1[i * 4 + j] = in[j].range((IL + FL) * 2 - 1, (IL + FL) * 1);
        	b2[i * 4 + j] = in[j].range((IL + FL) * 3 - 1, (IL + FL) * 2);
        	b3[i * 4 + j] = in[j].range((IL + FL) * 4 - 1, (IL + FL) * 3);
        }
    }
    

    // compute
    while (1) {   	   
        // read submatrix 
        {
        HLS_DEFINE_PROTOCOL("input");
        in[0] = i_a0.get();
        in[1] = i_a1.get();
        in[2] = i_a2.get();
        in[3] = i_a3.get();
        wait();
        }
        for (int i = 0; i < 4; i++) {
        	HLS_UNROLL_LOOP(ON, "in2_mv");
        	HLS_CONSTRAIN_LATENCY(0, 1, "in2_mv_lat");
            in2[i][0] = in[i].range((IL + FL) * 1 - 1, 0);
            in2[i][1] = in[i].range((IL + FL) * 2 - 1, (IL + FL) * 1);
            in2[i][2] = in[i].range((IL + FL) * 3 - 1, (IL + FL) * 2);
            in2[i][3] = in[i].range((IL + FL) * 4 - 1, (IL + FL) * 3);
        }

        if (col == 0) {     // new row
        	if (mode == 0) {
		    	HLS_CONSTRAIN_LATENCY(0, 1, "read_b_0");
		    	xp[0] = b0[row / 4] << FL;
		    	xp[1] = b1[row / 4] << FL;
		    	xp[2] = b2[row / 4] << FL;
		    	xp[3] = b3[row / 4] << FL; 
        	} else {
		    	HLS_CONSTRAIN_LATENCY(0, 1, "read_b_0");
		    	xp[0] = 0;
		    	xp[1] = 0;
		    	xp[2] = 0;
		    	xp[3] = 0; 
        	}
        }
        
        if (mode == 0) {
		    HLS_CONSTRAIN_LATENCY(0, 1, "read_x_0");
		    xp0[0] = x0[col / 4];
		    xp0[1] = x1[col / 4];
		    xp0[2] = x2[col / 4];
		    xp0[3] = x3[col / 4];
        } else {
        	HLS_CONSTRAIN_LATENCY(0, 1, "read_x_0");
		    xp0[0] = b0[row / 4];
		    xp0[1] = b1[col / 4];
		    xp0[2] = b2[col / 4];
		    xp0[3] = b3[col / 4];
        }
        /*{
        for (int i = 0; i < 4; i++) {       // current x computation   	
        	HLS_CONSTRAIN_LATENCY(1, 1, "xp_cmp_lat");      	
            xp[i] -= in2[i][0] * xp0[0] + in2[i][1] * xp0[1] + in2[i][2] * xp0[2] + in2[i][3] * xp0[3];
            
        }
        }*/
        
        
        
        {HLS_CONSTRAIN_LATENCY(0, 0, "out");     
        tmp_a0.range(63, 48) = in2[0][0]; 
        tmp_a0.range(47, 32) = in2[0][1];
        tmp_a0.range(31, 16) = in2[0][2]; 
        tmp_a0.range(15, 0) = in2[0][3];
        tmp_a1.range(63, 48) = in2[1][0]; 
        tmp_a1.range(47, 32) = in2[1][1];
        tmp_a1.range(31, 16) = in2[1][2]; 
        tmp_a1.range(15, 0) = in2[1][3];
        tmp_a2.range(63, 48) = in2[2][0]; 
        tmp_a2.range(47, 32) = in2[2][1];
        tmp_a2.range(31, 16) = in2[2][2]; 
        tmp_a2.range(15, 0) = in2[2][3];
        tmp_a3.range(63, 48) = in2[3][0]; 
        tmp_a3.range(47, 32) = in2[3][1];
        tmp_a3.range(31, 16) = in2[3][2]; 
        tmp_a3.range(15, 0) = in2[3][3];
        tmp_b.range(63, 48) = xp0[0]; 
        tmp_b.range(47, 32) = xp0[1];
        tmp_b.range(31, 16) = xp0[2]; 
        tmp_b.range(15, 0) = xp0[3];
        }
        {
        HLS_DEFINE_PROTOCOL("output");
        o_mul_a0.put(tmp_a0);
        o_mul_a1.put(tmp_a1);
        o_mul_a2.put(tmp_a2);
        o_mul_a3.put(tmp_a3);
        o_mul_b.put(tmp_b);
        wait();
        }         
        {
        HLS_DEFINE_PROTOCOL("input");
        tmp_c = i_mul_c.get();
        wait();
        }
        {HLS_CONSTRAIN_LATENCY(0, 0, "in");
        tmp_c2[0] = tmp_c.range(127, 96);
        tmp_c2[1] = tmp_c.range(95, 64);
        tmp_c2[2] = tmp_c.range(63, 32);
        tmp_c2[3] = tmp_c.range(31, 0);
        xp[0] -= tmp_c2[0];
        xp[1] -= tmp_c2[1];
        xp[2] -= tmp_c2[2];
        xp[3] -= tmp_c2[3];
        }
        
        
		
        // save diagonal
        if (col == row) {
            for (int i = 0; i < 4; i++) {
            	HLS_UNROLL_LOOP(ON, "diag_mv");
            	HLS_CONSTRAIN_LATENCY(0, 1, "diag_mv_lat");
                diag[i] = in2[i][i];
                diag_x[i] = xp0[i];
            }
        }
        col += 4;
 
        if (col == dim) {       // row done
        	//HLS_PIPELINE_LOOP(SOFT_STALL, 1, "row_rsl_pipe");
        	if (mode == 0) {
        		//HLS_CONSTRAIN_LATENCY(0, 1, "row_rsl_cmp_lat");
		        for (int i = 0; i < 4; i++) {
		        	//HLS_UNROLL_LOOP(COMPLETE, 4, "row_rsl_cmp");   
		        	//HLS_CONSTRAIN_LATENCY(0, 1, "row_rsl_cmp_lat");
		        	//HLS_PIPELINE_LOOP(SOFT_STALL, 1, "row_rsl_pipe");
		            row_result[i] = xp[i] / diag[i] + diag_x[i];
		        }
            } else {
            	//HLS_CONSTRAIN_LATENCY(0, 1, "row_rsl_cmp_lat");
		        for (int i = 0; i < 4; i++) {
		        	//HLS_UNROLL_LOOP(COMPLETE, 4, "row_rsl_cmp");   
		        	//HLS_CONSTRAIN_LATENCY(0, 1, "row_rsl_cmp_lat");
		        	//HLS_PIPELINE_LOOP(SOFT_STALL, 1, "row_rsl_pipe");
		            row_result[i] = xp[i] >> 8;
		        }
            }
            {
            HLS_CONSTRAIN_LATENCY(0, 1, "write_x_1");
            x0[row / 4] = row_result[0].range(IL + FL - 1, 0);
            x1[row / 4] = row_result[1].range(IL + FL - 1, 0);
            x2[row / 4] = row_result[2].range(IL + FL - 1, 0);
            x3[row / 4] = row_result[3].range(IL + FL - 1, 0);
            }
            row += 4;
            col = 0;
        }
        
        if (row == dim) {       // matrix done
            row = 0;
            iter++;
        }
        if (iter == iterMax) {   
            for (int i = 0; i < 32; i++) {
                sc_dt::sc_biguint<(IL + FL) * 4> tmp_send;
                {
                HLS_CONSTRAIN_LATENCY(0, 1, "read_x_1");
                tmp_send.range((IL + FL) * 1 - 1, 0) = x0[i];
                tmp_send.range((IL + FL) * 2 - 1, (IL + FL) * 1) = x1[i];
                tmp_send.range((IL + FL) * 3 - 1, (IL + FL) * 2) = x2[i];
                tmp_send.range((IL + FL) * 4 - 1, (IL + FL) * 3) = x3[i];
                }
                {
                HLS_DEFINE_PROTOCOL("output");
                o_x.put(tmp_send);
                wait();
                }
            }
            iter = 0;
        }
    }
}

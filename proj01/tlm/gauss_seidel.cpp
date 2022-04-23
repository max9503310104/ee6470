//#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "gauss_seidel.h"

using namespace std;

gauss_seidel::gauss_seidel( sc_module_name n ): sc_module( n )
{
    t_skt.register_b_transport(this, &gauss_seidel::blocking_transport);
    SC_THREAD(compute);
	sensitive << i_clk.pos();
	dont_initialize();
}

gauss_seidel::~gauss_seidel() {}

void gauss_seidel::compute() {
    sc_dt::sc_uint<64> in[4];
    sc_dt::sc_int<16> in2[4][4];
    sc_dt::sc_uint<8> dim;
    sc_dt::sc_int<16> xp0[4];
    sc_dt::sc_int<32> xp[4]; 
    sc_dt::sc_int<16> diag[4];
    sc_dt::sc_int<16> diag_x[4];
    sc_dt::sc_int<32> row_result[4];
    sc_dt::sc_uint<8> col, row;
    sc_dt::sc_uint<8> iter;
    
    // initialize
    for (int i = 0; i < 128; i++) {
        x[i] = 0;
    }
    row = 0;
    col = 0;
    iter = 0;
    dim = i_n.read();
    cout << "dim = " << dim << endl;
    for (int i = 0; i < 8; i++) {
        in[0] = i_a0.read();
        in[1] = i_a1.read();
        in[2] = i_a2.read();
        in[3] = i_a3.read();
        for (int j = 0; j < 4; j++) {
            b[i * 16 + j * 4] = in[j].range(15, 0);
            b[i * 16 + j * 4 + 1] = in[j].range(31, 16);
            b[i * 16 + j * 4 + 2] = in[j].range(47, 32);
            b[i * 16 + j * 4 + 3] = in[j].range(63, 48);
        }
    }

    // compute
    while(1) {      
        // read submatrix 
        in[0] = i_a0.read();
        in[1] = i_a1.read();
        in[2] = i_a2.read();
        in[3] = i_a3.read();
        for (int i = 0; i < 4; i++) {
            in2[i][0] = in[i].range(15, 0);
            in2[i][1] = in[i].range(31, 16);
            in2[i][2] = in[i].range(47, 32);
            in2[i][3] = in[i].range(63, 48);
        }

        if (col == 0) {     // new row
            for (int i = 0; i < 4; i++) {
                xp[i] = b[row + i] << 8;
            }         
        }
        for (int i = 0; i < 4; i++) {       // old x
            xp0[i] = x[col + i];
        }
        for (int i = 0; i < 4; i++) {       // current x computation
            xp[i] -= in2[i][0] * xp0[0] + in2[i][1] * xp0[1] + in2[i][2] * xp0[2] + in2[i][3] * xp0[3];
        }

        // save diagonal
        if (col == row) {
            for (int i = 0; i < 4; i++) {
                diag[i] = in2[i][i];
                diag_x[i] = xp0[i];
            }
        }
        col += 4;
 
        if (col == dim) {       // row done
            for (int i = 0; i < 4; i++) {
                //row_result[i] = ((xp[i] + diag_prd[i])) / diag[i];    // not used
                row_result[i] = xp[i] / diag[i] + diag_x[i];
                x[row + i] = row_result[i].range(15, 0);
            }
            row += 4;
            col = 0;
        }
        if (row == dim) {       // matrix done
            row = 0;
            iter++;
            
            /*cout << "x at " << sc_core::sc_time_stamp() << endl;
            for (int i = 0; i < dim; i++) {
                cout << (double)x[i]/(1<<8) << " ";
            }
            cout << endl;*/
            //cout << "iter " << iter << endl;
        }
        if (iter == 4) {
            for (int i = 0; i < 32; i++) {
                sc_dt::sc_uint<64> tmp_send;
                tmp_send.range(15, 0) = x[i * 4];
                tmp_send.range(31, 16) = x[i * 4 + 1];
                tmp_send.range(47, 32) = x[i * 4 + 2];
                tmp_send.range(63, 48) = x[i * 4 + 3];
                o_x.write(tmp_send);
            }
            iter = 0;
        }
       
        wait(1 * CLOCK_PERIOD, SC_NS);
    }
}

void gauss_seidel::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
    sc_dt::uint64 addr = payload.get_address();
    addr -= 0;  // base offset = 0
    //unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    unsigned char *data_ptr = payload.get_data_ptr();
    sc_dt::sc_uint<64> *uint_ptr = (sc_dt::sc_uint<64> *)data_ptr;
    sc_dt::sc_uint<8> tmp_dim = *(uint_ptr + 5);
    //word buffer;
    switch (payload.get_command()) {
        case tlm::TLM_READ_COMMAND:
            *(uint_ptr + 4) = o_x.read();
            break;
        case tlm::TLM_WRITE_COMMAND:
            switch (addr) {
                case 0:
                    i_a0.write(*(uint_ptr));
                    i_a1.write(*(uint_ptr + 1));
                    i_a2.write(*(uint_ptr + 2));
                    i_a3.write(*(uint_ptr + 3));
                    break;
                case 1:
                    i_n.write(tmp_dim);
                    break;
            }          
            break;
        case tlm::TLM_IGNORE_COMMAND:
            payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            return;
        default:
            payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            return;
    }
    payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
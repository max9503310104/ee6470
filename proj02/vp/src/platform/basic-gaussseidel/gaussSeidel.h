#ifndef GAUSS_SEIDEL_H_
#define GAUSS_SEIDEL_H_
#include <systemc>
#include <cmath>
#include <iomanip>
#include <stdio.h>
#include <cstdio>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "def.h"

struct GaussSeidel : public sc_module {
  tlm_utils::simple_target_socket<GaussSeidel> tsock;

  sc_fifo<bool> i_start;
  sc_fifo<bool> i_mode;
  sc_fifo<sc_dt::sc_uint<8> > i_N;
  sc_fifo<bool> o_done;


  SC_HAS_PROCESS(GaussSeidel);

  GaussSeidel(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt") 
    //base_offset(0) 
  {
    tsock.register_b_transport(this, &GaussSeidel::blocking_transport);
    SC_THREAD(do_computation);
  }

  ~GaussSeidel() {
	}

  //unsigned int base_offset;
  /*sc_dt::sc_int<64> A0[32][32], A1[32][32], A2[32][32], A3[32][32];
  sc_dt::sc_int<16> b0[32], b1[32], b2[32], b3[32];
  sc_dt::sc_int<32> bs0[32], bs1[32], bs2[32], bs3[32];
  sc_dt::sc_int<16> x0[32], x1[32], x2[32], x3[32];*/
  sc_dt::sc_int<64> A0[MAX_PART][MAX_PART], A1[MAX_PART][MAX_PART], A2[MAX_PART][MAX_PART], A3[MAX_PART][MAX_PART];
  sc_dt::sc_int<16> b0[MAX_PART], b1[MAX_PART], b2[MAX_PART], b3[MAX_PART];
  sc_dt::sc_int<32> bs0[MAX_PART], bs1[MAX_PART], bs2[MAX_PART], bs3[MAX_PART];
  sc_dt::sc_int<16> x0[MAX_PART], x1[MAX_PART], x2[MAX_PART], x3[MAX_PART];

  sc_dt::sc_int<16> subA[4][4];
  sc_dt::sc_int<16> xp0[4];
  sc_dt::sc_int<32> xp[4];		
  sc_dt::sc_int<16> diag[4];
  sc_dt::sc_int<16> diag_x[4];
  sc_dt::sc_int<32> row_result[4];		
  sc_dt::sc_uint<8> col, row;
  sc_dt::sc_uint<8> iter;
  sc_dt::sc_uint<8> N;
  //bool start
  bool mode;

  sc_dt::sc_uint<8> iterMax;


    void do_computation(){

        while (true) {       
            i_start.read();
			mode = i_mode.read();
            N = i_N.read();
            //printf("N = %d, mode = %d\n", (int)N, (int)mode);

            // initialize
            row = 0;
            col = 0;
            iter = 0;  
			if (mode == 0) {
				iterMax = 4;	// solve linear equation
			}  else {
				iterMax = 1;	// matrix-vector multiplication
			}

			for (int i = 0; i < MAX_PART; i++) {		// b shifted 8
				int temp[4];
				if (b0[i] & 0x00008000) {
					temp[0] = b0[i];
					temp[0] |= 0xffff0000; 
				} else {
					temp[0] = b0[i];
					temp[0] &= 0x0000ffff; 
				}
				if (b1[i] & 0x00008000) {
					temp[1] = b1[i];
					temp[1] |= 0xffff0000; 
				} else {
					temp[1] = b1[i];
					temp[1] &= 0x0000ffff; 
				}
				if (b2[i] & 0x00008000) {
					temp[2] = b2[i];
					temp[2] |= 0xffff0000; 
				} else {
					temp[2] = b2[i];
					temp[2] &= 0x0000ffff; 
				}
				if (b3[i] & 0x00008000) {
					temp[3] = b3[i];
					temp[3] |= 0xffff0000; 
				} else {
					temp[3] = b3[i];
					temp[3] &= 0x0000ffff; 
				}
				
				bs0[i] = temp[0] << 8;
				bs1[i] = temp[1] << 8;
				bs2[i] = temp[2] << 8;
				bs3[i] = temp[3] << 8;
			} 

			if (mode == 0) {	// solve linear equation
				for (int i = 0; i < MAX_PART; i++) {
					x0[i] = 0;
					x1[i] = 0;
					x2[i] = 0;
					x3[i] = 0;
				}
			} else {		// matrix-vector multiplication
				for (int i = 0; i < MAX_PART; i++) {
					bs0[i] = bs0[i] >> 8;
					bs1[i] = bs1[i] >> 8;
					bs2[i] = bs2[i] >> 8;
					bs3[i] = bs3[i] >> 8;
				}
			}

            // compute
            while (iter < iterMax) {   	   
                // read submatrix 
                for (int i = 0; i < 4; i++) {
                    subA[0][i] = A0[row / 4][col / 4].range((4 - i) * 16 - 1, (3 - i) * 16);
                    subA[1][i] = A1[row / 4][col / 4].range((4 - i) * 16 - 1, (3 - i) * 16);
                    subA[2][i] = A2[row / 4][col / 4].range((4 - i) * 16 - 1, (3 - i) * 16);
                    subA[3][i] = A3[row / 4][col / 4].range((4 - i) * 16 - 1, (3 - i) * 16);
                }

                if (col == 0) {     // new row
					if (mode == 0) {
						xp[0] = bs0[row / 4];
						xp[1] = bs1[row / 4];
						xp[2] = bs2[row / 4];
						xp[3] = bs3[row / 4];
					} else {
						xp[0] = 0;
						xp[1] = 0;
						xp[2] = 0;
						xp[3] = 0;
					}
                }
				if (mode == 0) {
					xp0[0] = x0[col / 4];
					xp0[1] = x1[col / 4];
					xp0[2] = x2[col / 4];
					xp0[3] = x3[col / 4]; 
				} else {
					xp0[0] = bs0[col / 4];
					xp0[1] = bs1[col / 4];
					xp0[2] = bs2[col / 4];
					xp0[3] = bs3[col / 4];
				}

				/*std::cout << "row,col= " << row << " " << col << "\n";
				for (int i = 0; i < 4; i++) {
					std::cout << xp[i] << " ";
				}
				std::cout << "\n";*/
                 

                for (int i = 0; i < 4; i++) {       // current x computation
                    xp[i] -= (sc_dt::sc_int<32>)subA[i][0] * xp0[0] + (sc_dt::sc_int<32>)subA[i][1] * xp0[1] + (sc_dt::sc_int<32>)subA[i][2] * xp0[2] + (sc_dt::sc_int<32>)subA[i][3] * xp0[3];
                }
                              
                // save diagonal
                if (col == row) {
                    for (int i = 0; i < 4; i++) {
                        diag[i] = subA[i][i];
                        diag_x[i] = xp0[i];
                    }
                }
                col += 4;
               
                if (col == N) {       // row done
					/*std::cout << "row sum= ";
					for (int i = 0; i < 4; i++) {
						std::cout << xp[i] << " ";
					}
					std::cout << "\n";*/

                    for (int i = 0; i < 4; i++) {
						if (mode == 0)
                        	row_result[i] = (sc_dt::sc_int<32>)xp[i] / diag[i] + (sc_dt::sc_int<32>)diag_x[i];
						else
							row_result[i] = (sc_dt::sc_int<32>)xp[i] / 256;
                    }

					/*std::cout << "row sum shifted= ";
					for (int i = 0; i < 4; i++) {
						std::cout << row_result[i] << " ";
					}
					std::cout << "\n";*/

                    x0[row / 4] = row_result[0].range(15, 0);
                    x1[row / 4] = row_result[1].range(15, 0);
                    x2[row / 4] = row_result[2].range(15, 0);
                    x3[row / 4] = row_result[3].range(15, 0);
                    row += 4;
                    col = 0;
                }
                
                if (row == N) {       // matrix done
                    row = 0;
                    iter++; 
                }
            }
            
            o_done.write(1);
            wait(CLOCK_PERIOD, SC_NS);
        }
    }

  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
    wait(delay);
    // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    // auto len = payload.get_data_length();
    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    unsigned char *data_ptr = payload.get_data_ptr();

    //addr -= 0; // base offset = 0
    int matrix_row;
    if (addr < GS_R_ADDR_A) {
      matrix_row = addr - GS_W_ADDR_A;
      addr = GS_W_ADDR_A;
    } else if (addr < 0x00004000) {
      matrix_row = addr - GS_R_ADDR_A;
      addr = GS_R_ADDR_A;
    }


    sc_dt::sc_int<64> *data_int_ptr;
    sc_dt::sc_int<16> *data_int16_ptr;
    unsigned char buffer[2048];

    switch (cmd) {
      case tlm::TLM_READ_COMMAND:
        switch (addr) {
          /*case GS_R_ADDR_A:
            for (int i = 0; i < 32; i++) {
              buffer[0 * 32 * 8 + i * 8 + 0] = A0[matrix_row][i].range(63, 56);
              buffer[0 * 32 * 8 + i * 8 + 1] = A0[matrix_row][i].range(55, 48);
              buffer[0 * 32 * 8 + i * 8 + 2] = A0[matrix_row][i].range(47, 40);
              buffer[0 * 32 * 8 + i * 8 + 3] = A0[matrix_row][i].range(39, 32);
              buffer[0 * 32 * 8 + i * 8 + 4] = A0[matrix_row][i].range(31, 24);
              buffer[0 * 32 * 8 + i * 8 + 5] = A0[matrix_row][i].range(23, 16);
              buffer[0 * 32 * 8 + i * 8 + 6] = A0[matrix_row][i].range(15, 8);
              buffer[0 * 32 * 8 + i * 8 + 7] = A0[matrix_row][i].range(7, 0);

              buffer[1 * 32 * 8 + i * 8 + 0] = A1[matrix_row][i].range(63, 56);
              buffer[1 * 32 * 8 + i * 8 + 1] = A1[matrix_row][i].range(55, 48);
              buffer[1 * 32 * 8 + i * 8 + 2] = A1[matrix_row][i].range(47, 40);
              buffer[1 * 32 * 8 + i * 8 + 3] = A1[matrix_row][i].range(39, 32);
              buffer[1 * 32 * 8 + i * 8 + 4] = A1[matrix_row][i].range(31, 24);
              buffer[1 * 32 * 8 + i * 8 + 5] = A1[matrix_row][i].range(23, 16);
              buffer[1 * 32 * 8 + i * 8 + 6] = A1[matrix_row][i].range(15, 8);
              buffer[1 * 32 * 8 + i * 8 + 7] = A1[matrix_row][i].range(7, 0);

              buffer[2 * 32 * 8 + i * 8 + 0] = A2[matrix_row][i].range(63, 56);
              buffer[2 * 32 * 8 + i * 8 + 1] = A2[matrix_row][i].range(55, 48);
              buffer[2 * 32 * 8 + i * 8 + 2] = A2[matrix_row][i].range(47, 40);
              buffer[2 * 32 * 8 + i * 8 + 3] = A2[matrix_row][i].range(39, 32);
              buffer[2 * 32 * 8 + i * 8 + 4] = A2[matrix_row][i].range(31, 24);
              buffer[2 * 32 * 8 + i * 8 + 5] = A2[matrix_row][i].range(23, 16);
              buffer[2 * 32 * 8 + i * 8 + 6] = A2[matrix_row][i].range(15, 8);
              buffer[2 * 32 * 8 + i * 8 + 7] = A2[matrix_row][i].range(7, 0);

              buffer[3 * 32 * 8 + i * 8 + 0] = A3[matrix_row][i].range(63, 56);
              buffer[3 * 32 * 8 + i * 8 + 1] = A3[matrix_row][i].range(55, 48);
              buffer[3 * 32 * 8 + i * 8 + 2] = A3[matrix_row][i].range(47, 40);
              buffer[3 * 32 * 8 + i * 8 + 3] = A3[matrix_row][i].range(39, 32);
              buffer[3 * 32 * 8 + i * 8 + 4] = A3[matrix_row][i].range(31, 24);
              buffer[3 * 32 * 8 + i * 8 + 5] = A3[matrix_row][i].range(23, 16);
              buffer[3 * 32 * 8 + i * 8 + 6] = A3[matrix_row][i].range(15, 8);
              buffer[3 * 32 * 8 + i * 8 + 7] = A3[matrix_row][i].range(7, 0);
            }
            
            for (int i = 0; i < 1024; i++) {
              data_ptr[i] = buffer[i];
            }
            break;*/
          case GS_R_ADDR_x:
            for (int i = 0; i < MAX_PART; i++) {
                buffer[i * 8] = x0[i].range(15, 8);
                buffer[i * 8 + 1] = x0[i].range(7, 0);
                buffer[i * 8 + 2] = x1[i].range(15, 8);
                buffer[i * 8 + 3] = x1[i].range(7, 0);
                buffer[i * 8 + 4] = x2[i].range(15, 8);
                buffer[i * 8 + 5] = x2[i].range(7, 0);
                buffer[i * 8 + 6] = x3[i].range(15, 8);
                buffer[i * 8 + 7] = x3[i].range(7, 0);
            }
            for (int i = 0; i < 256; i++) {
              data_ptr[i] = buffer[i];
            }
            break;
          case GS_R_ADDR_CTRL:
            *data_ptr = o_done.read();
            break;
          default:
            std::cerr << "READ Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
          }
        break;
      case tlm::TLM_WRITE_COMMAND:
        switch (addr) {
          case GS_W_ADDR_A:
            for (int i = 0; i < MAX_PART; i++) {
              data_int_ptr = (sc_dt::sc_int<64> *)(data_ptr + 0 * MAX_PART * 8 + i * 8 - 8);
              A0[matrix_row][i] = *data_int_ptr;
              data_int_ptr = (sc_dt::sc_int<64> *)(data_ptr + 1 * MAX_PART * 8 + i * 8 - 8);
              A1[matrix_row][i] = *data_int_ptr;
              data_int_ptr = (sc_dt::sc_int<64> *)(data_ptr + 2 * MAX_PART * 8 + i * 8 - 8);
              A2[matrix_row][i] = *data_int_ptr;
              data_int_ptr = (sc_dt::sc_int<64> *)(data_ptr + 3 * MAX_PART * 8 + i * 8 - 8);
              A3[matrix_row][i] = *data_int_ptr;
            }
            
            break;
          case GS_W_ADDR_b:
            for (int i = 0; i < MAX_PART; i++) {
                data_int16_ptr = (sc_dt::sc_int<16> *)(data_ptr + i * 8 - 8 + 6);
                b0[i] = *data_int16_ptr;
                data_int16_ptr = (sc_dt::sc_int<16> *)(data_ptr + i * 8 - 8 + 4);
                b1[i] = *data_int16_ptr;
                data_int16_ptr = (sc_dt::sc_int<16> *)(data_ptr + i * 8 - 8 + 2);
                b2[i] = *data_int16_ptr;
                data_int16_ptr = (sc_dt::sc_int<16> *)(data_ptr + i * 8 - 8 + 0);
                b3[i] = *data_int16_ptr;
            }
            
            break;
          case GS_W_ADDR_CTRL:
            i_start.write(1);
            i_N.write(*data_ptr);
			i_mode.write(*(data_ptr + 1));
            break;
          default:
            std::cerr << "WRITE Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                     << std::dec << " is not valid" << std::endl;
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
};
#endif

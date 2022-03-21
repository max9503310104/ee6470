#include "Top.h"
#include <cstdio>
#include <iomanip>

Top::Top(sc_module_name name) : sc_module(name), t_skt("t_skt"), base_offset(0) {
	SC_THREAD(do_buffer);
	SC_METHOD(do_filter);
	sensitive << trig_filter;
	dont_initialize();

	t_skt.register_b_transport(this, &Top::blocking_transport);
}

void Top::do_buffer()
{
	int i, j;
 
	row = 0, col = 0;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 256; j++) {
			rowBuf1[i][j] = 0;
			rowBuf2[i][j] = 0;
			rowBuf3[i][j] = 0;
		}
	}

	while (1) {
		if (row < 257) {
			rowBuf1[0][col] = i_r.read();
			rowBuf1[1][col] = i_g.read();
			rowBuf1[2][col] = i_b.read();
		} else if (row == 257) {
			break;
		}
		trig_filter.notify();

		col++;
		if (col == 256) {
			row++;
			col = 0;
			for (i = 0; i < 3; i++) {
				for (j = 0; j < 256; j++) {
					rowBuf3[i][j] = rowBuf2[i][j];
					rowBuf2[i][j] = rowBuf1[i][j];
				}
			}
		}
		
		wait(1,SC_NS);
	}
}

void Top::do_filter()
{
	int i, j;
	int val[3];
	int kernel[3][3] = {{1, 2, 1},
						{2, 4, 2},
						{1, 2, 1}};
	int temp;

	if (row >= 1 && row <= 256) {
		if (col >=1 && col <= 254) {
			for (i = 0; i < 3; i++) {	// r,g,b
				val[i] = 0;
				for (j = 0; j < 3; j++) {
					val[i] += rowBuf1[i][col - 1 + j] * kernel[0][j];
					val[i] += rowBuf2[i][col - 1 + j] * kernel[1][j];
					val[i] += rowBuf3[i][col - 1 + j] * kernel[2][j];
				}
			}
		} else if (col == 0) {
			for (i = 0; i < 3; i++) {	// r,g,b
				val[i] = 0;
				for (j = 1; j < 3; j++) {
					val[i] += rowBuf1[i][col - 1 + j] * kernel[0][j];
					val[i] += rowBuf2[i][col - 1 + j] * kernel[1][j];
					val[i] += rowBuf3[i][col - 1 + j] * kernel[2][j];
				}
			}
		} else if (col == 255) {
			for (i = 0; i < 3; i++) {	// r,g,b
				val[i] = 0;
				for (j = 0; j < 2; j++) {
					val[i] += rowBuf1[i][col - 1 + j] * kernel[0][j];
					val[i] += rowBuf2[i][col - 1 + j] * kernel[1][j];
					val[i] += rowBuf3[i][col - 1 + j] * kernel[2][j];
				}
			}			
		}
		temp = (double)val[0] / 16;
		o_r.nb_write(temp);
		temp = (double)val[1] / 16;
		o_g.nb_write(temp);
		temp = (double)val[2] / 16;
		o_b.nb_write(temp);
		//std::cout<<"filter write r c bleu"<<row<<" "<<col<<" "<<temp<<std::endl;
	}
}

void Top::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
	sc_dt::uint64 addr = payload.get_address();
	addr -= base_offset;
    unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    unsigned char *data_ptr = payload.get_data_ptr();
    word buffer;
    switch (payload.get_command()) {
		case tlm::TLM_READ_COMMAND:
    		switch (addr) {
	    		case SOBEL_FILTER_RESULT_ADDR: 
					buffer.uc[0] = o_r.read(); 
					buffer.uc[1] = o_g.read(); 
					buffer.uc[2] = o_b.read(); 
					break;	
				case SOBEL_FILTER_CHECK_ADDR: buffer.uint = o_r.num_available(); break;
		 		default:
					std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
		   	                  << std::setfill('0') << std::setw(8) << std::hex << addr
			                  << std::dec << " is not valid" << std::endl;
			}
			data_ptr[0] = buffer.uc[0];
			data_ptr[1] = buffer.uc[1];
		    data_ptr[2] = buffer.uc[2];
		    data_ptr[3] = buffer.uc[3];
		    break;
		case tlm::TLM_WRITE_COMMAND:
			switch (addr) {
    			case SOBEL_FILTER_R_ADDR:
	      			if (mask_ptr[0] == 0xff) {
		          		i_r.write(data_ptr[0]);
						//std::cout<<"b_transport write R"<<(int)data_ptr[0]<<std::endl;
					}
					if (mask_ptr[1] == 0xff) {
						i_g.write(data_ptr[1]);
						//std::cout<<"b_transport write G"<<(int)data_ptr[1]<<std::endl;
					}
					if (mask_ptr[2] == 0xff) {
						i_b.write(data_ptr[2]);
						//std::cout<<"b_transport write B"<<(int)data_ptr[2]<<std::endl;
					}
					break;
				default:
				    std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
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


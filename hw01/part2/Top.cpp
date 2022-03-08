#include "Top.h"
#include <cstdio>

Top::Top(sc_module_name name) : sc_module(name) {
	SC_THREAD(do_buffer);
	SC_METHOD(do_filter);
	sensitive << trig_filter;
	dont_initialize();
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
		if (row < 256) {
			rowBuf1[0][col] = i_r.read();
			rowBuf1[1][col] = i_g.read();
			rowBuf1[2][col] = i_b.read();
		} else if (row == 256) {
			rowBuf1[0][col] = 0;
			rowBuf1[1][col] = 0;
			rowBuf1[2][col] = 0;
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
		o_r.write(temp);
		temp = (double)val[1] / 16;
		o_g.write(temp);
		temp = (double)val[2] / 16;
		o_b.write(temp);
	}
}

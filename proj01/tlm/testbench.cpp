#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "testbench.h"
using namespace std;

testbench::testbench(sc_module_name n) : sc_module(n), initiator("initiator") {
	SC_THREAD(send_data);
	sensitive << i_clk.pos();
	dont_initialize();
	SC_THREAD(receive_data);
	sensitive << i_clk.pos();
	dont_initialize();
	dim = 80;
}

testbench::~testbench() {
	//cout<< "Max txn time = " << max_txn_time << endl;
	//cout<< "Min txn time = " << min_txn_time << endl;
	//cout<< "Avg txn time = " << total_txn_time/n_txn << endl;
	//cout << "Total run time = " << sc_core::sc_time_stamp() << endl;
}

void testbench::send_data() {	
	double A[128][128];
	double b[128];
	//int dim;
	unsigned char mask[320];
	word data;

#ifdef FILE_IN
	FILE *fin = NULL;
	fin = fopen("../m.dat", "r");
	if (fin) {
		cout << "read file success\n";
	} else {
		cout << "read file failed\n";
	}
	fscanf(fin, "%d", &dim);
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			fscanf(fin, "%lf", &A[i][j]);
		}
	}
	for (int i = 0; i < dim; i++) {
		fscanf(fin, "%lf", &b[i]);
	}
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			A[i][j] *= (1 << 8);
		}
		b[i] *= (1 << 8);
	}
#else
	srand(time(NULL));

	// generate A matrix. A is symmetric and positive definite
	//dim = 40;
	for (int i = 0; i < dim; i++) {
		for (int j = i; j < dim; j++) {
			A[i][j] = (double)rand() / (RAND_MAX + 1.0) * 2 - 1;	// -1 ~ 1
		}
	}
	for (int i = 1; i < dim; i++) {
		for (int j = 0; j < i; j++) {
			A[i][j] = A[j][i];
		}
	}
	for (int i = 0; i < dim; i++) {
		double sum = 0;
		for (int j = 0; j < dim; j++) {
			sum += abs(A[i][j]);
		}
		sum -= abs(A[i][i]);
		A[i][i] = sum;
	}
	for (int i = 0; i < dim; i++) {
		//x[i] = (double)(i + 1) / 10;		// set the value
		x[i] = (double)rand() / (RAND_MAX + 1.0); 		// 0 ~ 1
		b[i] = 0;		// initialization
	}
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			b[i] += A[i][j] * x[j];
		}
	}
#endif

	// show matrix
	/*cout << "show A, b, x\n";
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			printf("%lf ", A[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for (int i = 0; i < dim; i++) {
		printf("%lf ", b[i]);
	}
	printf("\n");
	for (int i = 0; i < dim; i++) {
		printf("%lf ", x[i]);
	}
	printf("\n\n");*/

	// start computation
	sc_dt::sc_int<16> tmp[4][4];
	sc_dt::sc_uint<64> tmp_sent[5];
	sc_dt::sc_uint<8> dim_sent;

	dim_sent = dim;
	data.uint[5] = dim_sent;
	initiator.write_to_socket(GS_MM_BASE + 1, mask, data.uc, 384);
	for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				tmp[j][k] = b[i * 16 + j * 4 + k] * (1 << 8);
			}
			tmp_sent[j].range(15, 0) = tmp[j][0];
			tmp_sent[j].range(31, 16) = tmp[j][1];
			tmp_sent[j].range(47, 32) = tmp[j][2];
			tmp_sent[j].range(63, 48) = tmp[j][3];
			data.uint[j] = tmp_sent[j];
		}
		initiator.write_to_socket(GS_MM_BASE, mask, data.uc, 384);		
    }
	//cout << "tb ini done at " << sc_core::sc_time_stamp() << endl;

	for (int iter = 0; iter < 1000; iter++) {
		for (int i = 0; i < dim / 4; i++) {			// row
			for (int j = 0; j < dim / 4; j++) {			// col
				for (int k = 0; k < 4; k++) {
					for (int l = 0; l < 4; l++) {
						tmp[k][l] = A[i * 4 + k][j * 4 + l] * (1 << 8);
					}
					tmp_sent[k].range(15, 0) = tmp[k][0];
					tmp_sent[k].range(31, 16) = tmp[k][1];
					tmp_sent[k].range(47, 32) = tmp[k][2];
					tmp_sent[k].range(63, 48) = tmp[k][3];
					data.uint[k] = tmp_sent[k];
				}
				initiator.write_to_socket(GS_MM_BASE, mask, data.uc, 384);
			}
		}
	}


	/*for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tmp[i][j] = A[i][j] * (1 << 8);
		}
		tmp_sent[i].range(15, 0) = tmp[i][0];
		tmp_sent[i].range(31, 16) = tmp[i][1];
		tmp_sent[i].range(47, 32) = tmp[i][2];
		tmp_sent[i].range(63, 48) = tmp[i][3];
		data.uint[i] = tmp_sent[i];
	}
	initiator.write_to_socket(GS_MM_BASE, mask, data.uc, 384);*/
	
	//cout << "tb write at " << sc_core::sc_time_stamp() << endl;

	
	wait(1 * CLOCK_PERIOD, SC_NS);
}

void testbench::receive_data() {
	unsigned char mask[320];
	word data;
	sc_dt::sc_uint<64> in;
	sc_dt::sc_int<16> in2[4];

	//double x_rcv[128];

	for (int i = 0; i < 32; i++) {
		initiator.read_from_socket(GS_MM_BASE, mask, data.uc, 384);
		in = data.uint[4];
		in2[0] = in.range(15, 0);
		in2[1] = in.range(31, 16);
		in2[2] = in.range(47, 32);
		in2[3] = in.range(63, 48);
		x_rcv[i * 4] = (double)in2[0] / (1 << 8);
		x_rcv[i * 4 + 1] = (double)in2[1] / (1 << 8);
		x_rcv[i * 4 + 2] = (double)in2[2] / (1 << 8);
		x_rcv[i * 4 + 3] = (double)in2[3] / (1 << 8);
	}
	cout << "\ntestbench receive\n";
	for (int i = 0; i < dim; i++) {
		cout << x_rcv[i] << " ";
	}
	cout << endl << endl;
	
	double max = 0;
	for (int i = 0; i < dim; i++) {
		if (abs(x_rcv[i] - x[i]) > max) {
			max = abs(x_rcv[i] - x[i]);
		}
	}
	cout << "solution difference = " << max << endl;

	sc_stop();
}
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "testbench.h"
using namespace std;

testbench::testbench(sc_module_name n) : sc_module(n) {
	SC_THREAD(send_data);
	sensitive << i_clk.pos();
	dont_initialize();
	SC_THREAD(receive_data);
	sensitive << i_clk.pos();
	dont_initialize();
	dim = DIM;
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

	// start computation
	sc_dt::sc_int<IL + FL> tmp[4][4];
	sc_dt::sc_biguint<(IL + FL) * 4> tmp_sent[5];
	sc_dt::sc_uint<8> dim_sent;
	
	o_a0.reset();
	o_a1.reset();
	o_a2.reset();
	o_a3.reset();
	o_n.reset();
	o_rst.write(false);
	wait(5);
	o_rst.write(true);
	wait(1);
	start_time = sc_time_stamp();

	dim_sent = dim;
	o_n.put(dim_sent);
	cout << endl << "dim = " << dim << endl;
	for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				tmp[j][k] = b[i * 16 + j * 4 + k] * (1 << FL);
			}
			tmp_sent[j].range(IL + FL - 1, 0) = tmp[j][0];
			tmp_sent[j].range((IL + FL) * 2 - 1, IL + FL) = tmp[j][1];
			tmp_sent[j].range((IL + FL) * 3 - 1, (IL + FL) * 2) = tmp[j][2];
			tmp_sent[j].range((IL + FL) * 4 - 1, (IL + FL) * 3) = tmp[j][3];
		}	
		o_a0.put(tmp_sent[0]);
		o_a1.put(tmp_sent[1]);
		o_a2.put(tmp_sent[2]);
		o_a3.put(tmp_sent[3]);
    }

	for (int iter = 0; iter < 1000; iter++) {
		for (int i = 0; i < dim / 4; i++) {			// row
			for (int j = 0; j < dim / 4; j++) {			// col
				for (int k = 0; k < 4; k++) {
					for (int l = 0; l < 4; l++) {
						tmp[k][l] = A[i * 4 + k][j * 4 + l] * (1 << FL);
					}
					tmp_sent[k].range((IL + FL) * 1 - 1, 0) = tmp[k][0];
					tmp_sent[k].range((IL + FL) * 2 - 1, (IL + FL) * 1) = tmp[k][1];
					tmp_sent[k].range((IL + FL) * 3 - 1, (IL + FL) * 2) = tmp[k][2];
					tmp_sent[k].range((IL + FL) * 4 - 1, (IL + FL) * 3) = tmp[k][3];
				}
				o_a0.put(tmp_sent[0]);
				o_a1.put(tmp_sent[1]);
				o_a2.put(tmp_sent[2]);
				o_a3.put(tmp_sent[3]);
			}
		}
	}
}

void testbench::receive_data() {
	sc_dt::sc_biguint<(IL + FL) * 4> in;
	sc_dt::sc_int<IL + FL> in2[4];
	int first = 1;

	i_x.reset();

	for (int i = 0; i < 32; i++) {
		in = i_x.get();

		in2[0] = in.range((IL + FL) * 1 - 1, 0);
		in2[1] = in.range((IL + FL) * 2 - 1, (IL + FL) * 1);
		in2[2] = in.range((IL + FL) * 3 - 1, (IL + FL) * 2);
		in2[3] = in.range((IL + FL) * 4 - 1, (IL + FL) * 3);
		x_rcv[i * 4] = (double)in2[0] / (1 << FL);
		x_rcv[i * 4 + 1] = (double)in2[1] / (1 << FL);
		x_rcv[i * 4 + 2] = (double)in2[2] / (1 << FL);
		x_rcv[i * 4 + 3] = (double)in2[3] / (1 << FL);
	}
	start_time = sc_time_stamp() - start_time;
	
	cout << "testbench receive\n";
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
	cout << "simulation time = " << start_time.to_default_time_units() << " ns" << endl;	
	cout << "simulation cycle = " << start_time.to_default_time_units() / 10 << endl;
	sc_stop();
}

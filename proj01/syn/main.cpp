#include <iostream>
#include <string>
using namespace std;

#include "esc.h"

// Wall Clock Time Measurement
#include <sys/time.h>

#include "gauss_seidel.h"
#include "testbench.h"
#include "system.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

System * sys = NULL;

extern void esc_elaborate()
{
	sys = new System("sys");
}
extern void esc_cleanup()
{
	delete sys;
}

int sc_main(int argc, char **argv) {
  esc_initialize(argc, argv);
  esc_elaborate();
  sc_start();
  esc_cleanup();

  return 0;
}

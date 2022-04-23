#include <iostream>
#include <string>
using namespace std;

// Wall Clock Time Measurement
#include <sys/time.h>

//#include "SimpleBus.h"
#include "gauss_seidel.h"
#include "testbench.h"
#include "system.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

System * sys = NULL;

int sc_main(int argc, char **argv) {

  sys = new System("sys");
  sc_start(1000000, SC_NS);

  return 0;
}

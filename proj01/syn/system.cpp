#include "system.h"

System::System(sc_module_name name): sc_module(name), 
	tb("tb"), gauss_seidel_0("GS"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst")
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	gauss_seidel_0.i_clk(clk);
	gauss_seidel_0.i_rst(rst);
	tb.o_a0(a0);
  tb.o_a1(a1);
  tb.o_a2(a2);
  tb.o_a3(a3);
	tb.o_n(n);
	tb.i_x(x);
	gauss_seidel_0.i_a0(a0);
  gauss_seidel_0.i_a1(a1);
  gauss_seidel_0.i_a2(a2);
  gauss_seidel_0.i_a3(a3);
	gauss_seidel_0.i_n(n);
	gauss_seidel_0.o_x(x);

	
}

// must exist
System::~System() {

}

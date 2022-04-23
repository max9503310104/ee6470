#include "System.h"

System::System(sc_module_name name): sc_module(name), 
	tb("tb"), gauss_seidel_0("GS"), bus("bus"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst")
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	gauss_seidel_0.i_clk(clk);
	gauss_seidel_0.i_rst(rst);
	#ifndef NATIVE_SYSTEMC
	tb.o_a0(a0);
    tb.o_a1(a1);
    tb.o_a2(a2);
    tb.o_a3(a3);
	tb.o_n(n)
	tb.i_x(x);
	gauss_seidel_0.i_a0(a0);
    gauss_seidel_0.i_a1(a1);
    gauss_seidel_0.i_a2(a2);
    gauss_seidel_0.i_a3(a3);
	gauss_seidel_0.i_n(n);
	gauss_seidel_0.o_x(x);
	#endif

	bus.set_clock_period(sc_time(CLOCK_PERIOD, SC_NS));
	tb.initiator.i_skt(bus.t_skt[0]);
	bus.setDecode(0, GS_MM_BASE, GS_MM_BASE + GS_MM_SIZE - 1);
	bus.i_skt[0](gauss_seidel_0.t_skt);
    
	//std::cout << "Simulation time == " << sc_core::sc_time_stamp() << std::endl;
}

// must exist
System::~System() {

}

all : main.cpp bootstrap.S
	riscv32-unknown-elf-g++ -std=c++14 main.cpp  bootstrap.S -o main -march=rv32ima -mabi=ilp32 -nostartfiles

#all : main.cpp
#	riscv32-unknown-elf-g++ -std=c++14 main.cpp bootstrap.S -o main -march=rv32ima -mabi=ilp32

# don't use
#all : main_c.c bootstrap.S
#	riscv32-unknown-elf-gcc main_c.c bootstrap.S -o main -march=rv32ima -mabi=ilp32 -lm -nostartfiles
	
sim: all
	$(EE6470)/riscv-vp/vp/build/bin/riscv-vp-gaussseidel --intercept-syscalls main
	
clean:
	rm -f main

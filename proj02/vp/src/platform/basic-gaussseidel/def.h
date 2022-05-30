#ifndef DEF_H_
#define DEF_H_

#define CLOCK_PERIOD 10
#define MAX_PART 8    // the submatrix number in one side

// inner transport addresses
const int GS_W_ADDR_A = 0x00000000;
const int GS_R_ADDR_A = 0x00002000;
const int GS_W_ADDR_CTRL = 0x00004000;
const int GS_R_ADDR_CTRL = 0x00005000;
const int GS_W_ADDR_b = 0x00006000;
const int GS_R_ADDR_x = 0x00007000;

/*const int SOBEL_FILTER_RS_R_ADDR   = 0x00000000;
const int SOBEL_FILTER_RS_W_WIDTH  = 0x00000004;
const int SOBEL_FILTER_RS_W_HEIGHT = 0x00000008;
const int SOBEL_FILTER_RS_W_DATA   = 0x0000000C;
const int SOBEL_FILTER_RS_RESULT_ADDR = 0x00800000;*/


union word {
  sc_dt::sc_uint<64> uint[4][32];
  unsigned char uc[1024];
  word() {
    for (int i = 0; i < 1024; i++) {
      uc[i] = 0;
    }
  };
  ~word() {};
};


#endif

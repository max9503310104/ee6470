#ifndef GS_DEF_H_
#define GS_DEF_H_

#define IL 8
#define FL 8

union word {
    sc_dt::sc_uint<64> uint[6];
    unsigned char uc[384];

    word() {
        for (int i = 0; i < 6; i++) {
            uint[i] = 0;
        }
    };
    ~word() {};
};


#endif
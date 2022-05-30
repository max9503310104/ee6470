#include "string"
#include "string.h"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "cmath"

#define N 128                       // metrix size      // N <= 256
#define MAX_N 32                    // max hardware size
#define MAX_submtx (N / MAX_N)      // max partition
#define N_PE 4                      // number of PE     // N_PE <= 8

// hardware acc
static char* const GS_W_ADDR_A = reinterpret_cast<char* const>(0x73000000);
static char* const GS_R_ADDR_A  = reinterpret_cast<char* const>(0x73002000);
static char* const GS_W_ADDR_CTRL = reinterpret_cast<char* const>(0x73004000);
static char* const GS_R_ADDR_CTRL = reinterpret_cast<char* const>(0x73005000);
static char* const GS_W_ADDR_b = reinterpret_cast<char* const>(0x73006000);
static char* const GS_R_ADDR_x = reinterpret_cast<char* const>(0x73007000);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;
bool _is_using_dma = true;

// barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 

// golbal variables
double A[N][N];
double x[N];
double b[N];

int64_t Ai[MAX_N][MAX_N / 4];  //32*8
int64_t bi[MAX_N / 4];      //8
unsigned char *w_start, *r_start;
unsigned char r_buffer[MAX_N * 4 * 2];    //256
unsigned char w_buffer[16];

double subA[MAX_submtx][MAX_submtx][MAX_N][MAX_N];
double subx[MAX_submtx][MAX_N];
double subb[MAX_submtx][MAX_N];
double subr[MAX_submtx][MAX_N];
double subr_acc[MAX_submtx][MAX_N];

int work_list[512][4];
int n_work;


int sem_init (uint32_t *__sem, uint32_t count) __THROW
{
  *__sem=count;
  return 0;
}

int sem_wait (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int sem_post (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
	sem_wait(__lock);
	if (*counter == thread_count - 1) { //all finished
		*counter = 0;
		sem_post(__lock);
		for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
	} else {
		(*counter)++;
		sem_post(__lock);
		sem_wait(__sem);
	}
	return 0;
}


void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

//int main(int argc, char *argv[]) {
int main(unsigned hart_id) {
    if (hart_id == 0) {     // semaphore initialization
        sem_init(&barrier_lock, 1);
		sem_init(&barrier_sem, 0);
    }
    
    if (hart_id == 0) {
    printf("======================================\n");
    printf("Start\n");
    printf("Matrix generation (it takes some time)\nN = %d\n", N);
    srand(5);
    for (int i = 0; i < N; i++) {
		for (int j = i; j < N; j++) {
			A[i][j] = (double)rand() / (RAND_MAX + 1.0) - 0.5;	// -0.5 ~ 0.5
		}
	}
    for (int i = 1; i < N; i++) {
		for (int j = 0; j < i; j++) {
			A[i][j] = A[j][i];
		}
	}
	for (int i = 0; i < N; i++) {
		double sum = 0;
		for (int j = 0; j < N; j++) {
			sum += abs(A[i][j]);
		}
		sum -= abs(A[i][i]);
		A[i][i] = sum;
	}
	for (int i = 0; i < N; i++) {
        //x[i] = (double)(i + 1) / N;		// set the value
		x[i] = (double)rand() / (RAND_MAX + 1.0) / 2; 		// 0 ~ 0.5
		b[i] = 0;		// initialization
	}
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			b[i] += A[i][j] * x[j];
		}
	}
    
    printf("\nx\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", (int)(x[i]*256));
    }
    printf("\n");

    printf("\nMatrix partition\n");
    for (int i = 0; i < N / MAX_N; i++) {
        for (int j = 0; j < N / MAX_N; j++) {
            for (int k = 0; k < MAX_N; k++) {
                for (int l = 0; l < MAX_N; l++) {
                    subA[i][j][k][l] = A[i * MAX_N + k][j * MAX_N + l];
                }
            }
        }
    }
    for (int i = 0; i < N / MAX_N; i++) {
        for (int j = 0; j < MAX_N; j++) {
            subb[i][j] = b[i * MAX_N + j];
            subx[i][j] = x[i * MAX_N + j];
        }
    }

    printf("\nScheduling\n");
    int idx_list;
    int PEtime[N_PE] = {0};
    int min_PE;
    int min_time;
    int sol_queue[N / MAX_N][2] = {0};
    int sol_queue_h = 0;
    int sol_queue_t = 0;
    int finish_time[N / MAX_N][N / MAX_N] = {0};
    int max_row_time;
    for (int i = 0; i < N / MAX_N; i++) {
        finish_time[i][i] = -1;
    }
    for (int i = 0; i < N / MAX_N; i++) {
        for (int j = 0; j < N / MAX_N; j++) {
            if (j == i) {
                j++;
                if (j == N / MAX_N)
                    break;
            }

            min_time = 10000;
            for (int k = 0; k < N_PE; k++) {
                if (PEtime[k] < min_time) {
                    min_time = PEtime[k];
                    min_PE = k;
                }
            }

            while (sol_queue_h != sol_queue_t && min_time >= sol_queue[sol_queue_h][0]) { // clear the queue
                min_time = 10000;
                for (int k = 0; k < N_PE; k++) {
                    if (PEtime[k] < min_time) {
                        min_time = PEtime[k];
                        min_PE = k;
                    }
                }
                work_list[idx_list][0] = 3;     // sol
                work_list[idx_list][1] = sol_queue[sol_queue_h][1];
                work_list[idx_list][2] = sol_queue[sol_queue_h][1];
                work_list[idx_list][3] = min_PE;
                PEtime[min_PE] += 4;
                idx_list++;
                sol_queue_h++;
            }

            min_time = 10000;
            for (int k = 0; k < N_PE; k++) {
                if (PEtime[k] < min_time) {
                    min_time = PEtime[k];
                    min_PE = k;
                }
            }
            work_list[idx_list][0] = 1;     // mul
            work_list[idx_list][1] = i;
            work_list[idx_list][2] = j;
            work_list[idx_list][3] = min_PE;
            PEtime[min_PE] += 1;
            finish_time[i][j] = PEtime[min_PE];
            idx_list++;
        }
        for (int k = 0; k < N / MAX_N; k++) {
            if (k != i) {
                work_list[idx_list][0] = 2;     // acc
                work_list[idx_list][1] = i;
                work_list[idx_list][2] = k;
                idx_list++;
            }
        }

        min_time = 10000;
        for (int k = 0; k < N_PE; k++) {
            if (PEtime[k] < min_time) {
                min_time = PEtime[k];
                min_PE = k;
            }
        }
        max_row_time = -1;
        for (int k = 0; k < N / MAX_N; k++) {
            if (finish_time[i][k] > max_row_time) {
                max_row_time = finish_time[i][k];
            }
        }
        if (min_time < max_row_time) {
            sol_queue[sol_queue_t][0] = max_row_time;
            sol_queue[sol_queue_t][1] = i;
            sol_queue_t++;
        } else {
            work_list[idx_list][0] = 3;     // sol
            work_list[idx_list][1] = i;
            work_list[idx_list][2] = i;
            work_list[idx_list][3] = min_PE;
            PEtime[min_PE] += 4;
            idx_list++;
        }
    }
    while (sol_queue_h != sol_queue_t) {   // clear the queue
        do {
            min_time = 10000;
            for (int k = 0; k < N_PE; k++) {
                if (PEtime[k] < min_time) {
                    min_time = PEtime[k];
                    min_PE = k;
                }
            }
            max_row_time = -1;
            for (int k = 0; k < N / MAX_N; k++) {
                if (finish_time[sol_queue[sol_queue_h][1]][k] > max_row_time) {
                    max_row_time = finish_time[sol_queue[sol_queue_h][1]][k];
                }
            }
            if (min_time < max_row_time) {
                PEtime[min_PE]++;
            }
        } while (min_time < max_row_time);
        work_list[idx_list][0] = 3;     // sol
        work_list[idx_list][1] = sol_queue[sol_queue_h][1];//N / MAX_N - 1; 
        work_list[idx_list][2] = sol_queue[sol_queue_h][1];//N / MAX_N - 1; 
        work_list[idx_list][3] = min_PE;//work_list[idx_list - N / MAX_N][3];
        PEtime[min_PE] += 4;
        idx_list++;
        sol_queue_h++;
    }
    printf("final time: ");
    for (int k = 0; k < N_PE; k++) {
        printf("%d ", PEtime[k]);
    }
    printf("\n");
    n_work = idx_list;
    
    printf("the number of works: %d\n", n_work);
    for (int i = 0; i < n_work; i++) {
        switch (work_list[i][0]) {
            case 1: printf("mul A_%d%d * x_%d PE %d\n", work_list[i][1], work_list[i][2], work_list[i][2], work_list[i][3]); break;
            case 2: printf("acc r_%d -= A_%d%d * x_%d\n", work_list[i][1], work_list[i][1], work_list[i][2], work_list[i][2]); break;
            case 3: printf("sol A_%d%d * x_%d = r_%d PE %d\n", work_list[i][1], work_list[i][1], work_list[i][1], work_list[i][1], work_list[i][3]); break;
            default: printf("wrong work");
        }  
    }
    }
    barrier(&barrier_sem, &barrier_lock, &barrier_counter, 2);
    
    
    if (hart_id == 1) {
    printf("\nIteration starts\n");
    for (int i = 0; i < MAX_submtx; i++) {
        for (int j = 0; j < MAX_N; j++) {
            subx[i][j] = 0;
        }
    }
    for (int i = 0; i < 4; i++) {       // iter num = 4
        printf("iter %d/4\n", i);
        // initialization
        for (int j = 0; j < MAX_submtx; j++) {
            for (int k = 0; k < MAX_N; k++) {
                subr_acc[j][k] = subb[j][k];
            }
        }
        // do the list
        for (int j = 0; j < n_work; j++) {
            int row = work_list[j][1];
            int col = work_list[j][2];
            int pe = work_list[j][3];
            printf("%d ", j);
            switch (work_list[j][0]) {
                case 1:     // mul
                    for (int k = 0; k < MAX_N; k++) {    
                        for (int l = 0; l < MAX_N / 4; l++) {    
                            Ai[k][l] = ((int64_t)(subA[row][col][k][l * 4] * 256) << 48) 
                                        + ((int64_t)(subA[row][col][k][l * 4 + 1] * 256) << 32) 
                                        + ((int64_t)(subA[row][col][k][l * 4 + 2] * 256) << 16) 
                                        + ((int64_t)(subA[row][col][k][l * 4 + 3] * 256));
                        }
                    }
                    for (int k = 0; k < MAX_N / 4; k++) {
                        bi[k] = ((int64_t)(subx[col][k * 4] * 256) << 48)
                                + ((int64_t)(subx[col][k * 4 + 1] * 256) << 32)
                                + ((int64_t)(subx[col][k * 4 + 2] * 256) << 16)
                                + ((int64_t)(subx[col][k * 4 + 3] * 256));
                    }

                    printf("mul: send to PE %d, row %d, col %d...", pe, row, col);
                    for (int k = 0; k < MAX_N / 4; k++) {
                        w_start = (unsigned char*)Ai[k * 4];
                        write_data_to_ACC(GS_W_ADDR_A + k + 0x01000000 * pe, w_start, MAX_N * 4 * 2);
                    }
                    w_start = (unsigned char*)bi;
                    write_data_to_ACC(GS_W_ADDR_b + 0x01000000 * pe, w_start, MAX_N * 2);
                    w_buffer[0] = MAX_N;   // N
                    w_buffer[1] = 1;    // mode
                    write_data_to_ACC(GS_W_ADDR_CTRL + 0x01000000 * pe, w_buffer, 2);

                    r_buffer[0] = 0;
                    read_data_from_ACC(GS_R_ADDR_CTRL + 0x01000000 * pe, r_buffer, 1);
                    printf("receive %d\n",r_buffer[0]);
                    read_data_from_ACC(GS_R_ADDR_x + 0x01000000 * pe, r_buffer, MAX_N * 2); 
                    for (int k = 0; k < MAX_N; k++) {
                        int t;
                        double td;
                        t = (int)r_buffer[k * 2] * (1 << 8) + (int)r_buffer[k * 2 + 1];
                        if (r_buffer[k * 2] & 0x80) {
                            t = -(-t + 65536);
                        }
                        td = (double)t / 256;
                        subr[col][k] = td;
                    }
                    break;
                case 2:     // acc
                    printf("acc: row %d, col %d\n", row, col);
                    for (int k = 0; k < MAX_N; k++) {
                        subr_acc[row][k] += subr[col][k];
                    }
                    break;
                case 3:     // sol
                    for (int k = 0; k < MAX_N; k++) {    
                        for (int l = 0; l < MAX_N / 4; l++) {    
                            Ai[k][l] = ((int64_t)(subA[row][col][k][l * 4] * 256) << 48) 
                                        + ((int64_t)(subA[row][col][k][l * 4 + 1] * 256) << 32) 
                                        + ((int64_t)(subA[row][col][k][l * 4 + 2] * 256) << 16) 
                                        + ((int64_t)(subA[row][col][k][l * 4 + 3] * 256));
                        }
                    }
                    for (int k = 0; k < MAX_N / 4; k++) {
                        bi[k] = ((int64_t)(subr_acc[col][k * 4] * 256) << 48) + ((int64_t)(subr_acc[col][k * 4 + 1] * 256) << 32)
                                + ((int64_t)(subr_acc[col][k * 4 + 2] * 256) << 16) + ((int64_t)(subr_acc[col][k * 4 + 3] * 256) << 0);
                    }

                    printf("sol: send to PE %d, row %d, col %d...", pe, row, col);
                    for (int k = 0; k < MAX_N / 4; k++) {
                        w_start = (unsigned char*)Ai[k * 4];
                        write_data_to_ACC(GS_W_ADDR_A + k + 0x01000000 * pe, w_start, MAX_N * 4 * 2);
                    }
                    w_start = (unsigned char*)bi;
                    write_data_to_ACC(GS_W_ADDR_b + 0x01000000 * pe, w_start, MAX_N * 2);
                    w_buffer[0] = MAX_N;   // N
                    w_buffer[1] = 0;    // mode
                    write_data_to_ACC(GS_W_ADDR_CTRL + 0x01000000 * pe, w_buffer, 2);

                    r_buffer[0] = 0;
                    read_data_from_ACC(GS_R_ADDR_CTRL + 0x01000000 * pe, r_buffer, 1);
                    printf("receive %d\n",r_buffer[0]);
                    read_data_from_ACC(GS_R_ADDR_x + 0x01000000 * pe, r_buffer, MAX_N * 2); 
                    for (int k = 0; k < MAX_N; k++) {
                        int t;
                        double td;
                        t = (int)r_buffer[k * 2] * (1 << 8) + (int)r_buffer[k * 2 + 1];
                        if (r_buffer[k * 2] & 0x80) {
                            t = -(-t + 65536);
                        }
                        td = (double)t / 256;
                        subx[col][k] = td;
                    }
                    break;

                default: printf("skip\n");

            }
        }
    }

    printf("\nSolution\n");
    int err = 0;
    for (int j = 0; j < MAX_submtx; j++) {
        for (int k = 0; k < MAX_N; k++) {
            printf("%d ", (int)(subx[j][k]*256));
            //err += ((int)(subx[j][k]*256) - (int)(x[j * MAX_N + k]*256)) * ((int)(subx[j][k]*256) - (int)(x[j * MAX_N + k]*256));
            if (abs((int)(subx[j][k]*256) - (int)(x[j * MAX_N + k]*256)) > err) {
                err = abs((int)(subx[j][k]*256) - (int)(x[j * MAX_N + k]*256));
            }
        }
    }
    //err = sqrt(err);
    //printf("\ndifference = %d*2^(-%d)/%d\n", err, 8, N);
    printf("\ndifference = %d/2^8\n\n", err);

    }
    
    
    return 0;
}

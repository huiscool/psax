#include "psax.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __APPLE__
#include "pthread_barriers.h"
#endif

#define DEBUG

#ifdef DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#define start_loop_print(np, rank) {\
    pthread_barrier_t barrier; \
    pthread_barrier_init(&barrier, NULL, np); \
    for(int p = 0; p<np; p++){ \
        if(p == rank)

#define end_loop_print \
        pthread_barrier_wait(&barrier); \
    } \
    pthread_barrier_destroy(&barrier); \
}
#include <assert.h>
#else
#define debug_printf(...)
#define NDEBUG
#include <assert.h>
#endif //DEBUG

///////////////////////////////
typedef struct glo{
    
}glo_t;



int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename){

#ifdef TEST_1
//test open file
start_loop_print(1,0){
    debug_printf("rank: 0\n");
}
end_loop_print

#else //TEST_1

#endif //TEST_1
    return 0;
}

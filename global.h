#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include "psax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <time.h>
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

typedef unsigned char bool;

typedef struct glov{
    int np; // number of threads participating parsing
    int fd; // file descriptor
    char* file_buf; // map file into memory space
    int64_t file_size; // file size
    event_handler_t event_handler;
    error_handler_t error_handler;
}glov_t;

glov_t glo;
error_t glo_error;

void raise_error(error_type_t type, int64_t row, int64_t col, char* msg);
void open_file(const char* filename, glov_t* glo);
void close_file(glov_t* glo);

typedef struct send_recv_buf{
    pthread_mutex_t lock;
    pthread_rwlock_t rwlock;
    pthread_cond_t cond_empty;
    pthread_cond_t cond_full;
    size_t max_size;
    size_t ele_size;
    size_t cur_size;
    int64_t front;
    int64_t back;
    void* buf;
}send_recv_buf_t;

//return 0 if success; return -1 if failed.
int buf_init(send_recv_buf_t* buf, size_t size, size_t ele_size);
void buf_destroy(send_recv_buf_t* buf);
void buf_push_back(send_recv_buf_t* buf, void* read_ele);
void buf_push_back_serial(send_recv_buf_t* buf, void* read_eles, int ele_counts);
void buf_pop_front(send_recv_buf_t* buf, void* write_ele);
bool buf_is_empty(send_recv_buf_t* buf);
bool buf_is_full(send_recv_buf_t* buf);

typedef enum bcs_type{
    BCS_START_TAG   = 0,
    BCS_END_TAG     = 1,
    BCS_COMMENT     = 2,
    BCS_PI          = 3,
    BCS_CDATA       = 4,
    BCS_DONE        = 5,
} bcs_type_t;

typedef struct bcs{
    bcs_type_t type;
    int64_t row;
    int64_t col;
    const char* head;
    int64_t size;
} bcs_t;

typedef struct event_node{
    event_t event;
    struct event_node* next;
} event_node_t;

typedef struct event_list{
    event_node_t* head;
    event_node_t* tail;
} event_list_t;

void event_init(event_t* event, event_type_t type, char* name, char* value);
void event_destroy(event_t* event);

void list_init(event_list_t* list);
void list_destroy(event_list_t* list);
void list_insert(event_list_t* list, event_t event);
event_list_t list_merge(event_list_t list_head, event_list_t list_tail);


#endif //__GLOBAL_H__
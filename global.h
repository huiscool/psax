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
#endif //__APPLE__

#ifdef DEBUG
#include <time.h>
#define debug_printf(...) printf(__VA_ARGS__)
#include <assert.h>
#else
#define debug_printf(...)
#define NDEBUG
#include <assert.h>
#endif //DEBUG


////////////////////////////////////////////////////////////////
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

void raise_error(error_type_t type, int64_t offset, char* msg);
void open_file(const char* filename, glov_t* glo);
void close_file(glov_t* glo);

////////////////////////////////////////////////////////////////
typedef struct send_recv_buf{
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    size_t max_size;
    size_t ele_size;
    size_t cur_size;
    int64_t front;
    int64_t back;
    void* buf;
}send_recv_buf_t;

int buf_init(send_recv_buf_t* buf, size_t size, size_t ele_size);
void buf_destroy(send_recv_buf_t* buf);
void buf_push_back(send_recv_buf_t* buf, void* read_ele);
void buf_push_back_serial(send_recv_buf_t* buf, void* read_eles, int ele_counts);
void buf_pop_front(send_recv_buf_t* buf, void* write_ele);
bool buf_is_empty(send_recv_buf_t* buf);
bool buf_is_full(send_recv_buf_t* buf);

////////////////////////////////////////////////////////////////
typedef enum bcs_type{
    BCS_START_TAG_BEGIN = 0,
    BCS_END_TAG_BEGIN   = 1,
    BCS_TAG_END         = 2,
    BCS_COMMENT_BEGIN   = 3,
    BCS_COMMENT_END     = 4,
    BCS_PI_BEGIN        = 5,
    BCS_PI_END          = 6,
    BCS_CDATA_BEGIN     = 7,
    BCS_CDATA_END       = 8,
    BCS_NONE            = 9,
} bcs_type_t;

typedef struct bcs_node{
    bcs_type_t type;
    char* p;
    struct bcs_node* next;
} bcs_node_t;

typedef struct bcs_list{
    bcs_node_t* head;
    bcs_node_t* tail;
} bcs_list_t;

void bcs_list_init(bcs_list_t* list);
void bcs_list_destroy(bcs_list_t* list);
void bcs_list_insert(bcs_list_t* list, char* p, bcs_type_t type);
bcs_list_t bcs_list_merge(bcs_list_t* list1, bcs_list_t* list2);

typedef struct preprocess_glov{
    pthread_mutex_t lock;
    pthread_barrier_t barrier;
    int np;
    char* buf;
    int64_t size;
    bcs_list_t* lists;
    bcs_list_t glo_blist;
} preprocess_glov_t;

void preprocess_glov_init(preprocess_glov_t* pre_glo, glov_t* glo);
void preprocess_glov_destroy(preprocess_glov_t* pre_glo);
bcs_list_t* preprocess(glov_t* glo);
////////////////////////////////////////////////////////////////
typedef struct event_node{
    event_t event;
    struct event_node* next;
} event_node_t;

typedef struct event_list{
    event_node_t* head;
    event_node_t* tail;
} event_list_t;

void event_list_init(event_list_t* list);
void event_list_destroy(event_list_t* list);
void event_list_insert(event_list_t* list, const event_t* event);
void event_list_insert_after(event_list_t* list, event_node_t* node, const event_t* event);
event_list_t event_list_merge(event_list_t* list1, event_list_t* list2);


typedef struct parse_glov{
    int np;
    event_list_t* elists;
    bcs_list_t* blists;
} parse_glov_t;
parse_glov_t par_glo;

void parse_glov_init(parse_glov_t* par_glo, glov_t* glo, bcs_list_t* blists);
void parse_glov_destroy(parse_glov_t* par_glo);
void loc_parse(event_list_t* elist, bcs_list_t* blist);
event_list_t* glo_parse(glov_t* glo, bcs_list_t* blists);

int element(char* p, char** next_pos, event_list_t* list);
int emptyelemtag(char* p, char** next_pos, event_list_t* list);
int attribute(char* p, char** next_pos, event_list_t* list);
int stag(char* p, char** next_pos, event_list_t* list);
int etag(char* p, char** next_pos, event_list_t* list);
int content(char* p, char** next_pos, event_list_t* list);
int comment(char* p, char** next_pos, event_list_t* list);
int pi(char* p, char** next_pos, event_list_t* list);
int pitarget(char* p, char** next_pos, event_list_t* list);
int cdsect(char* p, char** next_pos, event_list_t* list);
int cdstart(char* p, char** next_pos, event_list_t* list);
int cdata(char* p, char** next_pos, event_list_t* list);
int cdend(char* p, char** next_pos, event_list_t* list);
int namestartchar(char* p, char** next_pos, event_list_t* list);
int namechar(char* p, char** next_pos, event_list_t* list);
int name(char* p, char** next_pos, event_list_t* list);
int attvalue(char* p, char** next_pos, event_list_t* list);
int reference(char* p, char** next_pos, event_list_t* list);
int entityref(char* p, char** next_pos, event_list_t* list);
int charref(char* p, char** next_pos, event_list_t* list);
int chardata(char* p, char** next_pos, event_list_t* list);
int eq(char* p, char** next_pos, event_list_t* list);
int space(char* p, char** next_pos, event_list_t* list);
int charc(char* p, char** next_pos, event_list_t* list);


////////////////////////////////////////////////////////////////
typedef struct event_stack_node{
    event_t event;
    struct event_stack_node* prev;
    struct event_stack_node* next;
} event_stack_node_t;

typedef struct event_stack{
    int n;
    event_stack_node_t* top;
} event_stack_t;

void stack_init(event_stack_t* stack);
void stack_destroy(event_stack_t* stack);
event_t* stack_top(event_stack_t* stack);
void stack_pop(event_stack_t* stack);
void stack_push(event_stack_t* stack, const event_t* event);
bool stack_is_empty(event_stack_t* stack);

event_list_t post_process(glov_t* glo, event_list_t* elists);


#endif //__GLOBAL_H__
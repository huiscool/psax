#include "global.h"
int buf_init(send_recv_buf_t* buf, size_t max_size, size_t ele_size){
    if(pthread_mutex_init(&(buf->lock), NULL) != 0)return -1;
    if(pthread_cond_init(&(buf->not_empty), NULL) != 0)return -1;
    if(pthread_cond_init(&(buf->not_full), NULL) != 0)return -1;
    buf->max_size = (max_size/ele_size) * ele_size;//make sure ele_size divides size
    buf->ele_size = ele_size;
    buf->cur_size = 0;
    buf->front = 0;
    buf->back = 0;
    buf->buf = malloc(buf->max_size);
    if(buf->buf == 0)return -1;
    else return 0;
}
void buf_destroy(send_recv_buf_t* buf){
    pthread_mutex_destroy(&(buf->lock));
    pthread_cond_destroy(&(buf->not_empty));
    pthread_cond_destroy(&(buf->not_full));
    free(buf->buf);
}
void buf_push_back(send_recv_buf_t* buf, void* read_ele){
    int empty = 0;
    pthread_mutex_lock(&(buf->lock));
    while(buf_is_full(buf)){
        pthread_cond_wait(&(buf->not_full), &(buf->lock));
    }
    int64_t back = buf->back;
    int64_t ele_size = buf->ele_size;
    memcpy(&(buf->buf[back]), read_ele, ele_size);
    buf->back = (back + ele_size >= buf->max_size) ? 0 : (back + ele_size);
    if(buf_is_empty(buf)) empty = 1;
    buf->cur_size += ele_size;
    pthread_mutex_unlock(&(buf->lock));
    if(empty){//after push back, queue become not empty
        pthread_cond_signal(&(buf->not_empty));
    }
}
void buf_push_back_serial(send_recv_buf_t* buf, void* read_eles, int ele_counts){
    //TODO
}
void buf_pop_front(send_recv_buf_t* buf, void* write_ele){
    int full = 0;
    pthread_mutex_lock(&(buf->lock));
    while(buf_is_empty(buf)){
        pthread_cond_wait(&(buf->not_empty), &(buf->lock));
    }
    int64_t front = buf->front;
    int64_t ele_size = buf->ele_size;
    memcpy(write_ele, &(buf->buf[front]), ele_size);
    buf->front = (front + ele_size >= buf->max_size) ? 0 : (front + ele_size);
    if(buf_is_full(buf)) full = 1;
    buf->cur_size -= ele_size;
    pthread_mutex_unlock(&(buf->lock));
    if(full){//after popping, queue become not full
        pthread_cond_signal(&(buf->not_full));
    }
}
inline bool buf_is_empty(send_recv_buf_t* buf){
    return buf->cur_size == 0;
}
inline bool buf_is_full(send_recv_buf_t* buf){
    return buf->cur_size == buf->max_size;
}
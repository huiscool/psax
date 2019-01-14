#include "global.h"

void raise_error(error_type_t type, int64_t offset, char* msg){
    glo_error.type = type;
    int row = 1;
    int col = 1;
    for(int i=0; i<offset; i++,col++){
        if(glo.file_buf[i] == '\n'){
            row++;
            col=0;
        }
    }
    glo_error.row = row;
    glo_error.col = col;
    if(msg != glo_error.msg)strcpy(glo_error.msg, msg);
    glo.error_handler(&glo_error);
}

void open_file(const char* filename, glov_t* glo){
    ////get file descriptor
    int fd = open(filename, O_RDWR);
    if(fd == -1){
        sprintf(glo_error.msg, "%s cannot open", filename);
        raise_error(FILE_OPEN_ERROR, -1, glo_error.msg);
        return;
    }
    ////get file size
    struct stat st;
    fstat(fd, &st);
    glo->file_size = st.st_size;
    ////mmap the file into file_buf
    glo->file_buf = mmap(NULL, glo->file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if(glo->file_buf == (char*)-1){
        sprintf(glo_error.msg, "%s cannot open", filename);
        raise_error(FILE_OPEN_ERROR, -1, glo_error.msg);
    }
    return;
}
void close_file(glov_t* glo){
    close(glo->fd);
}

int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename){
#ifdef PERFORMANCE
    struct timespec start, finish;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif //PERFORMANCE
    glo.error_handler = error_handler;
    glo.event_handler = event_handler;
    glo.np = thread_num;
    int np = thread_num;
    if(thread_num <= 0){
        sprintf(glo_error.msg, "thread_num is: %d",thread_num);
        raise_error(THREAD_NUM_ERROR, -1, glo_error.msg);
        return -1;
    }
    open_file(filename, &glo);
    
#ifdef SERIAL
    event_list_t list;
    event_list_init(&list);
    char* a = glo.file_buf;
    //char* a = "</a>  ";
    int res = content(a, &a, &list);
    event_list_t final_list = post_process(&glo, &list);
    printf("res:%d, a:%s\n", res, a);
#endif //SERIAL

#ifdef PARALLEL
    bcs_list_t* blists = preprocess(&glo);
    event_list_t* elists = glo_parse(&glo, blists);
    event_list_t final_list = post_process(&glo, elists);

#endif //PARALLEL
#ifdef PERFORMANCE
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("parse time: %f ms\n", elapsed);
#endif //PERFORMANCE
    event_node_t* p = final_list.head;
    while(p != NULL){
        event_handler(&(p->event));
        p = p->next;
    }
    close_file(&glo);

    return 0;
}

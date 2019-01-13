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

void* preprocess_workload(void* p_void){
    int proprocess_done = 0;
    int64_t row = 1;
    int64_t col = 0;
    char* p = glo.file_buf;
    while(!proprocess_done){
        bcs_t res = produce_bcs_iterator(p, &p, &glo);
    }
    return (void*)0;
}

void* parse_workload(void* p_void){
    int parse_done = 0;
    while(!parse_done){

    }
    return (void*)0;
}

void* postprocess_workload(void* p_void){
    int postprocess_done = 0;
    while(!postprocess_done){

    }
    return (void*)0;
}

void serial_parse(){
    int proprocess_done = 0;
    int64_t row = 1;
    int64_t col = 0;
    char* p = glo.file_buf;
    while(!proprocess_done){
        bcs_t res = produce_bcs_iterator(p, &p, &glo);

        if(res.type == BCS_DONE)break;
    }
}

int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename){
#ifdef DEBUG
    clock_t start_time = clock();
#endif //DEBUG
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

#ifdef TEST_1 //preprocess
    bcs_t* chunks = malloc(np * sizeof(bcs_t));
    produce_bcs_chunks(chunks, &glo);
    free(chunks);
#endif //TEST_1

#ifdef TEST_2 //parse
    //char* a = "<? pi example 20<30 ?>";
    //char* p = a;
    char* p = glo.file_buf;
    event_list_t list;
    event_list_init(&list);
    printf("%d:%s\n", content(p, &p, &list), p);
#ifdef DEBUG
    clock_t end_time = clock();
    double dur = 1000.0 * (end_time - start_time) / CLOCKS_PER_SEC;
#endif //DEBUG
    event_node_t* node_p = list.head;
    while(node_p!= NULL){
        event_handler(&(node_p->event));
        node_p = node_p->next;
    }
#endif //TEST_2
    close_file(&glo);
    debug_printf("time: %lf ms\n", dur);
    return 0;
}

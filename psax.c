#include "global.h"

void raise_error(error_type_t type, int64_t row, int64_t col, char* msg){
    glo_error.type = type;
    glo_error.row = row;
    glo_error.col = col;
    if(msg != glo_error.msg)strcpy(glo_error.msg, msg);
    error_handler(&glo_error);
}

void open_file(const char* filename){
    ////get file descriptor
    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        sprintf(glo_error.msg, "%s cannot open", filename);
        raise_error(FILE_OPEN_ERROR, -1,-1, glo_error.msg);
        return;
    }
    ////get file size
    struct stat st;
    fstat(fd, &st);
    glo.file_size = st.st_size;
    ////mmap the file into file_buf
    glo.file_buf = mmap(NULL, glo.file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(glo.file_buf == (byte_t*)-1){
        sprintf(glo_error.msg, "%s cannot open", filename);
        raise_error(FILE_OPEN_ERROR, -1,-1, glo_error.msg);
    }
    return;
}

void close_file(){
    close(glo.fd);
}

void* preprocess_workload(void* p_void){

}

void* parse_workload(void* p_void){

}

void* postprocess_workload(void* p_void){

}

int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename){
    if(thread_num <= 0){
        sprintf(glo_error.msg, "thread_num is: %d",thread_num);
        raise_error(THREAD_NUM_ERROR, -1, -1, glo_error.msg);
        return -1;
    }
    open_file(filename);
#ifdef TEST_1 //test open file
    for(int64_t i=0; i<glo.file_size; i++){
        printf("%c",glo.file_buf[i]);
    }
    printf("\n");
    return 0;
#else //TEST_1

    close_file();
#endif //TEST_1
    return 0;
}

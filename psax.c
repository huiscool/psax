#include "global.h"

void raise_error(error_type_t type, int64_t row, int64_t col, char* msg){
    glo_error.type = type;
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
        raise_error(FILE_OPEN_ERROR, -1,-1, glo_error.msg);
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
        raise_error(FILE_OPEN_ERROR, -1,-1, glo_error.msg);
    }
    return;
}
void close_file(glov_t* glo){
    close(glo->fd);
}

int buf_init(send_recv_buf_t* buf, size_t max_size, size_t ele_size){
    if(pthread_mutex_init(&(buf->lock), NULL) != 0)return -1;
    if(pthread_rwlock_init(&(buf->rwlock), NULL) != 0)return -1;
    if(pthread_cond_init(&(buf->cond_empty), NULL) != 0)return -1;
    if(pthread_cond_init(&(buf->cond_full), NULL) != 0)return -1;
    buf->max_size = (max_size/ele_size) * ele_size;//make sure ele_size divides size
    buf->ele_size = ele_size;
    buf->cur_size = 0;
    buf->front = 0;
    buf->back = 0;
    buf->buf = malloc(buf->size);
    if(buf->buf == 0)return -1;
    else return 0;
}
void buf_destroy(send_recv_buf_t* buf){
    pthread_mutex_destroy(&(buf->lock));
    pthread_rwlock_destroy(&(buf->rwlock));
    pthread_cond_destroy(&(buf->cond_empty));
    pthread_cond_destroy(&(buf->cond_full));
    free(buf->buf);
}
int buf_push_back(send_recv_buf_t* buf, void* read_ele){
    if(!buf_is_full(buf)){
        int64_t back = buf->back;
        int64_t ele_size = buf->ele_size;
        memcpy(&(buf->buf[back], read_ele, ele_size);
        buf->back = (back + ele_size >= buf->max_size) ? 0 : (back + ele_size);
        buf->cur_size += ele_size;
        return 0;
    }// not thread-safe
    else return -1;
}
void buf_push_back_serial(send_recv_buf_t* buf, void* read_eles, int ele_counts){
    
}
void buf_pop_front(send_recv_buf_t* buf, void* write_ele){
    if(!buf_is_empty(buf)){
        int64_t front = buf->front;
        int64_t ele_size = buf->ele_size;
        memcpy(write_ele, &buf->buf[front], ele_size);
        buf->front = (front + ele_size >= buf->max_size) ? 0 : (front + ele_size);
        buf->cur_size -= ele_size;
        return 0;
    }
    else return -1;
}
bool buf_is_empty(send_recv_buf_t* buf){
    return buf->cur_size == 0;
}
bool buf_is_full(send_recv_buf_t* buf){
    return buf->cur_size == buf->max_size;
}

bcs_t produce_bcs_iterator(char* p, int64_t row, int64_t col, char** next_p , int64_t* next_row, int64_t* next_col){
    static const unsigned char special_delimiter_counts = 3;
    static bcs_type_t delimiter_type[] = {
        BCS_COMMENT,
        BCS_PI,
        BCS_CDATA,
    };
    static char* begin_delimiters[] = {
        "<!--",
        "<?",
        "<![CDATA[",
    };
    static unsigned char begin_delimiter_len[] = {
        4, 2, 9,
    };
    static char* end_delimiters[] = {
        "-->",
        "?>",
        "]]>",
    };
    static unsigned char end_delimiter_len[] = {
        3, 2, 3,
    };
    while(*p != '<' && *p != '\0'){
        p++;// look for the next '<'
        col++;
        if(*p == '\n'){
            col = -1;
            row++;
        }
    }
    if(*p == '\0'){// when p reach the end of file
        return (bcs_t){
            .type   = BCS_DONE,
            .row    = row,
            .col    = col,
            .head   = "",
            .size   = 0,
        };
    }
    bcs_t res;
    char* head = p;
    for(int i=0; i<special_delimiter_counts; i++){//preprocess the special bcs
        if(strncmp(head, begin_delimiters[i], begin_delimiter_len[i]) == 0){
            res.type = delimiter_type[i];
            res.head = head;
            res.row = row;
            res.col = col;
            int find_tail = 0;
            while(!find_tail){
                while(*p != '>' && *p != '\0'){// look for the next '>'
                    p++;
                    col++;
                    if(*p == '\n'){
                        col = -1;
                        row++;
                    }
                }
                if(*p == '\0'){// cannot find the tail
                    raise_error(SYNTAX_ERROR, res.row, res.col, "cannot find matched symbol");
                }
                char* end_delimiter_head = p - (end_delimiter_len[i]-1);
                if(strncmp(end_delimiter_head, end_delimiters[i], end_delimiter_len[i]) == 0){
                    find_tail = 1;
                }else{
                    p++;//when cannot find the tail, go on;
                    col++;
                }
            }
            while(*p != '<' && *p != '\0'){
                p++;// look for the next '<'
                col++;
                if(*p == '\n'){
                    col = -1;
                    row++;
                }
            }
            res.size = p - head;
            *next_p = p;
            *next_row = row;
            *next_col = col;
            return res;
        }
    }
    if(head[1] == '/'){// match the begin_tag and end_tag
        res.type = BCS_END_TAG;
    }else{
        res.type = BCS_START_TAG;
    }
    res.head = head;
    res.row = row;
    res.col = col;
    p++;
    while(*p != '<' && *p != '\0'){
        p++;// look for the next '<'
        col++;
        if(*p == '\n'){
            col = -1;
            row++;
        }
    }
    res.size = p - head;
    *next_p = p;
    *next_row = row;
    *next_col = col;
    return res;
}

void* preprocess_workload(void* p_void){
    int proprocess_done = 0;
    int64_t row = 1;
    int64_t col = 0;
    char* p = glo.file_buf;
    while(!proprocess_done){
        bcs_t res = produce_bcs_iterator(p, row, col, &p, &row, &col);
        if(res.type == BCS_DONE){proprocess_done = 1;break;}
        printf("row: %lld col:%lld\n",res.row, res.col);
        printf("type:%d\n",res.type);
        for(int i=0; i<res.size; i++){
            printf("%c",res.head[i]);
        }
        printf("\n");
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

int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename){
#ifdef DEBUG
    clock_t start_time = clock();
#endif //DEBUG
    glo.error_handler = error_handler;
    glo.event_handler = event_handler;
    glo.np = thread_num;
    if(thread_num <= 0){
        sprintf(glo_error.msg, "thread_num is: %d",thread_num);
        raise_error(THREAD_NUM_ERROR, -1, -1, glo_error.msg);
        return -1;
    }
    open_file(filename, &glo);

    close_file(&glo);

#ifdef DEBUG
    clock_t end_time = clock();
    double dur = 1000.0 * (end_time - start_time) / CLOCKS_PER_SEC;
    debug_printf("time: %lf ms\n", dur);
#endif //DEBUG
    return 0;
}

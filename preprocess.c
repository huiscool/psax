#include "global.h"
bcs_t produce_bcs_iterator(char* p, char** next_p, glov_t* glo){
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
    }
    if(*p == '\0'){// when p reach the end of file
        return (bcs_t){
            .type   = BCS_DONE,
            .offset = p - glo->file_buf,
            .head   = "",
            .size   = 0,
        };
    }
    char* head = p;
    for(int i=0; i<special_delimiter_counts; i++){//preprocess the special bcs
        if(strncmp(head, begin_delimiters[i], begin_delimiter_len[i]) == 0){
            int find_tail = 0;
            while(!find_tail){
                while(*p != '>' && *p != '\0'){// look for the next '>'
                    p++;
                }
                if(*p == '\0'){// cannot find the tail
                    raise_error(SYNTAX_ERROR, p - glo->file_buf, "cannot find matched symbol");
                }
                char* end_delimiter_head = p - (end_delimiter_len[i]-1);
                if(strncmp(end_delimiter_head, end_delimiters[i], end_delimiter_len[i]) == 0){
                    find_tail = 1;
                }else{
                    p++;//when cannot find the tail, go on;
                }
            }
            while(*p != '<' && *p != '\0'){
                p++;// look for the next '<'
            }
            *next_p = p;
            return (bcs_t){
                .type   = delimiter_type[i],
                .offset = p - glo->file_buf,
                .head   = head,
                .size   = p-head,
            };
        }
    }
    p++;
    while(*p != '<' && *p != '\0'){
        p++;// look for the next '<'
    }
    *next_p = p;
    return (bcs_t){
        .type   = head[1]=='/' ? BCS_END_TAG : BCS_START_TAG ,
        .offset = p - glo->file_buf,
        .head   = head,
        .size   = p - head,
    };
}

void delimiter_list_init(delimiter_list_t* list){
    list->head = NULL;
    list->tail = NULL;
}

void delimiter_list_destroy(delimiter_list_t* list){
    delimiter_node_t* p = list->head;
    while(p!=NULL){
        delimiter_node_t* next = p->next;
        free(p);
        p = next;
    }
}
void delimiter_list_insert(delimiter_list_t* list, const char* p){
    delimiter_node_t* new_node = malloc(sizeof(delimiter_node_t));
    new_node->p = p;
    new_node->next = NULL;
    if(list->tail == NULL){
        list->head = new_node;
        list->tail = new_node;
    }else{
        list->tail->next = new_node;
        list->tail = new_node;
    }
}
delimiter_list_t delimiter_list_merge(delimiter_list_t* list1, delimiter_list_t* list2){
    if(list1->tail == NULL){
        return (delimiter_list_t){
            .head = list2->head,
            .tail = list2->tail,
        };
    }
    list1->tail->next = list2->head;
    return (delimiter_list_t){
        .head = list1->head,
        .tail = list2->tail,
    };
}

void* bcs_chunk_workload(void* p_glov){
    const unsigned char special_delimiter_counts = 3;
    const unsigned char flag_full = 7;
    static char* end_delimiters[] = {"-->","?>","]]>",};
    static int end_delimiter_len[] = {3, 2, 3,};
    static bcs_type_t delimiter_type[] = {BCS_COMMENT,BCS_PI,BCS_CDATA,};
    static char* begin_delimiters[] = {"<!--","<?","<![CDATA[",};
    static unsigned char begin_delimiter_len[] = {4, 2, 9,}; 
    preprocess_glov_t* glo = (preprocess_glov_t*)p_glov;
    int np = glo->np;
    char* buf = glo->buf;
    int64_t size = glo->size;
    static int atomic_id = 0;
    int tid = __sync_fetch_and_add(&atomic_id, 1);
    int64_t loc_size = size / np;
    int64_t m_offset = tid * loc_size;
    if(tid == np - 1)loc_size = size - m_offset;
    int64_t m_end = m_offset + loc_size;
    delimiter_list_t list;
    delimiter_list_init(&list);
    int flag = 0;
    delimiter_node_t* scan_begin;
    for(int64_t i=m_offset; i<m_end; i++){
        if(buf[i] == '<'){
            delimiter_list_insert(&list, &buf[i]);
        }
        if(buf[i] == '>'){
            for(int k=0; k<special_delimiter_counts; k++){
                int64_t j = i + 1 - end_delimiter_len[k];
                if(strncmp(&buf[j], end_delimiters[k], end_delimiter_len[k]) == 0){
                    delimiter_list_insert(&list, &buf[j]);
                    flag |= 1<<k;
                    if(flag == flag_full){
                        scan_begin = list.tail;
                        flag |= 1<<3; // make flag no longer == flag_full
                    }
                    break;
                }
            }
        }
    }
    glo->lists[tid] = list;
    pthread_barrier_wait(&(glo->barrier));
    if(tid == 0){
        list.head = NULL;
        list.tail = NULL;
    }
    pthread_barrier_wait(&(glo->barrier));
    delimiter_list_destroy(&list);
    return (void*)0;
}

void preprocess_glov_init(preprocess_glov_t* pre_glo, glov_t* glo, bcs_t* chunks){
    int np = glo->np;
    pthread_mutex_init(&(pre_glo->lock), NULL);
    pthread_barrier_init(&(pre_glo->barrier), NULL, np);
    pre_glo->np = np;
    pre_glo->buf = glo->file_buf;
    pre_glo->size = glo->file_size;
    pre_glo->begins = malloc(np * sizeof(delimiter_node_t*));
    pre_glo->ends = malloc(np * sizeof(delimiter_node_t*));
    pre_glo->lists = malloc(np * sizeof(delimiter_list_t));
    pre_glo->chunks = chunks;
}

void preprocess_glov_destroy(preprocess_glov_t* pre_glo){
    pthread_mutex_destroy(&(pre_glo->lock));
    pthread_barrier_destroy(&(pre_glo->barrier));
    free(pre_glo->begins);
    free(pre_glo->ends);
    free(pre_glo->lists);
}

void produce_bcs_chunks(bcs_t* chunks, glov_t* glo){
    preprocess_glov_t pre_glo;
    preprocess_glov_init(&pre_glo, glo, chunks);
    int np = glo->np;
    pthread_t threads[np];
    for(int i=0; i<np; i++){
        pthread_create(&threads[i], NULL, bcs_chunk_workload, &pre_glo);
    }
    for(int i=0; i<np; i++){
        pthread_join(threads[i], NULL);
    }
    preprocess_glov_destroy(&pre_glo);
}
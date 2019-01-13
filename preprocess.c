#include "global.h"

void bcs_list_init(bcs_list_t* list){
    list->head = NULL;
    list->tail = NULL;
}
void bcs_list_destroy(bcs_list_t* list){
    bcs_node_t* p = list->head;
    while(p!=NULL){
        bcs_node_t* next = p->next;
        free(p);
        p = next;
    }
}
void bcs_list_insert(bcs_list_t* list, const char* p, bcs_type_t type){
    bcs_node_t* new_node = malloc(sizeof(bcs_node_t));
    new_node->p = p;
    new_node->type = type;
    new_node->next = NULL;
    if(list->tail == NULL){
        list->head = new_node;
        list->tail = new_node;
    }else{
        list->tail->next = new_node;
        list->tail = new_node;
    }
}
bcs_list_t bcs_list_merge(bcs_list_t* list1, bcs_list_t* list2){
    if(list1->tail == NULL){
        return (bcs_list_t){
            .head = list2->head,
            .tail = list2->tail,
        };
    }
    list1->tail->next = list2->head;
    return (bcs_list_t){
        .head = list1->head,
        .tail = list2->tail,
    };
}
void preprocess_glov_init(preprocess_glov_t* pre_glo, glov_t* glo){
    int np = glo->np;
    char* buf = glo->file_buf;
    int64_t size = glo->file_size;
    pthread_mutex_init(&(pre_glo->lock), NULL);
    pthread_barrier_init(&(pre_glo->barrier), NULL, np);
    pre_glo->buf = buf;
    pre_glo->np = np;
    pre_glo->size = size;
    pre_glo->lists = malloc(np * sizeof(bcs_list_t));
}
void preprocess_glov_destroy(preprocess_glov_t* pre_glo){
    pthread_mutex_destroy(&(pre_glo->lock));
    pthread_barrier_destroy(&(pre_glo->barrier));
    free(pre_glo->lists);
}

bcs_type_t recognizer(char* p){
    if(*p == '<'){
        if(!strncmp(p, "<!", 2))return BCS_END_TAG_BEGIN;
        if(!strncmp(p, "<?", 2))return BCS_PI_BEGIN;
        if(!strncmp(p, "<!--", 4))return BCS_COMMENT_BEGIN;
        if(!strncmp(p, "<![CDATA[", 9))return BCS_CDATA_BEGIN;
        return BCS_START_TAG_BEGIN;
    }else if(*p == '>'){
        p-=1;
        if(!strncmp(p, "?>", 2))return BCS_PI_END;
        p+=1;
        p-=2;
        if(!strncmp(p, "-->", 3))return BCS_COMMENT_END;
        p+=2;
        p-=2;
        if(!strncmp(p, "]]>", 3))return BCS_CDATA_END;
        p+=2;
        return BCS_TAG_END;
    }
    return BCS_NONE;
}


void create_global_bcs_list(preprocess_glov_t* pre_glo, int tid){
    int np = pre_glo->np;
    char* buf = pre_glo->buf;
    int64_t m_size = pre_glo->size / np;
    int64_t m_begin = tid * m_size;
    int64_t m_end = (tid+1) * m_size;
    if(tid == np - 1) m_end = pre_glo->size;
    bcs_list_t loc_list;
    for(int64_t i=m_begin; i<m_end; i++){
        if(buf[i] == '<' || buf[i] == '>'){
            bcs_type_t type = recognizer(&buf[i]);
            if(type != BCS_TAG_END) bcs_list_insert(&loc_list, &buf[i], type);
        }
    }
    pre_glo->lists[tid] = loc_list;
    pthread_barrier_wait(&(pre_glo->barrier));
    if(tid == 0){
        bcs_list_init(&(pre_glo->glo_blist));
        for(int i=0; i<np; i++){
            pre_glo->glo_blist = bcs_list_merge(&(pre_glo->glo_blist), &(pre_glo->lists[i]));
        }
    }
}

void delete_wrong_bcs_in_glo_blist(preprocess_glov_t* pre_glo, int tid){
    if(tid == 0){
        char* buf = pre_glo->buf;
        bcs_node_t* ps = pre_glo->glo_blist.head;
        while(ps != NULL){
            bcs_type_t ps_type = ps->type;
            bcs_node_t* pe = ps->next;
            if(ps_type == BCS_CDATA_BEGIN || ps_type == BCS_PI_BEGIN || ps_type == BCS_COMMENT_BEGIN){
                if(pe == NULL)raise_error(SYNTAX_ERROR, ps->p - buf, "cdata, pi, comment not matched");
                while(pe != NULL){
                    bcs_type_t pe_type = pe->type;
                    if(pe_type == ps_type + 1){
                        break;
                    }else{
                        bcs_node_t* tmp = pe;
                        pe = pe->next;
                        free(tmp);
                    }
                }
                ps->next = pe;
                ps= ps->next;
                continue;
            }
            if( pe!= NULL && ( pe->type == BCS_TAG_END || pe->type == BCS_COMMENT_END || pe->type == BCS_PI_END || pe->type == BCS_CDATA_END )){
                bcs_node_t* tmp = pe;
                ps->next = pe->next;
                free(tmp);
                continue;
            }
            ps = ps->next;
        }
    }
}

void divide_global_bcs_list(preprocess_glov_t* pre_glo, int tid){
    if(tid == 0){
        int np = pre_glo->np;
        char* buf = pre_glo->buf;
        int64_t size = pre_glo->size;
        bcs_node_t* ps = pre_glo->glo_blist.head;
        bcs_node_t* pe = ps->next;
        int64_t loc_size = pre_glo->size / np;
        pre_glo->lists[0].head = ps;
        for(int i=1; i<np; i++){
            int64_t offset = i * loc_size;
            while(pe != NULL){
                if(ps->p <= &buf[offset] && &buf[offset] < pe->p){
                    pre_glo->lists[i-1].tail = ps;
                    ps->next = NULL;
                    pre_glo->lists[i].head = pe;
                    ps = pe;
                    pe = pe->next;
                    break;
                }
                ps = pe;
                pe = pe->next;
            }
        }
        ps = pe;
        pe = pe->next;
        while(pe != NULL){
            ps = pe;
            pe = pe->next;
        }
        pre_glo->lists[np-1].tail = ps;
        ps->next = NULL;
        //忘记插入末尾的标志位了,这里插入
        for(int i=0; i<np-1; i++){
            bcs_node_t* phead = pre_glo->lists[i+1].head;
            bcs_list_insert(&(pre_glo->list[i]), phead->p, phead->type);
        }
        bcs_list_insert(&(pre_glo->list[i]), buf+size, BCS_NONE);
    }
}

void* preprocess_workload(void* p_pre_glo){
    preprocess_glov_t* pre_glo = p_pre_glo;
    static int id = 0;
    int tid = __sync_fetch_and_add(&id, 1);
    create_global_bcs_list(pre_glo, tid);
    delete_wrong_bcs_in_glo_blist(pre_glo, tid);
    divide_global_bcs_list(pre_glo, tid);
    return (void*)0;
}

bcs_list_t* preprocess(glov_t* glo){
    int np = glo->np;
    preprocess_glov_t pre_glo;
    preprocess_glov_init(&pre_glo, glo);
    pthread_t threads[np];
    for(int i=0; i<np; i++){
        pthread_create(&threads[i], NULL, preprocess_workload, &pre_glo);
    }
    for(int i=0; i<np; i++){
        pthread_join(threads[i], NULL);
    }
    bcs_list_t* res = malloc( np * sizeof(bcs_list_t));
    memcpy(res, pre_glo.lists, np * sizeof(bcs_list_t));
    preprocess_glov_destroy(&pre_glo);
    return res;
}
#include "global.h"

void event_list_init(event_list_t* list){
    list->head = NULL;
    list->tail = NULL;
}
void event_list_destroy(event_list_t* list){
    event_node_t* p = list->head;
    while(p!=NULL){
        event_node_t* next = p->next;
        free(p);
        p = next;
    }
}
void event_list_insert(event_list_t* list, const event_t* event){
    event_node_t* new_node = malloc(sizeof(event_node_t));
    memcpy(&(new_node->event), event, sizeof(event_t));
    new_node->next = NULL;
    if(list->tail == NULL){
        list->head = new_node;
        list->tail = new_node;
    }else{
        list->tail->next = new_node;
        list->tail = new_node;
    }
}
void event_list_insert_after(event_list_t* list, event_node_t* node, const event_t* event){
    event_node_t* new_node = malloc(sizeof(event_node_t));
    memcpy(&(new_node->event), event, sizeof(event_t));
    if(node == NULL){
        new_node->next = list->head;
        list->head = new_node;
        if(list->tail == NULL)list->tail = new_node;
    }else{
        if(node == list->tail)list->tail = new_node;
        new_node->next = node->next;
        node->next = new_node;
    }
}
event_list_t event_list_merge(event_list_t* list1, event_list_t* list2){
    if(list1->tail == NULL){
        return (event_list_t){
            .head = list2->head,
            .tail = list2->tail,
        };
    }
    list1->tail->next = list2->head;
    return (event_list_t){
        .head = list1->head,
        .tail = list2->tail,
    };
}

void parse_glov_init(parse_glov_t* par_glo, glov_t* glo, bcs_list_t* blists){
    int np = glo->np;
    par_glo->np = np;
    par_glo->elists = malloc(np * sizeof(event_list_t));
    for(int i=0; i<np; i++){
        event_list_init(&(par_glo->elists[i]));
    }
    par_glo->blists = blists;
}

void parse_glov_destroy(parse_glov_t* par_glo){
    free(par_glo->elists);
}

int element(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    int flag = 0;
    if(emptyelemtag(p, &p, list)){
    }else{
        if(!stag(p, &p, list))return flag;
        if(!content(p, &p, list))return flag;
        if(!etag(p, &p, list))return flag;
    }
    flag = 1;
    *next_pos = p;
    return flag;
}
int tag_helper(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    int flag = 0;
    if(!space(p, &p, list))return flag;
    if(!attribute(p, &p, list))return flag;
    flag = 1;
    *next_pos = p;
    return flag;
}
int emptyelemtag(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(*p != '<')return flag;
    p++;
    char* tag_name = p;
    if(!name(p, &p, list))return flag;
    int tag_name_len = p - tag_name;
    event_t empty_tag_event={
        .type       = EVENT_EMPTY_ELEMENT,
        .offset     = head - glo.file_buf,
        .name       = tag_name,
        .name_len   = tag_name_len,
        .value      = head,
        .value_len  = 0,
    };
    event_node_t* insert_point = list->tail;
    while(tag_helper(p, &p, list));
    space(p, &p, list);
    if(strncmp(p, "/>", 2) != 0){
        //失配, 把之前插入事件流的属性清空
        if(insert_point != NULL){
            event_node_t* p_ev = insert_point->next;
            insert_point->next = NULL;
            list->tail = insert_point;
            while(p_ev != NULL){
                event_node_t* tmp = p_ev;
                p_ev = p_ev->next;
                free(tmp);
            }
        }
        return flag;
    }
    p += 2;
    event_list_insert_after(list, insert_point, &empty_tag_event);
    flag = 1;
    *next_pos = p;
    return flag;
}
int attribute(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    char* attr_name = p;
    if(!name(p, &p, list))return flag;
    int attr_name_len = p - attr_name;
    if(!eq(p, &p, list))return flag;
    char* attr_value = p;
    if(!attvalue(p, &p, list))return flag;
    int attr_value_len = p - attr_value;
    event_t attr_event={
        .type       = EVENT_ATTRIBUTE,
        .offset     = head - glo.file_buf,
        .name       = attr_name,
        .name_len   = attr_name_len,
        .value      = attr_value,
        .value_len  = attr_value_len,
    };
    event_list_insert(list, &attr_event);
    flag = 1;
    *next_pos = p;
    return flag;
}
int stag(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(*p != '<')return flag;
    p++;
    char* tag_name = p;
    if(!name(p, &p, list))return flag;
    int tag_name_len = p - tag_name;
    event_t start_tag_event={
        .type       = EVENT_ELEMENT_BEGIN,
        .offset     = head - glo.file_buf,
        .name       = tag_name,
        .name_len   = tag_name_len,
        .value      = head,
        .value_len  = 0,
    };
    event_node_t* insert_point = list->tail;
    while(tag_helper(p, &p, list));
    space(p, &p, list);
    if(*p != '>'){
        //失配, 把之前插入事件流的属性清空
        if(insert_point != NULL){
            event_node_t* p_ev = insert_point->next;
            insert_point->next = NULL;
            list->tail = insert_point;
            while(p_ev != NULL){
                event_node_t* tmp = p_ev;
                p_ev = p_ev->next;
                free(tmp);
            }
        }
        return flag;
    }
    p++;
    event_list_insert_after(list, insert_point, &start_tag_event);
    flag = 1;
    *next_pos = p;
    return flag;
}
int etag(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(strncmp(p, "</", 2) != 0)return flag;
    p += 2;
    char* tag_name = p;
    if(!name(p, &p, list))return flag;
    int tag_name_len = p - tag_name;
    event_t end_tag_event={
        .type       = EVENT_ELEMENT_END,
        .offset     = head - glo.file_buf,
        .name       = tag_name,
        .name_len   = tag_name_len,
        .value      = head,
        .value_len  = 0,
    };
    event_list_insert(list, &end_tag_event);
    space(p, &p, list);
    if(*p != '>')return flag;
    p++;
    flag = 1;
    *next_pos = p;
    return flag;
}
int content_helper(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    int flag = 0;
    if(element(p, next_pos, list)|| reference(p, next_pos, list) || cdsect(p, next_pos, list) || pi(p, next_pos, list) || comment(p, next_pos, list)){
        p = *next_pos;
        chardata(p, &p, list);
        *next_pos = p;
        flag = 1;
        return flag;
    }
    *next_pos = head;
    return flag;
}
int content(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    int flag = 0;
    chardata(p, &p, list);
    while(content_helper(p, &p, list));
    *next_pos = p;
    flag = 1;
    return flag;
}
int comment_helper(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    int flag = 0;
    if(*p == '-')p++;
    if(*p == '-')return flag;
    if(!charc(p, &p, list))return flag;
    *next_pos = p;
    flag = 1;
    return flag;
}
int comment(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(strncmp(p, "<!--", 4) != 0)return flag;
    p += 4;
    char* value = p;
    while(comment_helper(p, &p, list));
    int value_len = p - value;
    if(strncmp(p, "-->", 3) != 0)return flag;
    p += 3;
    event_t comment_event={
        .type       = EVENT_COMMENT,
        .offset     = head - glo.file_buf,
        .name       = head,
        .name_len   = 0,
        .value      = value,
        .value_len  = value_len,
    };
    event_list_insert(list, &comment_event);
    *next_pos = p;
    flag = 1;
    return flag;
}
int pi_helper(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    int flag = 0;
    if(!space(p, &p, list))return flag;
    while(1){
        if(*p == '?' && strncmp(p, "?>", 2) == 0)break;
        else if(charc(p, &p, list))continue;
        else break;
    }
    *next_pos = p;
    flag = 1;
    return flag; 
}
int pi(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(strncmp(p, "<?", 2) != 0)return flag;
    p += 2;
    while(space(p, &p, list));
    char* pi_name = p;
    if(!pitarget(p, &p, list))return flag;
    int pi_name_len = p - pi_name;
    char* pi_value = p;
    pi_helper(p, &p, list);
    if(strncmp(p, "?>", 2) != 0)return flag;
    int pi_value_len = p - pi_value;
    p += 2;
    *next_pos = p;
    flag = 1;
    event_t pi_event={
        .type       = EVENT_PI,
        .offset     = head - glo.file_buf,
        .name       = pi_name,
        .name_len   = pi_name_len,
        .value      = pi_value,
        .value_len  = pi_value_len,
    };
    event_list_insert(list, &pi_event);
    return flag;
}
int pitarget(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    int flag = 0;
    if(!name(p, next_pos, list))return flag;
    else{
        flag = 1;
        if(*p != 'X' && *p != 'x')return flag;
        p++;
        if(*p != 'M' && *p != 'm')return flag;
        p++;
        if(*p != 'L' && *p != 'l')return flag;
        p++;
        if(p == *next_pos){//exactly "XML"
            flag = 0;
            *next_pos = head;
        }
    }
    return flag;
}
int cdsect(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(!cdstart(p, &p, list)){return flag;}
    char* value = p;
    if(!cdata(p, &p, list)){return flag;}
    int value_len = p - value;
    if(!cdend(p, &p, list)){return flag;}
    event_t cdata_event={
        .type       = EVENT_CDATA,
        .offset     = head - glo.file_buf,
        .name       = head,
        .name_len   = 0,
        .value      = value,
        .value_len  = value_len,
    };
    event_list_insert(list, &cdata_event);
    *next_pos = p;
    flag = 1;
    return flag;
}
int cdstart(char* p, char** next_pos, event_list_t* list){
    int flag = 0;
    if(strncmp(p, "<![CDATA[", 9) == 0){
        flag = 1;
        p += 9;
    }
    *next_pos = p;
    return flag;
}
int cdata(char* p, char** next_pos, event_list_t* list){
    int flag = 1;
    while(1){
        if(*p == ']' && strncmp(p, "]]>", 3) == 0)break;
        else if(charc(p, &p, list))continue;
        else break;
    }
    *next_pos = p;
    return flag;
}
int cdend(char* p, char** next_pos, event_list_t* list){
    int flag = 0;
    if(strncmp(p, "]]>", 3) == 0){
        flag = 1;
        p += 3;
    }
    *next_pos = p;
    return flag;
}
int namestartchar(char* p, char** next_pos, event_list_t* list){
    int flag = 0;
    if(*p == ':' || ('A' <= *p && *p <= 'Z') || *p == '_' || ('a' <= *p && *p <= 'z')){
        p++;
        flag = 1;
    }
    *next_pos = p;
    return flag;
}
int namechar(char* p, char** next_pos, event_list_t* list){
    int flag = 0;
    if(namestartchar(p, next_pos, list))return 1;
    if(*p == '-' || ('0' <= *p && *p <= '9') || *p == '.'){
        p++;
        flag = 1;
    }
    *next_pos = p;
    return flag;
}
int name(char* p, char** next_pos, event_list_t* list){
    if(!namestartchar(p, next_pos, list))return 0;
    while(namechar(p, &p, list));
    *next_pos = p;
    return 1;
}
int attvalue(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    char c;
    int flag = 0;
    if(*p != '\"' && *p != '\''){return flag;}
    else {c = *p; p++;}
    while(1){
        if(*p != '<' && *p != '&' && *p != c){p++;continue;}
        else if(reference(p, &p, list))continue;
        else break;
    }
    if(*p != c){return flag;}
    else p++;
    *next_pos = p;
    flag = 1;
    return flag;
}
int reference(char* p, char** next_pos, event_list_t* list){
    if(entityref(p, next_pos, list))return 1;
    else if(charref(p, next_pos, list))return 1;
    else return 0;
}
int entityref(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    int flag = 0;
    if(*p != '&'){return flag;}
    p++;
    if(!name(p, &p, list)){return flag;}
    if(*p != ';'){return flag;}
    p++;
    *next_pos = p;
    flag = 1;
    return flag;
}
int charref(char* p, char** next_pos, event_list_t* list){
    *next_pos = p;
    int flag = 0;
    if(strncmp(p, "&#x", 3) == 0){
        p += 3;
        while(('0' <= *p && *p <= '9')|| ('a' <= *p && *p <= 'f') || ('A' <= *p && *p <= 'F')){
            p++;
        }
        if(*p == ';'){flag = 1; p++;}
    }else if(strncmp(p, "&#", 2) == 0){
        p += 2;
        while('0' <= *p && *p <= '9'){
            p++;
        }
        if(*p == ';'){flag = 1; p++;}
    }
    if(flag)*next_pos = p;
    return flag;
}
int chardata(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    int flag = 1;
    while(*p != '<' && *p != '&' && *p != '\0'){
        if(*p == ']' && strncmp(p, "]]>", 3) == 0){
            break;
        }
        p++;
    }
    // event_t chardata_event={
    //     .type       = EVENT_CHAR_DATA,
    //     .offset     = head - glo.file_buf,
    //     .name       = head,
    //     .name_len   = 0,
    //     .value      = p,
    //     .value_len  = p-head,
    // };
    // if(p != head)event_list_insert(list, &chardata_event);
    *next_pos = p;
    return flag;
}
int eq(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    int flag = 0;
    space(p, &p, list);
    if(*p == '='){
        flag = 1;
        p++;
    }
    space(p, &p, list);
    if(flag)*next_pos = p;
    else *next_pos = head;
    return flag;
}
int space(char* p, char** next_pos, event_list_t* list){
    int flag = 0;
    while(*p == 0x20 || *p == 0x9 || *p == 0xD || *p == 0xA){
        flag = 1;
        p++;
    }
    *next_pos = p;
    return flag;
}
int charc(char* p, char** next_pos, event_list_t* list){
    char* head =0;
    int flag = 0;
    if(0x20 <= *p && *p <= 0x7F)flag = 1;
    if(*p == 0x9 || *p == 0xA || *p == 0xD)flag = 1;
    if(flag)p++;
    *next_pos = p;
    return flag;
}

////////////////////////////////////////////////////////////////



int start_tag_or_empty_tag(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(!stag(p, &p, list) && !emptyelemtag(p, &p, list))return flag;
    chardata(p, &p, list);
    *next_pos = p;
    flag = 1;
    return flag;
}

int end_tag(char* p, char** next_pos, event_list_t* list){
    char* head = p;
    *next_pos = p;
    int flag = 0;
    if(!etag(p, &p, list))return flag;
    chardata(p, &p, list);
    *next_pos = p;
    flag = 1;
    return flag;
}

int end_pi_or_comment_or_cdata(char* p, char** next_pos, event_list_t* list){
    p++;
    return chardata(p, next_pos, list);
}

int start_pi(char* p, char** next_pos, event_list_t* list){
    return pi(p, next_pos, list);
}

int start_comment(char* p, char** next_pos, event_list_t* list){
    return comment(p, next_pos, list);
}

int start_cdata(char* p, char** next_pos, event_list_t* list){
    return cdata(p, next_pos, list);
}

void loc_parse(event_list_t* elist, bcs_list_t* blist){
    bcs_node_t* ps = blist->head;
    bcs_node_t* pe = ps->next;
    while(pe != NULL){
        char* p = ps->p;
        char* p_next = pe->p;
        switch(ps->type){
            case BCS_START_TAG_BEGIN:
                start_tag_or_empty_tag(p, &p, elist);
                assert(p == p_next);
                break;
            case BCS_END_TAG_BEGIN:
                end_tag(p, &p, elist);
                assert(p == p_next);
                break;
            case BCS_PI_BEGIN:
                start_pi(p, &p, elist);
                //刚好错了一位
                //assert(p == p_next);
                break;
            case BCS_COMMENT_BEGIN:
                start_comment(p, &p, elist);
                //assert(p == p_next);
                break;
            case BCS_CDATA_BEGIN:
                start_cdata(p, &p, elist);
                //assert(p == p_next);
                break;
            case BCS_COMMENT_END:
            case BCS_PI_END:
            case BCS_CDATA_END:
            case BCS_TAG_END:
            case BCS_NONE:
                end_pi_or_comment_or_cdata(p, &p, elist);
                assert(p == p_next);
                break;
        }
        ps = pe;
        pe = pe->next;
    }
}

void* parse_workload(void* p_par_glo){
    static int id = 0;
    int tid = __sync_fetch_and_add(&id, 1);
    parse_glov_t* par_glo = p_par_glo;
    loc_parse(&(par_glo->elists[tid]), &(par_glo->blists[tid]));
    return (void*)0;
}

event_list_t* glo_parse(glov_t* glo, bcs_list_t* blists){
    parse_glov_t par_glo;
    parse_glov_init(&par_glo, glo, blists);
    int np = par_glo.np;
    pthread_t threads[np];
    for(int i=0; i<np; i++){
        pthread_create(&threads[i], NULL, parse_workload, &par_glo);
    }
    for(int i=0; i<np; i++){
        pthread_join(threads[i], NULL);
    }
    event_list_t* res = malloc(np * sizeof(event_list_t));
    memcpy(res, par_glo.elists, np * sizeof(event_list_t));
    parse_glov_destroy(&par_glo);
    return res;
}
#include "global.h"

void list_init(event_list_t* list){
    list->head = NULL;
    list->tail = NULL;
}
void list_destroy(event_list_t* list){
    event_node_t* p = list->head;
    while(p!=NULL){
        event_node_t* next = p->next;
        free(p);
        p = next;
    }
}
void list_insert(event_list_t* list, const event_t* event){
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
event_list_t list_merge(event_list_t* list1, event_list_t* list2){
    list1->tail->next = list2->head;
    return (event_list_t){
        .head = list1->head,
        .tail = list2->tail,
    };
}

event_list_t parse(bcs_t bcs){
    
}

int element(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(emptyelemtag(p, &p)){
    }else{
        if(!stag(p, &p))return flag;
        if(!content(p, &p))return flag;
        if(!etag(p, &p))return flag;
    }
    flag = 1;
    *next_pos = p;
    return flag;
}
int tag_helper(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(!space(p, &p))return flag;
    if(!attribute(p, &p))return flag;
    flag = 1;
    *next_pos = p;
    return flag;
}
int emptyelemtag(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(*p != '<')return flag;
    p++;
    if(!name(p, &p))return flag;
    while(tag_helper(p, &p));
    space(p, &p);
    if(strncmp(p, "/>", 2) != 0)return flag;
    p += 2;
    flag = 1;
    *next_pos = p;
    return flag;
}
int attribute(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(!name(p, &p))return flag;
    if(!eq(p, &p))return flag;
    if(!attvalue(p, &p))return flag;
    flag = 1;
    *next_pos = p;
    return flag;
}
int stag(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(*p != '<')return flag;
    p++;
    if(!name(p, &p))return flag;
    while(tag_helper(p, &p));
    space(p, &p);
    if(*p != '>')return flag;
    p++;
    flag = 1;
    *next_pos = p;
    return flag;
}
int etag(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(strncmp(p, "</", 2) != 0)return flag;
    p += 2;
    if(!name(p, &p))return flag;
    space(p, &p);
    if(*p != '>')return flag;
    p++;
    flag = 1;
    *next_pos = p;
    return flag;
}
int content_helper(char* p, char** next_pos){
    char* head = p;
    int flag = 0;
    if(element(p, next_pos)|| reference(p, next_pos) || cdsect(p, next_pos) || pi(p, next_pos) || comment(p, next_pos)){
        p = *next_pos;
        chardata(p, &p);
        *next_pos = p;
        flag = 1;
        return flag;
    }
    *next_pos = head;
    return flag;
}
int content(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    chardata(p, &p);
    while(content_helper(p, &p));
    *next_pos = p;
    flag = 1;
    return flag;
}
int comment_helper(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(*p == '-')p++;
    if(*p == '-')return flag;
    if(!charc(p, &p))return flag;
    *next_pos = p;
    flag = 1;
    return flag;
}
int comment(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(strncmp(p, "<!--", 4) != 0)return flag;
    p += 4;
    while(comment_helper(p, &p));
    if(strncmp(p, "-->", 3) != 0)return flag;
    p += 3;
    *next_pos = p;
    flag = 1;
    return flag;
}
int pi_helper(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(!space(p, &p))return flag;
    while(1){
        if(*p == '?' && strncmp(p, "?>", 2) == 0)break;
        else if(charc(p, &p))continue;
        else break;
    }
    *next_pos = p;
    flag = 1;
    return flag; 
}
int pi(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(strncmp(p, "<?", 2) != 0)return flag;
    p += 2;
    while(space(p, &p));
    if(!pitarget(p, &p))return flag;
    pi_helper(p, &p);
    if(strncmp(p, "?>", 2) != 0)return flag;
    p += 2;
    *next_pos = p;
    flag = 1;
    return flag;
}

int pitarget(char* p, char** next_pos){
    char* head = p;
    int flag = 0;
    if(!name(p, next_pos))return flag;
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
int cdsect(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(!cdstart(p, &p)){return flag;}
    if(!cdata(p, &p)){return flag;}
    if(!cdend(p, &p)){return flag;}
    *next_pos = p;
    flag = 1;
    return flag;
}
int cdstart(char* p, char** next_pos){
    int flag = 0;
    if(strncmp(p, "<![CDATA[", 9) == 0){
        flag = 1;
        p += 9;
    }
    *next_pos = p;
    return flag;
}
int cdata(char* p, char** next_pos){
    int flag = 1;
    while(1){
        if(*p == ']' && strncmp(p, "]]>", 3) == 0)break;
        else if(charc(p, &p))continue;
        else break;
    }
    *next_pos = p;
    return flag;
}
int cdend(char* p, char** next_pos){
    int flag = 0;
    if(strncmp(p, "]]>", 3) == 0){
        flag = 1;
        p += 3;
    }
    *next_pos = p;
    return flag;
}
int namestartchar(char* p, char** next_pos){
    int flag = 0;
    if(*p == ':' || ('A' <= *p && *p <= 'Z') || *p == '_' || ('a' <= *p && *p <= 'z')){
        p++;
        flag = 1;
    }
    *next_pos = p;
    return flag;
}
int namechar(char* p, char** next_pos){
    int flag = 0;
    if(namestartchar(p, next_pos))return 1;
    if(*p == '-' || ('0' <= *p && *p <= '9') || *p == '.'){
        p++;
        flag = 1;
    }
    *next_pos = p;
    return flag;
}
int name(char* p, char** next_pos){
    if(!namestartchar(p, next_pos))return 0;
    while(namechar(p, &p));
    *next_pos = p;
    return 1;
}
int attvalue(char* p, char** next_pos){
    *next_pos = p;
    char c;
    int flag = 0;
    if(*p != '\"' && *p != '\''){return flag;}
    else {c = *p; p++;}
    while(1){
        if(*p != '<' && *p != '&' && *p != c){p++;continue;}
        else if(reference(p,&p))continue;
        else break;
    }
    if(*p != c){return flag;}
    else p++;
    *next_pos = p;
    flag = 1;
    return flag;
}
int reference(char* p, char** next_pos){
    if(entityref(p, next_pos))return 1;
    else if(charref(p, next_pos))return 1;
    else return 0;
}
int entityref(char* p, char** next_pos){
    *next_pos = p;
    int flag = 0;
    if(*p != '&'){return flag;}
    p++;
    if(!name(p, &p)){return flag;}
    if(*p != ';'){return flag;}
    p++;
    *next_pos = p;
    flag = 1;
    return flag;
}
int charref(char* p, char** next_pos){
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
int chardata(char* p, char** next_pos){
    char* head = p;
    int flag = 1;
    while(*p != '<' && *p != '&' && *p != '\0'){
        if(*p == ']' && strncmp(p, "]]>", 3) == 0){
            break;
        }
        p++;
    }
    *next_pos = p;
    return flag;
}
int eq(char* p, char** next_pos){
    char* head = p;
    int flag = 0;
    space(p, &p);
    if(*p == '='){
        flag = 1;
        p++;
    }
    space(p, &p);
    if(flag)*next_pos = p;
    else *next_pos = head;
    return flag;
}
int space(char* p, char** next_pos){
    int flag = 0;
    while(*p == 0x20 || *p == 0x9 || *p == 0xD || *p == 0xA){
        flag = 1;
        p++;
    }
    *next_pos = p;
    return flag;
}
int charc(char* p, char** next_pos){
    int flag = 0;
    if(0x20 <= *p && *p <= 0x7F)flag = 1;
    if(*p == 0x9 || *p == 0xA || *p == 0xD)flag = 1;
    if(flag)p++;
    *next_pos = p;
    return flag;
}
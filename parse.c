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
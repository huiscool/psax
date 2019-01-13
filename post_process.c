#include "global.h"

void stack_init(event_stack_t* stack){
    stack->top = NULL;
    stack->n = 0;
}
void stack_destroy(event_stack_t* stack){
    event_stack_node_t* p = stack->top;
    while(p!=NULL){
        event_stack_node_t* tmp = p;
        p=p->prev;
        free(tmp);
    }
}
event_t* stack_top(event_stack_t* stack){
    return &(stack->top->event);
}
void stack_pop(event_stack_t* stack){
    (stack->n)--;
    event_stack_node_t* tmp = stack->top;
    event_stack_node_t* prev = tmp->prev;
    if(prev == NULL){
        stack->top = NULL;
        free(tmp);
    }else{
        stack->top = prev;
        prev->next = NULL;
        free(tmp);
    }
}
void stack_push(event_stack_t* stack, const event_t* event){
    (stack->n)++;
    event_stack_node_t* top = stack->top;
    event_stack_node_t* new_node = malloc(sizeof(event_stack_node_t));
    memcpy(&(new_node->event), event, sizeof(event_t));
    new_node->next = NULL;
    new_node->prev = NULL;
    if(top == NULL){
        stack->top = new_node;
    }else{
        top->next = new_node;
        new_node->prev = top;
        stack->top = new_node;
    }
}
bool stack_is_empty(event_stack_t* stack){
    return stack->n == 0;
}

event_list_t post_process(parse_glov_t* par_glo){
    int np = par_glo->np;
    event_list_t glo_events = par_glo->lists[0];
    for(int i=1; i<np; i++){
       glo_events = event_list_merge(&glo_events, &(par_glo->lists[i]));
    }
    event_node_t* p = glo_events.head;
    event_stack_t stack;
    stack_init(&stack);
    for( ; p != NULL; p=p->next){
        if( p->event.type == EVENT_ELEMENT_BEGIN){
            stack_push(&stack, &(p->event));
            continue;
        }
        if( p->event.type == EVENT_ELEMENT_END){
            if(stack_is_empty(&stack)){
                raise_error(SYNTAX_ERROR, p->event.offset, "tag not matched");
                break;
            }
            event_t* stag_event = stack_top(&stack);
            if(stag_event->name_len != p->event.name_len){
                raise_error(SYNTAX_ERROR, p->event.offset, "tag not matched");
            }else if(strncmp(stag_event->name, p->event.name, stag_event->name_len) != 0){
                raise_error(SYNTAX_ERROR, p->event.offset, "tag not matched"); 
            }else{
                stack_pop(&stack);
            }
        }
    }
    if(!stack_is_empty(&stack)){
        event_t* top = stack_top(&stack);
        raise_error(SYNTAX_ERROR, top->offset, "tag not matched");
    }
    stack_destroy(&stack);
    return glo_events;
}
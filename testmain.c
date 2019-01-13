#include "psax.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void* event_handler(const event_t* event){
#ifndef NO_OUTPUT
    static const char* event_names[] = {
        "EVENT_DOCUMENT_BEGIN",
        "EVENT_DOCUMENT_END",
        "EVENT_ELEMENT_BEGIN",
        "EVENT_EMPTY_ELEMENT",
        "EVENT_ELEMENT_END",
        "EVENT_ATTRIBUTE",
        "EVENT_CHAR_DATA",
        "EVENT_COMMENT",
        "EVENT_PI",
        "EVENT_CDATA", 
    };
    printf("event type: %s\n", event_names[ (int)event->type ]);
    printf("name:\n");
    for(int i=0; i<event->name_len; i++){
        printf("%c",event->name[i]);
    }
    printf("\nvalue:\n");
    for(int i=0; i<event->value_len; i++){
        printf("%c",event->value[i]);
    }
    printf("\n\n");
#endif //NO_OUTPUT
    return (void*)0;
}

void* error_handler(const error_t* error){
    static const char* error_names[] = {
        "THREAD_NUM_ERROR",
        "FILE_OPEN_ERROR",
        "LEXICAL_ERROR",
        "SYNTAX_ERROR"
    };
    printf("row:%lld col:%lld\n",error->row, error->col);
    printf("error_type: %s\n", error_names[ (int)error->type ]);
    printf("msg: %s\n",error->msg);
    exit(EXIT_FAILURE);
    return (void*)0;
}

int main(int argc, char const *argv[])
{
    if(argc != 3){
        printf("usage: ./testmain process_number file_path\n");
        return 0;
    }
    int process_num = atoi(argv[1]);
    const char* filename = argv[2];
    psax_parse(process_num, event_handler, error_handler, filename);
    return 0;
}


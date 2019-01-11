#include "psax.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void* event_handler(const event_t* event){
    static const char* event_names[] = {"DOCUMENT_BEGIN",
        "DOCUMENT_END",
        "ELEMENT_BEGIN",
        "ELEMENT_END",
        "CHAR_CONTENT",
        "CDATA",
        "PI",
        "COMMENT",
        "ATTRIBUTE"
    };
    printf("event type: %s\n", event_names[ (int)event->type ]);
    printf("name: %s\n", event->name);
    printf("value: %s\n", event->value);
    return (void*)0;
}

void* error_handler(const error_t* error){
    static const char* error_names[] = {
        "THREAD_NUM_ERROR",
        "FILE_OPEN_ERROR",
        "PARSE_ERROR"
    };
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


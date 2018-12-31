#include "psax.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
const char* filename = "test3.xml";

const int process_num = 1;

void* event_handler(event_t* event){
    static const char* event_names[] = {"DOCUMENT_BEGIN",
        "DOCUMENT_END",
        "ELEMENT_BEGIN",
        "ELEMENT_END",
        "CDATA",
        "PI",
        "COMMENT",
        "ATTRIBUTE"
    };
    printf("event type: %s\n", event_names[(int)event->type]);
    printf("name: %s\n", event->name);
    printf("value: %s\n", event->value);
    return (void*)0;
}

void* error_handler(error_t* error){
    printf("error_type: %d\n", (int)error->type);
    printf("row: %lld\n", error->row);
    printf("msg: %s\n",error->msg);
    exit(EXIT_FAILURE);
    return (void*)0;
}

int main(int argc, char const *argv[])
{
    psax_parse(process_num, event_handler, error_handler, filename);
    return 0;
}


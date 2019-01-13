#ifndef __PSAX_H__
#define __PSAX_H__
#include <stdlib.h>
typedef enum{
    EVENT_DOCUMENT_BEGIN  = 0,
    EVENT_DOCUMENT_END    = 1,
    EVENT_ELEMENT_BEGIN   = 2,
    EVENT_EMPTY_ELEMENT   = 3,
    EVENT_ELEMENT_END     = 4,
    EVENT_ATTRIBUTE       = 5,
    EVENT_CHAR_CONTENT    = 6,
    EVENT_COMMENT         = 7,
    EVENT_PI              = 8,
    EVENT_CDATA           = 9,
} event_type_t;

typedef enum{
    THREAD_NUM_ERROR    = 0,
    FILE_OPEN_ERROR     = 1,
    LEXICAL_ERROR       = 2,
    SYNTAX_ERROR        = 3,
} error_type_t;

#define MSG_BUF_SIZE 1024
#define SEND_RECV_BUF_SIZE 1024

typedef struct error{
    error_type_t type;
    int64_t row;
    int64_t col;
    char msg[MSG_BUF_SIZE];
} error_t;

typedef struct event{
    event_type_t type;
    int64_t offset;
    const char* name;
    int name_len;
    const char* value;
    int value_len;
} event_t;

typedef void* (*event_handler_t)(const event_t* event);

typedef void* (*error_handler_t)(const error_t* error);

//thread_num specifies the parsing threads num
int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename);

#endif //__PSAX_H__
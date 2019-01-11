#ifndef __PSAX_H__
#define __PSAX_H__

typedef enum{
    DOCUMENT_BEGIN  = 0,
    DOCUMENT_END    = 1,
    ELEMENT_BEGIN   = 2,
    ELEMENT_END     = 3,
    CHAR_CONTENT    = 4,
    CDATA           = 5,
    PI              = 6,
    COMMENT         = 7,
    ATTRIBUTE       = 8,
} event_type_t;

typedef enum{
    THREAD_NUM_ERROR    = 0,
    FILE_OPEN_ERROR     = 1,
    PARSE_ERROR         = 2,
} error_type_t;

#define MSG_BUF_SIZE 1024

typedef struct error{
    error_type_t type;
    int64_t row;
    int64_t col;
    char msg[MSG_BUF_SIZE];
} error_t;

typedef struct{
    event_type_t type;
    const char* name;
    const char* value;
} event_t;

typedef void* (*event_handler_t)(const event_t* event);

typedef void* (*error_handler_t)(const error_t* error);

//thread_num specifies the parsing threads num
int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename);

#endif //__PSAX_H__
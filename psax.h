#ifndef __PSAX_H__
#define __PSAX_H__

typedef enum{
    DOCUMENT_BEGIN,
    DOCUMENT_END,
    ELEMENT_BEGIN,
    ELEMENT_END,
    CDATA,
    PI,
    COMMENT,
    ATTRIBUTE
} event_type_t;

typedef enum{
    E_THREAD_NUM_ERROR,
    E_FILE_OPEN_FAILED,
    E_PARSE_ERROR
} error_type_t;

typedef struct error{
    error_type_t type;
    unsigned long long row;
    const char* msg;
} error_t;

typedef struct{
    event_type_t type;
    const char* name;
    const char* value;
} event_t;

typedef void* (*event_handler_t)(event_t* event);

typedef void* (*error_handler_t)(error_t* error);

int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename);
#endif //__PSAX_H__
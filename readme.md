#PSAX

利用posix pthread库的事件驱动式的并行XML解析器



## SAX(Simple API for XML)主要特性

* event-based API for reading XML documents  (提供解析XML所产生的事件API)

* reads an XML document only once(只解析XML一次)

* each time the parser sees a start-tag, an end-tag, character data, or a processing instruction, it tells your program (每当解析器看到开始标签, 结束标签,字符数据,处理指令,解析器会通过回调函数通知你的程序)



## PSAX特点

* 通过pthread库加快解析XML速度





## API

```c
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
    THREAD_NUM_ERROR,
    FILE_OPEN_ERROR,
    PARSE_ERROR
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

int psax_parse(int thread_num, event_handler_t event_handler, error_handler_t error_handler, const char* filename);

#endif //__PSAX_H__
```



## 实现原理

1. 预处理阶段:







### 严重错误(fatal error)检测

根据[XML1.0 规范](https://www.w3.org/TR/REC-xml/) , 合格的XML解析器应该能够识别严重错误,并将其发送给应用程序(Application). 



## 参考文献

[1]方跃坚,余枝强,翟磊,吴中海.一种混合并行XML解析方法[J].软件学报,2013,24(06):1196-1206.


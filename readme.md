#PSAX

利用posix pthread库的事件驱动式的并行XML解析器。



### SAX(Simple API for XML)主要特性：

* event-based API for reading XML documents  (提供解析XML所产生的事件API)

* reads an XML document only once（只解析XML一次）

* each time the parser sees a start-tag, an end-tag, character data, or a processing instruction, it tells your program （每当解析器看到开始标签，结束标签，字符数据，处理指令，解析器会告诉你的程序）



### PSAX特点：

* 通过pthread库并行加快解析XML速度



## API

```c
#include <psax.h>

typedef enum{
    DOCUMENT_BEGIN,
    
} event_type;

typedef struct{
    event_type type;
    
} event_t;

typedef void (*handler_t)(event_t* event);

int psax_start_parse(handler_t handler, const char* filename);
```





***

### 参考文献：

[1]方跃坚,余枝强,翟磊,吴中海.一种混合并行XML解析方法[J].软件学报,2013,24(06):1196-1206.


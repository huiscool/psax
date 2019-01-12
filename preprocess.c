#include "global.h"
bcs_t produce_bcs_iterator(char* p, int64_t row, int64_t col, char** next_p , int64_t* next_row, int64_t* next_col){
    static const unsigned char special_delimiter_counts = 3;
    static bcs_type_t delimiter_type[] = {
        BCS_COMMENT,
        BCS_PI,
        BCS_CDATA,
    };
    static char* begin_delimiters[] = {
        "<!--",
        "<?",
        "<![CDATA[",
    };
    static unsigned char begin_delimiter_len[] = {
        4, 2, 9,
    };
    static char* end_delimiters[] = {
        "-->",
        "?>",
        "]]>",
    };
    static unsigned char end_delimiter_len[] = {
        3, 2, 3,
    };
    while(*p != '<' && *p != '\0'){
        p++;// look for the next '<'
        col++;
        if(*p == '\n'){
            col = -1;
            row++;
        }
    }
    if(*p == '\0'){// when p reach the end of file
        return (bcs_t){
            .type   = BCS_DONE,
            .row    = row,
            .col    = col,
            .head   = "",
            .size   = 0,
        };
    }
    char* head = p;
    int64_t head_row = row;
    int64_t head_col = col;
    for(int i=0; i<special_delimiter_counts; i++){//preprocess the special bcs
        if(strncmp(head, begin_delimiters[i], begin_delimiter_len[i]) == 0){
            int find_tail = 0;
            while(!find_tail){
                while(*p != '>' && *p != '\0'){// look for the next '>'
                    p++;
                    col++;
                    if(*p == '\n'){
                        col = -1;
                        row++;
                    }
                }
                if(*p == '\0'){// cannot find the tail
                    raise_error(SYNTAX_ERROR, head_row, head_col, "cannot find matched symbol");
                }
                char* end_delimiter_head = p - (end_delimiter_len[i]-1);
                if(strncmp(end_delimiter_head, end_delimiters[i], end_delimiter_len[i]) == 0){
                    find_tail = 1;
                }else{
                    p++;//when cannot find the tail, go on;
                    col++;
                }
            }
            while(*p != '<' && *p != '\0'){
                p++;// look for the next '<'
                col++;
                if(*p == '\n'){
                    col = -1;
                    row++;
                }
            }
            *next_p = p;
            *next_row = row;
            *next_col = col;
            return (bcs_t){
                .type   = delimiter_type[i],
                .row    = head_row,
                .col    = head_col,
                .head   = head,
                .size   = p-head,
            };
        }
    }
    p++;
    while(*p != '<' && *p != '\0'){
        p++;// look for the next '<'
        col++;
        if(*p == '\n'){
            col = -1;
            row++;
        }
    }
    *next_p = p;
    *next_row = row;
    *next_col = col;
    return (bcs_t){
        .type   = head[1]=='/' ? BCS_END_TAG : BCS_START_TAG ,
        .row    = head_row,
        .col    = head_col,
        .head   = head,
        .size   = p - head,
    };
}
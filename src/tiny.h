#ifndef _TINY_H
#define _TINY_H

typedef struct url_list {
    char* url;
    struct url_list* next;
} url_list;

#ifdef __cplusplus
extern "C" void build_index(char*);
extern "C" url_list* search_index(char*);
extern "C" void delete_list(url_list*);
extern "C" int list_length(url_list*);

#else
void build_index(char*);
url_list* search_index(char*);
void delete_list(url_list*);
int list_length(url_list*);
#endif

#endif

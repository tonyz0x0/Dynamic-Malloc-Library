#include <stdio.h>
#include "common.h"

void *reallocarray(void *ptr, size_t nmemb, size_t size) {
    //Case 1: if ptr is NULL, reallocarray is the same as malloc
    //Case 2: if size or nmemb is 0, reallocarray will free the memory
    //Case 3: else it will be the same as realloc(ptr, nmemb * size)
    if(ptr == NULL) {
        return malloc(nmemb * size);
    } else {
        if(size == 0 || nmemb == 0){
            free_Memory(ptr);
            return NULL;
        } else {
           return realloc(ptr, nmemb * size);
        }
    }
}


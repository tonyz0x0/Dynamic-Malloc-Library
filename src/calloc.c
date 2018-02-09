#include <stdio.h>
#include "common.h"


void *calloc(size_t nmemb, size_t size) {
    //debug_print("Start calloc!\n");
    if(nmemb==0||size==0) {
        //debug_print("calloc 0!\n");
        return NULL;
    }
    void *res;
    size_t totalSize = nmemb * size;
   // size_t totalSize= align8(nmemb * size);
    if((res = allocateMemory(totalSize)) == NULL) {
        //debug_print("calloc Failed!\n");
        return NULL;
    }
    memset(res, 0, totalSize);
    //debug_print("calloc sucess!\n");
    return res;
}
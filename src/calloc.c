#include <stdio.h>
#include "common.h"


void *calloc(size_t nmemb, size_t size) {
    debug_print("Start calloc!\n");
    if(nmemb==0||size==0) {
        debug_print("calloc 0!\n");
        return NULL;
    }
    void *res;
    size_t totalSize= align8(nmemb * size);
    if((res = malloc(totalSize)) == NULL) {
        debug_print("calloc Failed!\n");
        return NULL;
    }
    pthread_mutex_lock(&lock);
    memset(res, 0, totalSize);
    debug_print("calloc sucess!\n");
    pthread_mutex_unlock(&lock);
    return res;
}
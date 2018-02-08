#include <stdio.h>
#include "common.h"


void *calloc(size_t nmemb, size_t size) {
    debug_print("Start calloc!\n");
    if(nmemb==0||size==0) {
        debug_print("calloc sucess!\n");
        return NULL;
    }
    void *res;
    if((res = malloc(nmemb * size)) == NULL) {
        debug_print("calloc Failed!\n");
        return NULL;
    }
    pthread_mutex_lock(&lock);
    memset(res, 0, size * nmemb);
    //debug_print("calloc sucess!\n");
    pthread_mutex_unlock(&lock);
    return res;
}
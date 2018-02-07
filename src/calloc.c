#include "common.h"


void *calloc(size_t nmemb, size_t size) {
    size = nmemb * size;
    void *res;
    if((res = malloc(size)) == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&lock);
    memset(res, 0, size);
    pthread_mutex_unlock(&lock);
    return res;
}
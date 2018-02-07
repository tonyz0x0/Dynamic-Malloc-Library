#include"common.h"


void *malloc(size_t size) {
    if(size <= 0) {
        return NULL;
    }
    void *res;
    pthread_mutex_lock(&lock);
    res = allocateMemory(size);
    pthread_mutex_unlock(&lock);
    return res;
}
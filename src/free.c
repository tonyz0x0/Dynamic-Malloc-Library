#include "common.h"

void free(void *ptr) {
    if(ptr == NULL) {
        return;
    }
    pthread_mutex_lock(&lock);
    free_Memory(ptr);
    pthread_mutex_unlock(&lock);
    return;
}
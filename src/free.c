#include <stdio.h>
#include "common.h"

void free(void *ptr) {
    //debug_print("Start free Entry!\n");

    if(ptr == NULL) {
        return;
    }
    pthread_mutex_lock(&lock);
    free_Memory(ptr);
    pthread_mutex_unlock(&lock);
    return;
}
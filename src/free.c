#include <stdio.h>
#include "common.h"

void free(void *ptr) {
    if(ptr == NULL) {
        return;
    }
    free_Memory(ptr);
    return;
}
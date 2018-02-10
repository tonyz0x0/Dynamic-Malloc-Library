#include <stdio.h>
#include "common.h"

void free(void *ptr) {
    //debug_print("Start free Entry!\n");

    if(ptr == NULL) {
        return;
    }
    free_Memory(ptr);
    return;
}
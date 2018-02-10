#include "common.h"
//
int posix_memalign(void **memptr, size_t alignment, size_t size) {
    void *mem;
    mem = memalign(alignment, size);
    if(mem != NULL) {
        *memptr = mem;
        return 0;
    }
    return ENOMEM;
}


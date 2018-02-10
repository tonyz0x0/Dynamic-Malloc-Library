#include <stdio.h>
#include "common.h"

#ifndef align_up
#define align_up(num, align) \
	(((num) + ((align) - 1)) & ~((align) - 1))
#endif

void *memalign(size_t alignment, size_t size) {

    void *ptr = NULL;

    if(alignment && size)
    {
        size_t offset = alignment - 1;
        void *mem1 = malloc(size + offset);
        void *mem2 = malloc(size);
        if(mem1 && mem2) {
            if((uintptr_t)mem2 % alignment != 0) {
                ptr = (void*)((uintptr_t)mem1 + alignment - ((uintptr_t)mem1 % alignment));
                memcpy(mem2, ptr, size);
                free_Memory(mem1);
                return mem2;
            }
            return mem2;
        }
    }
    return ptr;
}

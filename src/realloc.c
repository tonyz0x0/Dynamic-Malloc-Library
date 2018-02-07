#include "common.h"


void *realloc(void *ptr, size_t size) {
    //Case 1: if ptr is NULL, realloc is the same as malloc
    //Case 2: if size is 0, realloc will free the memory
    //Case 3: if size is not 0, it will change memory size
    if(ptr == NULL) {
        return malloc(size);
    } else {
        if(size == 0){
            free_Memory(ptr);
            return NULL;
        } else {
            void *newMem = allocateMemory(size);
            BlockHeader *bh = (BlockHeader*)(ptr - sizeof(BlockHeader));
            size_t size = 1 << (bh->level + MIN_ORDER);
            memcpy(newMem, ptr, size);
            free_Memory(ptr);
            return newMem;
        }
    }
}

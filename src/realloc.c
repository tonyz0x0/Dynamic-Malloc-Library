#include <stdio.h>
#include "common.h"


void *realloc(void *ptr, size_t size) {
    //Case 1: if ptr is NULL, realloc is the same as malloc
    //Case 2: if size is 0, realloc will free the memory
    //Case 3: if size is not 0, it will change memory size
    if (ptr == NULL) {
        return allocateMemory(size);
    }
    if (size == 0) {
        free_Memory(ptr);
        return NULL;
    }
    size += sizeof(BlockHeader);
    BlockHeader *tmp = (BlockHeader *) ((char *) ptr - sizeof(BlockHeader));
    int oldlevel = tmp->level;
    int oldSize = 1 << (oldlevel + MIN_ORDER);
    int newlevel = get_level(size);
    if (oldlevel >= newlevel) {
        return ptr;
    }
    void *newmem;
    size -= sizeof(BlockHeader);
    newmem = malloc(size);
    if (!newmem) {
        return NULL;
    }
    memcpy(newmem, ptr, oldSize - sizeof(BlockHeader));
    free(ptr);
    return newmem;
}
#include "common.h"

void *reallocarray(void *ptr, size_t nmemb, size_t size) {
    //Case 1: if ptr is NULL, reallocarray is the same as malloc
    //Case 2: if size or nmemb is 0, reallocarray will free the memory
    //Case 3: if size multiple nmemb overflows, it will return NULL
    //Case 4: else it will be the same as realloc(ptr, nmemb * size)
    size_t result;
    if(ptr == NULL) {
        return malloc(nmemb * size);
    } else {
        if(size == 0 || nmemb == 0){
            free_Memory(ptr);
            return NULL;
        } else if(mulOvf(&result, nmemb, size) == 0) {
            return NULL;
        } else {
            void *newMem = allocateMemory(size * nmemb);
            BlockHeader *bh = (BlockHeader*)(ptr - sizeof(BlockHeader));
            size_t oldSize = 1 << (bh->level + MIN_ORDER);
            memcpy(newMem, ptr, oldSize - sizeof(BlockHeader));
            free_Memory(ptr);
            return newMem;
        }
    }
}


#include <stdio.h>
#include "common.h"


void *realloc(void *ptr, size_t size) {
    //Case 1: if ptr is NULL, realloc is the same as malloc
    //Case 2: if size is 0, realloc will free the memory
    //Case 3: if size is not 0, it will change memory size
    debug_print("Start realloc!\n");
    if(ptr == NULL) {
        return malloc(size);
    } else {
        if(size == 0){
            free_Memory(ptr);
            return NULL;
        } else {
            void* newMem;
            size += sizeof(BlockHeader);
            BlockHeader *tmp = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
            int oldlevel = tmp->level;
            int newlevel = get_level(size);
            if(size > (MAX_BLOCK_SIZE / 2)) {
                newMem = request_memory_by_mmap(find_page_size(size));
                free_Memory(ptr);
                debug_print("realloc into mmap sucess!\n");
                return newMem;
            } else if(size <= MAX_BLOCK_SIZE && newlevel == oldlevel ) {
                return ptr;
            } else {
                newMem = malloc(size);
                memcpy(newMem, ptr, size - sizeof(BlockHeader));
                free_Memory(ptr);
                debug_print("realloc sucess!\n");
                return newMem;
            }
            return NULL;
        }
    }
}

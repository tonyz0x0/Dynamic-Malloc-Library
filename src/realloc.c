#include <stdio.h>
#include "common.h"


void *realloc(void *ptr, size_t size) {
    //Case 1: if ptr is NULL, realloc is the same as malloc
    //Case 2: if size is 0, realloc will free the memory
    //Case 3: if size is not 0, it will change memory size
    //debug_print("Start realloc!\n");
    if (ptr == NULL) {
        //debug_print("change realloc to malloc!\n");
        return allocateMemory(size);
    }
    if (size == 0) {
        free_Memory(ptr);
        //debug_print("chang realloc to free!\n");
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

//    size = align8(size);
//    void *newMem;
//    size += sizeof(BlockHeader);
//    BlockHeader *tmp = (BlockHeader *) ((char *) ptr - sizeof(BlockHeader));
//    int oldlevel = tmp->level;
//    int oldSize = 1 << (oldlevel + MIN_ORDER);
//    int newlevel = get_level(size);
//    //Case 1
//    if (size > (MAX_BLOCK_SIZE / 2)) {
//        newMem = request_memory_by_mmap(find_page_size(size));
//        free_Memory(ptr);
//        debug_print("realloc into mmap sucess!\n");
//        return newMem;
//    }
//    //Case 2:
//    if (newlevel == oldlevel) {
//        debug_print("realloc remain the same!\n");
//        return ptr;
//    }
//    //Case 3:
//    if(oldlevel < newlevel){
//        size -= sizeof(BlockHeader);
//        newMem = malloc(size);
//        memcpy(newMem, ptr, oldSize);
//        free_Memory(ptr);
//        debug_print("realloc sucess!\n");
//        return newMem;
//    }
//    if(oldlevel > newlevel) {
//        size -= sizeof(BlockHeader);
//        newMem = malloc(size);
//        memcpy(newMem, ptr, size);
//        free_Memory(ptr);
//        debug_print("realloc sucess!\n");
//        return newMem;
//    }
//    return NULL;
//}

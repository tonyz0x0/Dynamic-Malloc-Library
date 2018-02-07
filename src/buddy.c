#include <stdio.h>
#include "common.h"
#include <string.h>

//request memory in heap
void *request_memory_from_heap(size_t size){
    void *ret;
    if((ret = sbrk(size)) < (void *)0) {
        errno = ENOMEM;
        return NULL;
    }
    return ret;
}

void *allocateMemory(size_t size){
    //the actual size that needs to require
    size += sizeof(BlockHeader);
    int pageSize = find_page_size(size);
    //initialize the memory in the heap
    if(heap == NULL) {
        init_memory(MAX_BLOCK_SIZE);
    }
    //Check block in the free list
    //if the required memory is bigger than MAX_BLOCK_SIZE, we change to use memory map
    //to allocate the memory
    if(pageSize > MAX_BLOCK_SIZE) {
        return request_memory_by_mmap(pageSize);
    }
    //**check if there is available space in freeList***//
    int level = get_level(size);
//    char buf[1024];
//    snprintf(buf, 1024, "level: %d\n",
//             level);
//    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    //Step1: prepare for the available block for required memory if necessary
    int split_level = get_available_block(freeList, level);
    //split the memory into proper buddy blocks if necessary
    split_buddy(freeList, level, split_level);

//    int i = 0;
//        char buf[4096];
//    for(;i<=7;i++) {
//        snprintf(buf, 4096, "freelist[%d]: 1.startAddress: %p; 2.next: %p; 3.previous: %p; 4. level: %d; 5.status: %d\n",
//                 i, freeList[i], freeList[i]->next, freeList[i]->previous, freeList[i]->level, freeList[i]->status);
//        write(STDOUT_FILENO, buf, strlen(buf) + 1);
//    }

    //Step2: allocate block to the required memory
    if(freeList[level] != NULL) {
        BlockHeader* allocateblock = freeList[level];
        allocateblock->status = 1; //mark it as used
        freeList[level] = allocateblock->next;
        if(freeList[level] != NULL) {
            freeList[level]->previous = allocateblock->previous;
        }
        allocateblock->next = NULL;
        allocateblock->previous = NULL;
        allocateblock->level = level;
        return (void*)((char*) allocateblock + sizeof(BlockHeader));
    }
    return NULL;
}

//Initialize the memory in the heap
void init_memory(size_t size) {
    heap = (BlockHeader*)request_memory_from_heap(size);
    freeList[MAX_LEVEL] = heap;
    freeList[MAX_LEVEL]->status = 0;
	freeList[MAX_LEVEL]->level = MAX_LEVEL;
    freeList[MAX_LEVEL]->next = NULL;
    freeList[MAX_LEVEL]->previous = NULL;
}

//get available Block
int get_available_block(BlockHeader **freeList, int level){
    //if there is no available bock in freeList[level], we get to the upper freeList to Find
    //bigger block
    if(freeList[level] == NULL) {
        int i = level + 1;
        while((i <= MAX_LEVEL) && (freeList[i] == NULL)) {
            i++;
        }
        //if there is no available space in the biggest level of freeList, we need to require
        //another space in the heap
        //printf("i:%d\n", i);
        if(i > MAX_LEVEL) {
            heap = (BlockHeader*)request_memory_from_heap(MAX_BLOCK_SIZE);
            //if(freeList[MAX_LEVEL] == NULL) {
                freeList[MAX_LEVEL] = heap;
                freeList[MAX_LEVEL]->status = 0;
                freeList[MAX_LEVEL]->level = MAX_LEVEL;
                freeList[MAX_LEVEL]->next = NULL;
                freeList[MAX_LEVEL]->previous = NULL;
             return MAX_LEVEL;
//            } else {
//                BlockHeader *bh = freeList[MAX_LEVEL];
//                while(bh->next != NULL) {
//                    bh = bh->next;
//                }
//                BlockHeader *newBlock = heap;
//                newBlock->status = 0;
//                newBlock->level = MAX_LEVEL;
//                newBlock->next = NULL;
//                newBlock->previous = bh;
//                bh->next = newBlock;
//                bh->level = MAX_LEVEL;
//            }
        }
        return i;
    }
    return -1;
}

//split the memory
void split_buddy(BlockHeader **freeList, int level, int split_level) {
    //if freeList[level] has no available space, we start to split the memory from the freeList[MAX_LEVEL]
    // to freeList[level] until there is available space in freeList[level]
    int new_level;
     while ((freeList[level] == NULL) && (split_level > level)) {
        //there is enough space in this split_level, we start to split
        new_level = split_level - 1;
        if(freeList[new_level] == NULL) {
            // store the old freeList[split_level] address
            void *old = (void*)freeList[split_level];
            freeList[split_level] = freeList[split_level]->next;
            //crete two new buddy blocks
            BlockHeader *left_buddy = (BlockHeader*)old;
            left_buddy->status = 0;
            left_buddy->level = new_level;
            left_buddy->next = NULL;
            left_buddy->previous = NULL;
            int helper = 1 << (new_level + MIN_ORDER);
            BlockHeader *right_buddy = (BlockHeader*)((char*)old + helper);
            right_buddy->status = 0;
            right_buddy->level = new_level;
            right_buddy->next = NULL;
            right_buddy->previous = NULL;
            //connect two buddy blocks
            left_buddy->next = right_buddy;
            right_buddy->previous = left_buddy;
            freeList[new_level] = left_buddy;
        }
         split_level--;
      }
}

//Find the proper size of page
size_t find_page_size(size_t size) {
  size_t pageSize = PAGESIZE;
  size_t dataSize = pageSize - sizeof(BlockHeader);
  while (dataSize < size) {
      pageSize *= 2;
      dataSize = pageSize - sizeof(BlockHeader);
  }
  return pageSize;
}

//get the level of the BlockHeader in freeList array
int get_level(size_t size) {
    int level = 0;
    while(size >= MIN_BLOCK_SIZE) {
        size /= 2;
        level++;
    }
    return level;
}

void *request_memory_by_mmap(size_t pageSize) {
    void *map;
    if((map = mmap(0, pageSize, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
        exit(-1);
    }
    BlockHeader *bh = (BlockHeader*)map;
    bh->status = 0;
	bh->level = get_level(pageSize);
    bh->next = NULL;
    bh->previous = NULL;
    return (void *)((char*)bh + sizeof(BlockHeader));
}

//
void free_Memory(void* ptr) {
    BlockHeader *releasedBlock = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    //if the memory is allocated by mmap, use munmap to free that memory
    size_t size = 1 << (releasedBlock->level + MIN_ORDER - 1);
    if(size > MAX_BLOCK_SIZE){
        munmap((void*)(char*)ptr - sizeof(BlockHeader), size);
        return;
    }
    releasedBlock->status = 0;//set to not used
    BlockHeader *buddyBlock = find_buddy(releasedBlock);
    //now begin to determine whether it will merge its buddy into a bigger block or not
    while (buddyBlock != NULL && buddyBlock->status != 1 && buddyBlock->level == releasedBlock->level) {
        if(buddyBlock->next != NULL && buddyBlock->previous == NULL) {
            buddyBlock->next->previous = NULL;
            freeList[buddyBlock->level] = buddyBlock->next;
        }
        if(buddyBlock->previous != NULL && buddyBlock->next == NULL){
            buddyBlock->previous->next = NULL;
            freeList[buddyBlock->level] = buddyBlock->previous;
        }
        if(buddyBlock->next == NULL && buddyBlock->previous == NULL) {
            freeList[buddyBlock->level] = NULL;
        }
        if(buddyBlock->next != NULL && buddyBlock->previous != NULL) {
            buddyBlock->next->previous = buddyBlock->previous;
            buddyBlock->previous->next = buddyBlock->next;
            freeList[buddyBlock->level] = buddyBlock->previous;
        }
        if(releasedBlock > buddyBlock) {
            releasedBlock = buddyBlock;
        }
        releasedBlock->level += 1;
        buddyBlock = find_buddy(releasedBlock);
    }
    if(freeList[releasedBlock->level] != NULL) {
        BlockHeader* temp = freeList[releasedBlock->level];
        temp->previous = releasedBlock;
        releasedBlock->next = temp;
    }
    freeList[releasedBlock->level] = releasedBlock;
}

BlockHeader *find_buddy(BlockHeader* releasedBlock){
    if(releasedBlock->level == MAX_LEVEL) {
        return NULL;
    } else {
        int helper = 1 << (releasedBlock->level + MIN_ORDER);
        BlockHeader *buddyBlock = (BlockHeader *) ((unsigned long int) releasedBlock ^ helper);
        return buddyBlock;
    }
}

//determine whether it will cause overflow or not
int mulOvf(size_t *result, size_t a, size_t b) {
    *result = a * b;
    if(*result / a != b) {
        return 0;
    }
    return 1;
}
//void *memalign(size_t alignment, size_t size) {
//    int offset = (int)log2(alignment);
//    return (void *)((mymalloc(size) >> offset) << offset);
//}
//
//int posix_memalign(void **memptr, size_t alignment, size_t size) {
//    void *mem;
//    mem = memalign(alignment, size);
//    if(mem != NULL) {
//        *memptr = mem;
//        return 0;
//    }
//    return ENOMEM;
//}

#include <stdio.h>
#include "common.h"

__thread BlockHeader *freeList[MAX_LEVEL + 1]; //create a freeList array to implement buddy allocation algorithm
pthread_mutex_t lock;

//request memory in heap
void *request_memory_from_heap(size_t size){
    void *ret = sbrk(0);
    if((sbrk(size)) == (void *)-1) {
        errno = ENOMEM;
        return NULL;
    }
    return ret;
}

void *allocateMemory(size_t size){
    if(size <= 0) {
        return NULL;
    }
    pthread_mutex_lock(&lock);
    //the actual size that needs to require
    size += sizeof(BlockHeader);
    //Check block in the free list
    //if the required memory is bigger than 2048 bytes, we change to use memory map
    //to allocate the memory
    size_t pageSize = find_page_size(size);
    if(pageSize > (MAX_BLOCK_SIZE / 2)) {
        return request_memory_by_mmap(pageSize);
    }

    //**check if there is available space in freeList***//
    int level = get_level(size);

    //Step1: prepare for the available block for required memory if necessary

    //if there is no available bock in freeList[level], we get to the upper freeList to Find
    //bigger block
    int split_level = level;
    if(freeList[level] == NULL) {
        int i = level + 1;
        while ((i <= MAX_LEVEL) && (freeList[i] == NULL)) {
            i++;
        }
        //if there is no available space in the biggest level of freeList, we need to require
        //another space in the heap

        if (i > MAX_LEVEL) {

            BlockHeader* newHeap = (BlockHeader *) request_memory_from_heap(MAX_BLOCK_SIZE);

            newHeap->status = 0;
            newHeap->level = MAX_LEVEL;
            newHeap->next = NULL;
            newHeap->previous = NULL;
            split_level = MAX_LEVEL;
            freeList[MAX_LEVEL] = newHeap;

        } else {
            split_level = i;

        }
    }

    //split the memory into proper buddy blocks if necessary

    if(freeList[level] == NULL) {
        split_buddy(freeList, level, split_level);
    }



    //Step2: allocate block to the required memory
    if(freeList[level] != NULL) {
        BlockHeader* allocateblock = freeList[level];
        allocateblock->status = 1; //mark it as used
        freeList[level] = allocateblock->next;
        if(freeList[level] != NULL) {
            freeList[level]->previous = NULL;
        }
        allocateblock->next = NULL;
        allocateblock->previous = NULL;
        allocateblock->level = level;
        pthread_mutex_unlock(&lock);
        return (void*)((char*) allocateblock + sizeof(BlockHeader));
    }
    pthread_mutex_unlock(&lock);
    return NULL;
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
            BlockHeader *old = freeList[split_level];
            freeList[split_level] = freeList[split_level]->next;
            if(freeList[split_level]) {
                freeList[split_level]->previous = NULL;
            }
            //crete two new buddy blocks
            BlockHeader *left_buddy = old;
            left_buddy->status = 0;
            left_buddy->level = new_level;
            left_buddy->next = NULL;
            left_buddy->previous = NULL;
            int helper = 1 << (new_level + MIN_ORDER);
            BlockHeader *right_buddy = (BlockHeader*)((void*)old + helper);
            right_buddy->status = 0;
            right_buddy->level = new_level;
            right_buddy->next = NULL;
            right_buddy->previous = NULL;
            //connect two buddy blocks
            left_buddy->next = right_buddy;
            right_buddy->previous = left_buddy;
            freeList[new_level] = left_buddy;
            //debug_print("Split success! Left Address is: %p, right Address is: %p\n", left_buddy, right_buddy);
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
    int mem = 1 << MIN_ORDER;
    while(size > mem) {
        mem *= 2;
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
    bh->status = 1;
    bh->level = get_level(pageSize);
    bh->next = NULL;
    bh->previous = NULL;
    pthread_mutex_unlock(&lock);
    return (void *)((char*)bh + sizeof(BlockHeader));
}

//
void free_Memory(void* ptr) {
    pthread_mutex_lock(&lock);
    BlockHeader *releasedBlock = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    //if the memory is allocated by mmap, use munmap to free that memory
    size_t size = 1 << (releasedBlock->level + MIN_ORDER);
    if(size > (MAX_BLOCK_SIZE / 2)){
        munmap((void*)(char*)ptr - sizeof(BlockHeader), size);
        pthread_mutex_unlock(&lock);
        return;
    }

    //remove the releasedblock outside the freelist
    releasedBlock->status = 0;//set to not used

    //now begin to determine whether it will merge its buddy into a bigger block or not
    while(releasedBlock->level <= MAX_LEVEL) {
        //link the heap
        if(releasedBlock->level == MAX_LEVEL) {
            releasedBlock->next = freeList[MAX_LEVEL];
            if(freeList[MAX_LEVEL]) {
                freeList[MAX_LEVEL]->previous = releasedBlock;
            }
            releasedBlock->previous = NULL;
            freeList[MAX_LEVEL] = releasedBlock;
            pthread_mutex_unlock(&lock);
            break;
        }
        BlockHeader *buddyBlock = find_buddy(releasedBlock);

        if(buddyBlock != NULL && buddyBlock->status != 1 && buddyBlock->level == releasedBlock->level) {
            //remove the buddy out of the freelist
            if(buddyBlock->previous) {
                buddyBlock->previous->next = buddyBlock->next;
            }
            if(buddyBlock->next) {
                buddyBlock->next->previous = buddyBlock->previous;
            }
            buddyBlock->next = NULL;
            buddyBlock->previous = NULL;
            buddyBlock->status = 0;
            //judge which address is bigger
            if((void*)releasedBlock > (void*)buddyBlock) {
                releasedBlock = buddyBlock;
            }
            releasedBlock->level += 1;
            continue;
        } else {
            //no buddy can be merged further, then add the merged block into its freelist
            releasedBlock->next = freeList[releasedBlock->level];
            if(freeList[releasedBlock->level]) {
                freeList[releasedBlock->level]->previous = releasedBlock;
            }
            releasedBlock->previous = NULL;
            freeList[releasedBlock->level] = releasedBlock;
            pthread_mutex_unlock(&lock);
            break;
        }
    }
}

BlockHeader *find_buddy(BlockHeader* releasedBlock){
    if(releasedBlock->level == MAX_LEVEL) {
        return NULL;
    } else {
        int helper = 1 << (releasedBlock->level + MIN_ORDER);
        BlockHeader *buddyBlock = (BlockHeader *) ((uintptr_t) releasedBlock ^ helper);
        return buddyBlock;
    }
}

//
//size_t align8(size_t s) {
//    if((s & 0x7) == 0)
//        return s;
//    return ((s >> 3) + 1) << 3;
//}
//
//int judge_address(void *addr) {
//    int res = ((uintptr_t)addr - 24) % 256;
//    if(res != 0) {
//        return 0;
//    }
//    return 1;
//}
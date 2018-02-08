#include <stdio.h>
#include "common.h"

//request memory in heap
void *request_memory_from_heap(size_t size){
    void *ret = sbrk(0);
    debug_print("Size: %zu\n", size);
    if((sbrk(size)) == (void *)-1) {
        errno = ENOMEM;
        exit(-1);
    }
    debug_print("Ret Address: %p, newbreak: %p, size: %lu\n", ret, sbrk(0), sbrk(0) - ret);
    return ret;
}

void *allocateMemory(size_t size){
    debug_print("Start malloc!\n");
    //the actual size that needs to require
    size += sizeof(BlockHeader);
    int pageSize = find_page_size(size);
    //initialize the memory in the heap
//    if(heap == NULL) {
//        init_memory(MAX_BLOCK_SIZE);
//    }
    //Check block in the free list
    //if the required memory is bigger than 2048 bytes, we change to use memory map
    //to allocate the memory
    if(size > (MAX_BLOCK_SIZE / 2)) {
        return request_memory_by_mmap(pageSize);
    }
    //**check if there is available space in freeList***//
    int level = get_level(size);
//    char buf[1024];
//    snprintf(buf, 1024, "level: %d\n",
//             level);
//    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    //Step1: prepare for the available block for required memory if necessary
    debug_print("Get available!\n");
    //int split_level = get_available_block(freeList, level);

    //if there is no available bock in freeList[level], we get to the upper freeList to Find
    //bigger block
    int split_level = level;
    debug_print("Here, freeList[level]: %p\n", freeList[level]);
    if(freeList[level] == NULL) {
        int i = level + 1;
        while ((i <= MAX_LEVEL) && (freeList[i] == NULL)) {
            i++;
        }
        //if there is no available space in the biggest level of freeList, we need to require
        //another space in the heap
        //printf("i:%d\n", i);
        debug_print("Here, we need to split from level: %d\n", i);
        if (i > MAX_LEVEL) {
            debug_print("no available heap, needs to extend\n");
            debug_print("Before heap: %p\n", heap);
            heap = (BlockHeader *) request_memory_from_heap(MAX_BLOCK_SIZE);
            //if (freeList[MAX_LEVEL] == NULL) {
            debug_print("After heap: %p\n", heap);
                freeList[MAX_LEVEL] = heap;
                freeList[MAX_LEVEL]->status = 0;
                freeList[MAX_LEVEL]->level = MAX_LEVEL;
                freeList[MAX_LEVEL]->next = NULL;
                freeList[MAX_LEVEL]->previous = NULL;
                split_level = MAX_LEVEL;
            debug_print("extend heap success. freelist[MAX_LEVEL]: %p\n", freeList[split_level]);
            //}
        } else {
            split_level = i;
            debug_print("split level is: %d\n", split_level);
        }
    }

    //split the memory into proper buddy blocks if necessary

    if(freeList[level] == NULL) {
        debug_print("Start split!\n");
        split_buddy(freeList, level, split_level);
    }

//    int i = 0;
//        char buf[4096];
//    for(;i<=7;i++) {
//        snprintf(buf, 4096, "freelist[%d]: 1.startAddress: %p; 2.next: %p; 3.previous: %p; 4. level: %d; 5.status: %d\n",
//                 i, freeList[i], freeList[i]->next, freeList[i]->previous, freeList[i]->level, freeList[i]->status);
//        write(STDOUT_FILENO, buf, strlen(buf) + 1);
//    }

    //Step2: allocate block to the required memory
    debug_print("Start allocate!\n");
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
        debug_print("Malloc Success, address is: %p!\n", (void*)((char*) allocateblock + sizeof(BlockHeader)));
        //return (void*)((char*) allocateblock + sizeof(BlockHeader));
//        if(judge_address((void*)((char*) allocateblock + sizeof(BlockHeader))) == 0) {
//            debug_print("Wrong malloc address! %p\n", (void*)((char*) allocateblock + sizeof(BlockHeader)));
//            exit(-1);
//        }
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
            debug_print("freeList[split_level]: %p\n", freeList[split_level]);
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
            debug_print("Split success! Left Address is: %p, right Address is: %p\n", left_buddy, right_buddy);
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
    return (void *)((char*)bh + sizeof(BlockHeader));
}

//
void free_Memory(void* ptr) {
    debug_print("Start free! Entry Address is: %p\n", ptr);
    BlockHeader *releasedBlock = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    //if the memory is allocated by mmap, use munmap to free that memory
    size_t size = 1 << (releasedBlock->level + MIN_ORDER);
    if(size > (MAX_BLOCK_SIZE / 2)){
        //debug_print("Start free using munmap!\n");
        munmap((void*)(char*)ptr - sizeof(BlockHeader), size);
        //debug_print("Free success using munmap!\n");
        return;
    }
    //remove the releasedblock outside the freelist
    releasedBlock->status = 0;//set to not used
    debug_print("releasedBlock address: %p, RBlevel: %d, status: %d\n", releasedBlock, releasedBlock->level, releasedBlock->status);
    //now begin to determine whether it will merge its buddy into a bigger block or not
    debug_print("Start merge buddy!\n");
    while(releasedBlock->level <= MAX_LEVEL) {
        //link the heap
        if(releasedBlock->level == MAX_LEVEL) {
            releasedBlock->next = freeList[MAX_LEVEL];
            if(freeList[MAX_LEVEL]) {
                freeList[MAX_LEVEL]->previous = releasedBlock;
            }
            releasedBlock->previous = NULL;
            freeList[MAX_LEVEL] = releasedBlock;
            break;
        }
        BlockHeader *buddyBlock = find_buddy(releasedBlock);
        debug_print("BuddyBlock address: %p, BBlevel: %d, status: %d\n", buddyBlock, buddyBlock->level, buddyBlock->status);
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
            break;
        }
    }
    debug_print("free Success!\n");
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

//determine whether it will cause overflow or not
int mulOvf(size_t *result, size_t a, size_t b) {
    *result = a * b;
    if(*result / a != b) {
        return 0;
    }
    return 1;
}

size_t align8(size_t s) {
    if(s & 0x7 == 0)
        return s;
    return ((s >> 3) + 1) << 3;
}

int judge_address(void *addr) {
    int res = ((uintptr_t)addr - 24) % 256;
    if(res != 0) {
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

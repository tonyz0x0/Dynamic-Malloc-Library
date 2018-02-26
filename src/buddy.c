#include <stdio.h>
#include "common.h"

//__thread BlockHeader *freeList[MAX_LEVEL + 1]; //create a freeList array to implement buddy allocation algorithm
__thread ArenaHeader *arena = NULL;
__thread mallinfo *info = NULL;
int arenaNum = 0;
unsigned long int arenaSize = 0;
ArenaHeader *mainThreadStart = NULL;
ThreadHeader *startThreadHeader = NULL;
int init_main_arena_flag = 0;
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
__thread pthread_t tid;

void *allocateMemory(size_t size){
    if(size <= 0) {
        return NULL;
    }
    /*
     * initialize main thread arena
     */
    tid = pthread_self();
    /*
     * initialize thread arena or find spare arena. Judge whether the thread is created or not
     * before, if it was once existed, just assigned the old arena to that thread.
     */
    ThreadHeader *currentThread = startThreadHeader;
    do{
        if(currentThread->tid == tid) {
            arena = currentThread->arena;
            info = (mallinfo*)((char*)arena + sizeof(ArenaHeader));
            //pthread_mutex_unlock(&global_lock);
            break;
        }
        currentThread = currentThread->next;
    } while(currentThread != startThreadHeader);

    if (arena == NULL) {
        pthread_mutex_lock(&global_lock);
        if(arenaNum < NUM_ARENA_64) {
            //debug_print("Thread tid: %u needs a new arena.\n", (unsigned int)tid);
            //pthread_mutex_lock(&global_lock);
            arena = init_thread_arena();
            create_thread_header(arena);
            arenaNum++;
            pthread_mutex_unlock(&global_lock);
        } else {
            //debug_print("Thread tid: %u goes to find an existed arena.\n", (unsigned int)tid);
            //pthread_mutex_lock(&global_lock);
            pthread_mutex_unlock(&global_lock);
            ArenaHeader *current = mainThreadStart;
            while((current != NULL) && (current->status == 1)) {
                current = current->next;
            }
            current->status = 1;
            arena = current;
            info = (mallinfo*)((char*)current + sizeof(ArenaHeader));
            //pthread_mutex_unlock(&global_lock);
        }
    }
    pthread_mutex_lock(&arena->lock);
    info->arenaNum = arenaNum;
    info->alloReq++;
    //the actual size that needs to require
    BlockHeader *ret = NULL;
    size += sizeof(BlockHeader);
    int level = get_level(size);
    /* If the required memory is bigger than 2048 bytes, we go to find free block in mmap list
     * to allocate the memory. If there is no free mmap block, we will call mmap()
     * to ask for a new memory block.
     */
     if(level > MAX_LEVEL) {
         size_t pageSize = find_page_size(size);
         if ((ret = mmap_new_block(pageSize)) != NULL) {
             info->blockUsed++;
             info->blockFree--;
             ret->status = 1;
             ret->next = NULL;
             ret->previous = NULL;
             ret->level = get_level(pageSize);
             arenaSize += pageSize;
         }
     } else {
             /*
              * The required memory is less or equal than 2048 bytes, it will
              * go to arena to find them. If there is no enough block, the arena will
              * extend heap by calling mmap().
              */
             size_t blockSize = 1 << (level + MIN_ORDER);
             if ((ret = find_block(arena, blockSize)) != NULL ||
                 (ret = mmap_new_heap(arena, blockSize)) != NULL) {
                 info->blockUsed++;
                 info->blockFree--;
                 ret->status = 1;
                 ret->next = NULL;
                 ret->previous = NULL;
                 ret->level = get_level(blockSize);
                 arenaSize += blockSize;
             }
         }
        arena->status = 0;
        pthread_mutex_unlock(&arena->lock);
        return (void*)((char*) ret + sizeof(BlockHeader));
}

/*
 * mmap a new block for request memory that is bigger than 2048 bytes
 */
BlockHeader *mmap_new_block(size_t pageSize)
{
    BlockHeader* newBlock = (BlockHeader *)request_memory_by_mmap(pageSize);
    newBlock->status = 0;
    newBlock->level = get_level(pageSize);
    newBlock->next = NULL;
    newBlock->previous = NULL;
    return newBlock;
}

/*
 * go to arena find a free block that is less or equal than 2048 bytes
 */

BlockHeader *find_block(ArenaHeader *arena, size_t size) {
    //**check if there is available space in freeList***//
    //Step1: prepare for the available block for required memory if necessary
    //if there is no available bock in freeList[level], we get to the upper freeList to Find
    //bigger block
    int level = get_level(size);
    int split_level = level;
    if (arena->freeList[level] == NULL) {
        int i = level + 1;
        while ((i <= (MAX_LEVEL)) && (arena->freeList[i] == NULL)) {
            i++;
        }
        //if there is no available space in the biggest level of freeList, return NULL

        if (i > (MAX_LEVEL)) {
            return NULL;
        } else {
            split_level = i;
        }
    }

    //split the memory into proper buddy blocks if necessary

    if (arena->freeList[level] == NULL) {
        split_buddy(arena->freeList, level, split_level);
    }

    //Step2: allocate block to the required memory
    BlockHeader *allocateBlock = arena->freeList[level];
    arena->freeList[level] = allocateBlock->next;
    if (arena->freeList[level] != NULL) {
        arena->freeList[level]->previous = NULL;
    }
    return allocateBlock;
}

/*
 * mmap a new heap for arena and continue to find a free block
 * that is less or equal than 2048 bytes
 */
BlockHeader *mmap_new_heap(ArenaHeader *arena, size_t size)
{
    arena->freeList[MAX_LEVEL] = (BlockHeader*)request_memory_by_mmap(PAGESIZE);
    return find_block(arena, size);
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
            info->blockNum++;
            info->blockFree++;
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
    return map;
}

//
void free_Memory(void* ptr) {
    pthread_mutex_lock(&arena->lock);
    info->freeReq++;
    BlockHeader *releasedBlock = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    //if the memory is allocated by mmap, use munmap to free that memory
    size_t size = 1 << (releasedBlock->level + MIN_ORDER);
    if(size > (MAX_BLOCK_SIZE / 2)){
        //remove_mmap_block(arena, releasedBlock);
        munmap((void*)releasedBlock, size);
        arenaSize -= size;
        info->arenaSize -= size;
        info->blockUsed--;
        info->blockFree++;
        pthread_mutex_unlock(&arena->lock);
        return;
    }

    //remove the releasedblock outside the freelist
    releasedBlock->status = 0;//set to not used

    //now begin to determine whether it will merge its buddy into a bigger block or not
    while(releasedBlock->level <= MAX_LEVEL) {
        //link the heap
        if(releasedBlock->level == MAX_LEVEL) {
            releasedBlock->next = arena->freeList[MAX_LEVEL];
            if(arena->freeList[MAX_LEVEL]) {
                arena->freeList[MAX_LEVEL]->previous = releasedBlock;
            }
            releasedBlock->previous = NULL;
            arena->freeList[MAX_LEVEL] = releasedBlock;
            pthread_mutex_unlock(&arena->lock);
            break;
        }
        BlockHeader *buddyBlock = find_buddy(releasedBlock);

        if(buddyBlock != NULL && buddyBlock->status != 1 && buddyBlock->level == releasedBlock->level) {
            //remove the buddy out of the freelist
            //remove the buddy out of the freelist
            if((buddyBlock->previous) == NULL && buddyBlock->next != NULL) {
                buddyBlock->next->previous = NULL;
                arena->freeList[buddyBlock->level] = buddyBlock->next;
            }
            else if(buddyBlock->previous != NULL && buddyBlock->next != NULL){
                buddyBlock->previous->next = buddyBlock->next;
                buddyBlock->next->previous = buddyBlock->previous;
            }
            else if(buddyBlock->previous != NULL && buddyBlock->next == NULL) {
                buddyBlock->previous->next = NULL;
            }
            else if(buddyBlock->previous == NULL && buddyBlock->next == NULL) {
                arena->freeList[buddyBlock->level] = NULL;
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
            releasedBlock->next = arena->freeList[releasedBlock->level];
            if(arena->freeList[releasedBlock->level]) {
                arena->freeList[releasedBlock->level]->previous = releasedBlock;
            }
            releasedBlock->previous = NULL;
            arena->freeList[releasedBlock->level] = releasedBlock;
            pthread_mutex_unlock(&arena->lock);
            break;
        }
    }
    info->blockFree++;
    info->blockUsed--;
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

/*
 * Initialize the first arena in the main thread;
 */
ArenaHeader* init_main_arena()
{
    /*
     * initialize arena meta data
     */
    ArenaHeader *arena = (ArenaHeader*)request_memory_by_mmap(sizeof(ArenaHeader));
    arena->status = 1;
    arena->next = NULL;
    arena->previous = NULL;
    //arena->lock = PTHREAD_MUTEX_INITIALIZER;
    memset(&(arena->lock), 0, sizeof(pthread_mutex_t));
    memset(&(arena->blockNum), 0, NUM_LINKS * sizeof(uint32_t));

    /*
     * initialize mallinfo meta data
     */
    info = (mallinfo*)((char*)arena + sizeof(ArenaHeader));
    info->blockNum = 0;
    info->alloReq = 0;
    info->arenaSize = 0;
    info->arenaNum = 0;
    info->blockFree = 0;
    info->blockUsed = 0;
    info->freeReq = 0;

    mainThreadStart = arena;
    return arena;
}

/*
 * Initialize the new arena in thread;
 */
ArenaHeader* init_thread_arena()
{
    /*
     * initialize arena meta data
     */
    ArenaHeader *arena = (ArenaHeader*)request_memory_by_mmap(sizeof(ArenaHeader));
    arena->status = 1;
    arena->next = NULL;
    arena->previous = NULL;
    //arena->lock = PTHREAD_MUTEX_INITIALIZER;
    memset(&(arena->lock), 0, sizeof(pthread_mutex_t));
    memset(&(arena->blockNum), 0, NUM_LINKS * sizeof(uint32_t));

    /*
     * initialize mallinfo meta data
     */
    info = (mallinfo*)((char*)arena + sizeof(ArenaHeader));
    info->blockNum = 0;
    info->alloReq = 0;
    info->arenaSize = 0;
    info->arenaNum = 0;
    info->blockFree = 0;
    info->blockUsed = 0;
    info->freeReq = 0;

    /*
     * link the arena in a circle double linked list
     */
    ArenaHeader *current = mainThreadStart;
    if(current->next == NULL) {
        current->next = arena;
        arena->previous = current;
        arena->next = current;
        current->previous = arena;
    } else {
        current->previous->next = arena;
        arena->previous = current->previous;
        arena->next = current;
        current->previous = arena;
    }
    return arena;
}

/*
 * called upon before fork() takes place
 */
void prepare(void)
{
    //pthread_mutex_lock(&global_lock);
    ArenaHeader *current = mainThreadStart;
    while(current) {
        pthread_mutex_lock(&current->lock);
        current = current->next;
    }
}

/*
 * shared by parent and child upon completion of fork()
 */
void parent(void)
{
    ArenaHeader *current = mainThreadStart;
    while(current) {
        pthread_mutex_unlock(&current->lock);
        current = current->next;
    }
    //pthread_mutex_unlock(&global_lock);
}

/*
 * shared by parent and child upon completion of fork()
 */
void child(void)
{
    ArenaHeader *current = mainThreadStart;
    while(current) {
        pthread_mutex_unlock(&current->lock);
        current = current->next;
    }
    //pthread_mutex_unlock(&global_lock);
}

//request memory in heap
void *request_memory_from_sbrk(size_t size){
    void *ret = sbrk(0);
    if((sbrk(size)) == (void *)-1) {
        errno = ENOMEM;
        return NULL;
    }
    return ret;
}

/*
 * Create a thread header
 */
void create_thread_header(ArenaHeader *arena){
    ThreadHeader *th = request_memory_from_sbrk(sizeof(ThreadHeader));
    th->arena = arena;
    th->tid = pthread_self();
    th->next = NULL;

    if(startThreadHeader == NULL) {
        th->next = th;
        startThreadHeader = th;
    }
    ThreadHeader *current = startThreadHeader;
    th->next = current->next;
    current->next = th;
}

__attribute__ ((constructor))
void init() {
    //pthread_atfork(fork_hold, fork_release, fork_release);
    if(init_main_arena_flag == 0) {
        //pthread_mutex_lock(&global_lock);
        //initialize main arena
        arena = init_main_arena();
        //set init_main_arena_flag to 1
        init_main_arena_flag = 1;
        mainThreadStart = arena;
        arenaNum++;
        create_thread_header(arena);
    }
    pthread_atfork(prepare, parent, child);
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
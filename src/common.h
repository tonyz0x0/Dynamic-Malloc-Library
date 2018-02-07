#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>

typedef struct Header{
    unsigned int status;
    unsigned int level;
    struct Header *next;
    struct Header *previous;
} BlockHeader;

#define MIN_LEVEL 0
#define MAX_LEVEL 10
#define MIN_ORDER 6 //2^6->64
#define MAX_ORDER 16 // 2^16->65536
#define MIN_BLOCK_SIZE 64
#define MAX_BLOCK_SIZE 65536

static __thread pthread_mutex_t lock;
static __thread BlockHeader *heap = NULL; //point to the beginning of the heap
static __thread BlockHeader *freeList[MAX_LEVEL + 1]; //create a freeList array to implement buddy allocation algorithm


//#ifdef DEBUG
//    #define debug(fmt, ...) printf(fmt" f(): %s\n", ##__VA_ARGS__, __func__)
//#else
//    #define debug(fmt, ...)
//#endif

void *request_memory_from_heap(size_t size);
void *allocateMemory(size_t size);
void init_memory(size_t size);
void get_available_block(BlockHeader **freeList, int level);
void split_buddy(BlockHeader **freeList, int level);
size_t find_page_size(size_t size);
int get_level(size_t size);
void *request_memory_by_mmap(size_t pageSize);
void free_Memory(void* ptr);
BlockHeader *find_buddy(BlockHeader* releasedBlock);
void *realloc(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
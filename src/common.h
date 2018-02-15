#ifndef COMMON_H
#define COMMON_H

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

#define PAGESIZE sysconf(_SC_PAGESIZE)
#define MIN_LEVEL 0
#define MAX_LEVEL 8 /* 0~7 is for buddy allocation, 8 is for mmap section */
#define MIN_ORDER 5 //2^5->32
#define MAX_ORDER 12 // 2^12->4096
#define MIN_BLOCK_SIZE 32
#define MAX_BLOCK_SIZE PAGESIZE
#define CPU sysconf(_SC_NPROCESSORS_ONLN)
#define NUM_ARENA_64 (8*CPU)
#define NUM_LINKS (MAX_LEVEL+1)

#define DEBUG 1
#ifdef DEBUG
# define debug_print(...) fprintf (stderr, __VA_ARGS__)
#else
# define debug_print(...) do {} while (0)
#endif


/*
 * struct for block header
 */
typedef struct Header{
    uint32_t status;
    uint32_t level;
    struct Header *next;
    struct Header *previous;
} BlockHeader;


/*
 * struct for per-core arena
 */
typedef struct arenaheader{
    pthread_mutex_t lock;
    uint32_t status;
    uint32_t blockNum[NUM_LINKS];
    BlockHeader *freeList[NUM_LINKS];
    void *startAddr;
    struct arenaheader *next;
    struct arenaheader *previous;
} ArenaHeader;

/*
 * global variables
 */
int init_main_arena_flag = 0;
ArenaHeader* mainArenaAddr = NULL;
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * struct for malloc info
 */
typedef struct Struct{
    unsigned int arenaSize;     /* Total size of arena allocated with sbrk/mmap */
    unsigned int arenNum;   /* Total number of arenas */
    unsigned int blockNum;    /* Total number of blocks for each arena*/
    unsigned int blockUsed;     /* Number of Used blocks for each arena */
    unsigned int blockFree;    /* Number of Free blocks for each arena */
    unsigned int alloReq;   /* Total allocation requests for each arena */
    unsigned int freeReq;   /* Total free requests for each arena */
    unsigned int uordblks;  /* Total allocated space (bytes) */
    unsigned int fordblks;  /* Total free space (bytes) */
} mallinfo;

typedef void *(*my__malloc_hook)(size_t size, const void *caller);
typedef void (*my__free_hook)(void *ptr, const void *caller);
typedef void *(*my__realloc_hook)(void *ptr, size_t size, const void *caller);
typedef void *(*my__calloc_hook)(size_t nmemb, size_t size, const void *caller);
typedef void *(*my__memalign_hool)(size_t alignment, size_t size);
void *request_memory_from_heap(size_t size);
void *allocateMemory(size_t size);
void split_buddy(BlockHeader **freeList, int level, int split_level);
size_t find_page_size(size_t size);
int get_level(size_t size);
void *request_memory_by_mmap(size_t pageSize);
void free_Memory(void* ptr);
BlockHeader *find_buddy(BlockHeader* releasedBlock);
void *realloc(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);
void *memalign(size_t alignment, size_t size);
int posix_memalign(void **memptr, size_t alignment, size_t size);
void malloc_stats();

//size_t align8(size_t size);
//int judge_address(void *addr);

#endif
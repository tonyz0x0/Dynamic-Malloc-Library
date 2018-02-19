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
#define MAX_LEVEL 7 /* 0~7 is for buddy allocation*/
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
    struct arenaheader *next;
    struct arenaheader *previous;
} ArenaHeader;

/*
 * struct for thread
 */
typedef struct threadheader{
    pthread_t tid;
    ArenaHeader *arena;
    struct threadheader *next;
} ThreadHeader;

/*
 * global variables
 */
extern ArenaHeader *mainThreadStart;

/*
 * struct for malloc info
 */
typedef struct Struct{
    unsigned long int arenaSize;     /* Total size of arena allocated with sbrk/mmap */
    unsigned long int arenaNum;   /* Total number of arenas */
    unsigned long int blockNum;    /* Total number of blocks for each arena*/
    unsigned long int blockUsed;     /* Number of Used blocks for each arena */
    unsigned long int blockFree;    /* Number of Free blocks for each arena */
    unsigned long int alloReq;   /* Total allocation requests for each arena */
    unsigned long int freeReq;   /* Total free requests for each arena */
} mallinfo;

//void *request_memory_from_heap(size_t size);
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
void malloc_stats(void);
//BlockHeader *find_mmap_block(ArenaHeader *arena, size_t size);
BlockHeader *mmap_new_block(size_t pageSize);
BlockHeader *mmap_new_heap(ArenaHeader *arena, size_t size);
//void remove_mmap_block(ArenaHeader *arena, BlockHeader *releasedBlock);
ArenaHeader* init_main_arena();
ArenaHeader* init_thread_arena();
BlockHeader *find_block(ArenaHeader *arena, size_t size);
void *request_memory_by_mmap(size_t pageSize);
void *request_memory_from_sbrk(size_t size);
void create_thread_header(ArenaHeader *arena);
void prepare(void);
void parent(void);
void child(void);
//size_t align8(size_t size);
//int judge_address(void *addr);

#endif
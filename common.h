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


typedef struct Header{
    uint32_t status;
    uint32_t level;
    struct Header *next;
    struct Header *previous;
} BlockHeader;

#define PAGESIZE sysconf(_SC_PAGESIZE)
#define MIN_LEVEL 0
#define MAX_LEVEL 6
#define MIN_ORDER 6 //2^6->64
#define MAX_ORDER 12 // 2^12->4096
#define MIN_BLOCK_SIZE 32
#define MAX_BLOCK_SIZE PAGESIZE

#define DEBUG 1
#ifdef DEBUG
# define debug_print(...) fprintf (stderr, __VA_ARGS__)
#else
# define debug_print(...) do {} while (0)
#endif

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
//size_t align8(size_t size);
//int judge_address(void *addr);

#endif
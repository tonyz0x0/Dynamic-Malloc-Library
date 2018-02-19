# Dynamic Malloc Library

An implementation of thread-safe Dynamic Malloc Library using buddy allocation scheme.

### Design Overview
This malloc library includes these functions:
```sh
       void *malloc(size_t size);
       void free(void *ptr);
       void *calloc(size_t nmemb, size_t size);
       void *realloc(void *ptr, size_t size);
       void *reallocarray(void *ptr, size_t nmemb, size_t size);
       int posix_memalign(void **memptr, size_t alignment, size_t size);
       void *memalign(size_t alignment, size_t size);
	   void malloc _stats(void);
```
The main method is using buddy allocation algorithm to manage memory blocks. Each memory block consists of a header, which contains some meta data of that block. Each CPU core can maintain up to 8 arenas, so each thread can operate in one arena sperately. 
### Functions
Each function can be found in each c file. The main boddy of this program is buddy.c and commom.h file. Two test file, t-test1.c and test1, can use to check the correctness of the program.
### Data Structure
The program maintains a linklist of arena, each arena maintains an array of 7 doubly linked lists called FreeList. Each thread has its own threadheader. The header of each memory block, thread and arena looks like this:
```sh
typedef struct Header{
    uint32_t status;
    uint32_t level;
    struct Header *next;
    struct Header *previous;
} BlockHeader;

typedef struct arenaheader{
    pthread_mutex_t lock;
    uint32_t status;
    uint32_t blockNum[NUM_LINKS];
    BlockHeader *freeList[NUM_LINKS];
    struct arenaheader *next;
    struct arenaheader *previous;
} ArenaHeader;

typedef struct threadheader{
    pthread_t tid;
    ArenaHeader *arena;
    struct threadheader *next;
} ThreadHeader;
```
### Allocation Design
This program mainly uses Buddy Allocation Scheme to manage memory in operating system level. A classical scenario of requiring memory from operating system is below:
1. If the user requires a memory which its size is greater than 2048 bytes, the system will allocate using mmap() system call.
2. If the user requires less than or equals to 2048 bytes, the system will allocate the memory from the heap.
3. Every time the program will call a mmap(PAGESIZE) system call to extend heap once the memory is not enough.
4. Every time the allocated memory block is called to release by function free(), the program will maintain the management of memory in heap by using Buddy Allocation Algorithm.

### Multithreading and Fork Support
The program is thread-safe, it can be processed in multithreading. Each thread has its own arena, so it is totally safe.
The allocation can be done with fork() operation as well.

### Building and Testing
For building and testing in multithreading scenario:
```sh
$ make check
```
The t-test1.c file contains some parameters for testing. Change different value of them can change different testing level.

There is a test screenshot posted, named Test.png.  Ut creates 5 threads(plus one main thread) and run 150 nodes test, the result of arenas can print successfully.

### Existing Bugs
A malloc_stats() bug existed sometimes when the program goes to call fork(). 


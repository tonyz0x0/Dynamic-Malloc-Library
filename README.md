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
```
The main method is using buddy allocation algorithm to manage memory blocks. Each memory block consists of a header, which contains some meta data of that block.
### Functions
Each function can be found in each c file. The main boddy of this program is buddy.c file. Two test file, t-test1.c and test1, can use to check the correctness of the program.
### Data Structure
The program maintains an array of 7 doubly linked lists called FreeList. The header of each memory block looks like this:
```sh
typedef struct Header{
    uint32_t status;
    uint32_t level;
    struct Header *next;
    struct Header *previous;
} BlockHeader;
```
### Allocation Design
This program mainly uses Buddy Allocation Scheme to manage memory in operating system level. A classical scenario of requiring memory from operating system is below:
1. If the user requires a memory which its size is greater than 2048 bytes, the system will allocate using mmap() system call.
2. If the user requires less than or equals to 2048 bytes, the system will allocate the memory from the heap.
3. Every time the program will call a sbrk(PAGESIZE) system call to extend heap once the memory is not enough.
4. Every time the allocated memory block is called to release by function free(), the program will maintain the management of memory in heap by using Buddy Allocation Algorithm.

### Multithreading Support
The program is thread-safe, it can be processed in multithreading.

### Building and Testing
For building and testing in multithreading scenario:
```sh
$ make check
```
The t-test1.c file contains some parameters for testing. Change different value of them can change different testing level.

### Further Improvements
1. Per-Core Malloc Arenas
2. Support for fork
3. Print Malloc Statistics
4. more...

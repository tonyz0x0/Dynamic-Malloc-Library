project(hw3)
set(THREADS_PEREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_C_STANDARD 99)
add_executable(hw3 malloc.c calloc.c buddy.c common.h free.c realloc.c t-test1.c memalign.c posix_memalign.c reallocarray.c malloc_stats.c)
target_link_libraries(hw3 Threads::Threads)
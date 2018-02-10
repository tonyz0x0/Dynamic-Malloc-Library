#Sample Makefile for Malloc
CC=gcc
CFLAGS=-g -O0 -fPIC -Werror -Wall

TESTS=test1 t-test1
HEADERS=buddy.c malloc.c realloc.c free.c calloc.c reallocarray.c posix_memalign.c memalign.c

all:	${TESTS} libmalloc.so

clean:
	rm -rf *.o *.so ${TESTS}

libmalloc.so: buddy.c malloc.c realloc.c free.c calloc.c reallocarray.c posix_memalign.c memalign.c
	$(CC) $(CFLAGS) -shared -Wl,--unresolved-symbols=ignore-all buddy.c malloc.c realloc.c free.c calloc.c reallocarray.c posix_memalign.c memalign.c -o $@

%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread

# For every XYZ.c file, generate XYZ.o.
%.o: %.c ${HEADERS}
	$(CC) $(CFLAGS) $< -c -o $@ -lpthread

check1:	libmalloc.so test1
	LD_PRELOAD=`pwd`/libmalloc.so ./test1

check:	libmalloc.so test1 t-test1
	LD_PRELOAD=`pwd`/libmalloc.so ./t-test1

debug: libmalloc.so t-test1
    LD_PRELOAD=`pwd`/libmalloc.so valgrind --log-file=valgrind.log --tool=memcheck --leak-check=full --show-reachable=no --workaround-gcc296-bugs=yes ./t-test1

dist: clean
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar

#include <stdio.h>
#include"common.h"


void *malloc(size_t size) {
    //debug_print("Start malloc entrance!\n");
    if(size <= 0) {
        //debug_print("return malloc NULL!\n");
        return NULL;
    }
    void *res;
    //size =  size_align(size);
    res = allocateMemory(size);
    //debug_print("malloc success! The allocateblock address is : %p\n", res);
    return res;
}
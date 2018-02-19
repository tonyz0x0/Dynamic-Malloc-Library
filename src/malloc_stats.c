#include <stdio.h>
#include "common.h"

void malloc_stats(void) {
    ArenaHeader *currentArena = mainThreadStart;
    mallinfo *currentInfo = (mallinfo*)((char*)currentArena + sizeof(ArenaHeader));
    printf("**********Malloc Stats***************\n");
    int i = 1;
    do {
        printf("\n====Arena %d====\n", i);
        printf("**Total number of blocks in Arena %d: %lu\n",i , currentInfo->blockNum);
        printf("**Number of used blocks in Arena %d: %lu\n",i , currentInfo->blockUsed);
        printf("**Number of free blocks in Arena %d: %lu\n",i , currentInfo->blockFree);
        printf("**Total allocation requests in Arena %d: %lu\n",i , currentInfo->alloReq);
        printf("**Total free requests in Arena %d: %lu\n",i , currentInfo->freeReq);
        i++;
        currentArena = currentArena->next;
        currentInfo = (mallinfo*)((char*)currentArena + sizeof(ArenaHeader));
    } while((currentArena != mainThreadStart) && (i <= NUM_ARENA_64));
    printf("\n===========Total Information=============\n");
    currentInfo = (mallinfo*)((char*)currentArena->next + sizeof(ArenaHeader));
    printf("**Total Size of Arena Allocated: %lu\n", currentInfo->arenaSize);
    printf("**Total Number of Arena: %lu\n", currentInfo->arenaNum);
}
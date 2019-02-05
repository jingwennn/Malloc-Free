#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>

//thread safe malloc and free function: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

//thread safe malloc and free function: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

#endif

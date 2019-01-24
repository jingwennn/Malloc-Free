#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>

//first fit malloc and free function
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//best fit malloc and free function
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//print the whole memory list to check
void printlist();

//get the whole data size
unsigned long get_data_segment_size();

//get the free data size
unsigned long get_data_segment_free_space_size();


#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

typedef struct block {
  size_t size;
  int available;
  void *address;
  struct block *free_prev;
  struct block *free_next;
}block;

//Thread local storage
//no-locking version
//Assign the linked list to each thread

//pointer to the start of the free memory
__thread block *free_head=NULL;

//locking version
block *free_head_lock=NULL;

// locks for sbrk
pthread_mutex_t sbrk_mutex;
// locks for locking version
pthread_mutex_t lock_mutex;




//find the free memory for best fit
//if failed, return NULL
block * bf_findmemory(block *p_free_head, size_t size){
  block *curr=p_free_head;
  block *bf_find=NULL;
  size_t min_size=INT_MAX;

  while (curr!=NULL){
    //stop searching if finding the memory with the same size
    if(curr->size==size){
      bf_find=curr;
      break;
    }
    // find the suitable memory with the smallest size
    if (curr->size>size){
      if(curr->size<min_size){
	        min_size=curr->size;
	        bf_find=curr;
      }
    }
    curr=curr->free_next;
  }
  return bf_find;
}


// split one memory into two, the second memory has the size of "size"
// return the second memory
block *split_block(block *curr, size_t size){
  block *split_new=curr->address+curr->size-size-sizeof(block);

  split_new->size=size;
  split_new->available=0;
  split_new->address=(void*)split_new+sizeof(block);
  split_new->free_prev=NULL;
  split_new->free_next=NULL;

  //the first one remains free
  curr->size=curr->size-size-sizeof(block);
  curr->available=1;

  return split_new;
}                                                                                                                             
 

// increase the memory space by calling sbrk
// for non-lock version
block *add_memory_nolock(size_t size){
  block *curr=NULL;
  pthread_mutex_lock(&sbrk_mutex);
  curr=sbrk(size+sizeof(block));
  pthread_mutex_unlock(&sbrk_mutex);

  //create a new block to store information
  curr->size=size;
  curr->available=0;
  curr->address=(void *)curr+sizeof(block);
  curr->free_prev=NULL;
  curr->free_next=NULL;

  return curr;
}


// increase the memory space by calling sbrk
// for lock version
block *add_memory_lock(size_t size){
  block *curr=NULL;

  curr=sbrk(size+sizeof(block));
  
  //create a new block to store information
  curr->size=size;
  curr->available=0;
  curr->address=(void *)curr+sizeof(block);
  curr->free_prev=NULL;
  curr->free_next=NULL;

  return curr;
}



// delete a memory from the free linked list, which means it is unavailable from now
void delete_block(block **p_free_head, block * curr){
  block *f_prev=curr->free_prev;
  block *f_next=curr->free_next;

  if(f_prev!=NULL)
    f_prev->free_next=f_next;
  if(f_next!=NULL)
    f_next->free_prev=f_prev;

  if(curr->free_prev == NULL){
    *p_free_head=f_next;
  }
  
  curr->free_prev=NULL;
  curr->free_next=NULL;
  curr->available=0;
}



// thread safe malloc (non-locking version)
void *ts_malloc_nolock(size_t size){
  block * curr=NULL;
  
  if (free_head!=NULL){
    curr=bf_findmemory(free_head, size);
  }

  if(curr==NULL){
    curr=add_memory_nolock(size);
  }
  else{
    if(curr->size>=(size+sizeof(block)+8)){
      curr=split_block(curr, size);
    }
    else{
      delete_block(&free_head, curr);
    }
  }
                          
  return curr->address;
}

// thread safe malloc (locking version)
void *ts_malloc_lock(size_t size){
  pthread_mutex_lock(&lock_mutex);
  block * curr=NULL;

  if (free_head_lock!=NULL){
    curr=bf_findmemory(free_head_lock, size);
  }

  if(curr==NULL){
    curr=add_memory_lock(size);
   }
  else{
    if(curr->size>=(size+sizeof(block)+8)){
      curr=split_block(curr, size);
    }
    else{
      delete_block(&free_head_lock, curr);
    }
  }

  pthread_mutex_unlock(&lock_mutex);                                                                                                                                           
  return curr->address;
}


// thread safe free (non-locking version)
void ts_free_nolock(void *ptr){          
  // find the memory to free                                             
  block *curr=ptr-sizeof(block);        
  if(curr->available){
    return;
  }
  curr->available=1;
 // add this memory to the free linked list                                        
  curr->free_prev=NULL;
  curr->free_next=NULL;
  if(free_head!=NULL){
    free_head->free_prev=curr;
    curr->free_next=free_head;
  }
  
  free_head=curr;

}

                    

// thread safe free (locking version)
void ts_free_lock(void *ptr){          
  // find the memory to free                                                        
  pthread_mutex_lock(&lock_mutex);                              
  block *curr=ptr-sizeof(block);       
  if(curr->available){
    pthread_mutex_unlock(&lock_mutex);
    return;
  }
  curr->available = 1;
  // add this memory to the free linked list 
  curr->free_prev=NULL;
  curr->free_next=NULL;
  if(free_head_lock!=NULL){
    free_head_lock->free_prev=curr;
    curr->free_next=free_head_lock;
  }  
  free_head_lock=curr;

  pthread_mutex_unlock(&lock_mutex);                                                                                        
}



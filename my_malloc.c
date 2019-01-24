#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

typedef struct block {
  struct block *prev;
  struct block *next;
  size_t size;
  int available;
  void *address;
  struct block *free_prev;
  struct block *free_next;
}block;


//pointer to the start of the whole memory 
block *head=NULL;  
//pointer to the start of the free memory
block *free_head=NULL;
//pointer to the tail of the whole memory
block *tail=NULL;


//print the memory list to check
void printlist(){
  block * curr = head;

  printf("The memory list:\n");
  while(curr){
    printf("%d : curr.address: %d curr.next: %d curr.prev: %d curr.free_prev: %d curr.free_next: %d curr.available: %d curr.size: %d\n" , curr, curr->address,curr->next, curr->prev,curr->free_prev,curr->free_next, curr->available, curr->size );
    curr = curr->next;
  }
  printf("\n");
}

//find the free memory for first fit
//if failed, return NULL
block * ff_findmemory(size_t size){
  block *curr=free_head;
  block *ff_find=NULL;
  while (curr!=NULL){
    if (curr->size>=size){
      ff_find=curr;
      break;
    }
    curr=curr->free_next;
  }
  return ff_find;
}

//find the free memory for best fit
//if failed, return NULL
block * bf_findmemory(size_t size){
  block *curr=free_head;
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


// split one memory into two, the first memory has the size of "size"
// return the first memory
block *split_block(block *curr, size_t size){
  block *split_new=curr->address+size;

  split_new->prev=curr->prev;
  split_new->next=curr;
  split_new->size=size;
  split_new->address=curr->address;
  split_new->available=0;
  split_new->free_prev=NULL;
  split_new->free_next=NULL;

  if(curr->prev!=NULL){
    curr->prev->next=split_new;
  }

  // the second one remains free
  curr->prev=split_new;
  curr->size=curr->size-size-sizeof(block);
  curr->address=(void*)split_new+sizeof(block);

  if(curr==head)
    head=split_new;
  
  return split_new;
}                                                                                                                             
 
// increase the memory space by calling sbrk
block *add_memory(size_t size){
  block *curr=NULL;
  sbrk(size+sizeof(block));
  
  if(tail==NULL){
    curr=sbrk(0)-sizeof(block);
   }
  else{
    curr=(void*)tail+size+sizeof(block);
  }

  //create a new block to store information
  curr->prev=tail;
  curr->next=NULL;
  curr->size=size;
  curr->available=0;
  curr->address=(void *)curr-size;
  curr->free_prev=NULL;
  curr->free_next=NULL;

  if (tail!=NULL){
    tail->next=curr;
    tail=curr;
  }
  else{
    head=curr;
    tail=curr;
  }

  return curr;
}

// delete a memory from the free linked list, which means it is unavailable from now
void delete_block(block * curr){
  block *f_prev=curr->free_prev;
  block *f_next=curr->free_next;

  if(f_prev!=NULL)
    f_prev->free_next=f_next;
  else
    free_head=f_next;
  if(f_next!=NULL)
    f_next->free_prev=f_prev;

  curr->free_prev=NULL;
  curr->free_next=NULL;
  curr->available=0;
}
 
// add a memory to the free linked list
void add_block(block *curr){
  curr->available=1;
  
  //search the previous free memory
  block *f_prev=curr->prev;
  while(f_prev!=NULL){
    if(f_prev->available==1)
      break;
    f_prev=f_prev->prev;
  }
  //change the free linked list
  if(f_prev==NULL){
    curr->free_prev=NULL;
    curr->free_next=free_head;
    if(free_head!=NULL)
      free_head->free_prev=curr;
    free_head=curr;
  }
  else{
    if(f_prev->free_next==NULL){
      f_prev->free_next=curr;
      curr->free_prev=f_prev;
      curr->free_next=NULL;
    }
    else{
      curr->free_next=f_prev->free_next;
      f_prev->free_next->free_prev=curr;
      f_prev->free_next=curr;
      curr->free_prev=f_prev;
    }
  }
}

// merge one memory with the next one
void merge(block *curr){
  // change the linked list, but not the free linked list
  if(free_head==curr)
    free_head=curr->next;
  if (curr->prev!=NULL){
    curr->prev->next=curr->next;
  }
  else{
    head=curr->next;
  }

  curr->next->prev=curr->prev;
  curr->next->size+=curr->size+sizeof(block);
  curr->next->address=curr->address;

  //change the free linked list
  delete_block(curr);
}

  
// malloc for first fit
void *ff_malloc(size_t size){
  block * curr=NULL;
  if (free_head!=NULL){
    curr=ff_findmemory(size);
  }

  // if no free memory has the enough size, increase the whole memory
  if(curr==NULL){
    curr=add_memory(size);
  }

  else{
    // if the rest memory can store data with size of 8, split this memory
    if(curr->size>=(size+sizeof(block)+8)){
      curr=split_block(curr, size);
      }
      else{
        delete_block(curr);
        }
  }
  return curr->address;
}

// malloc for best fit
void *bf_malloc(size_t size){
  block * curr=NULL;
  if (free_head!=NULL){
    curr=bf_findmemory(size);
  }

  if(curr==NULL){
    curr=add_memory(size);
  }
  else{
    if(curr->size>=(size+sizeof(block)+8)){
      curr=split_block(curr, size);
    }
    else{
      delete_block(curr);
    }
  }
                                                                                                                                              
  return curr->address;
}

// free for first fit
void ff_free(void *ptr){          
  // find the memory to free                                                                                               
  block *curr=NULL;                                                                                                              
  if (head->address==ptr){                                                                                                       
    curr=head;                                                                                                                   
  }                                                                                                                              
  else{                                                                                                                          
    curr=ptr-sizeof(block);                                                                                                      
    curr=curr->next;                                                                                                             
  } 

  // can not free this one                                                                                                                            
  if(curr==NULL)                                                                                                                 
    {                                                                                                                            
      printf("Can not free this!\n");                                                                                                                                                                              
    }                                                                                                                            

  // add this memory to the free linked list                                                                                                                              
  add_block(curr);                                                                                                               

  // merge this memory if there are other free spaces next to it
  //merge with next one
  if (curr->next!=NULL && curr->next->available==1)                                                                              
    merge(curr);
  //merge with previous one
  if(curr->prev!=NULL){
    curr=curr->prev;
    if(curr->available==1)
      merge(curr);
  }
                                                                                                            
}

// free for best free                                                                                                                            
void bf_free(void * ptr){
  ff_free(ptr);
}
                      
// the whole memory size                                                 
unsigned long get_data_segment_size(){
  unsigned long datasize=0;
  block *curr=head;
  while(curr!=NULL){
    datasize=datasize+sizeof(block)+curr->size;
    curr=curr->next;
  }
  return datasize;
}

//the free memory size
unsigned long get_data_segment_free_space_size(){
  unsigned long datasize=0;
  block *curr=free_head;
  while(curr!=NULL){
    datasize=datasize+sizeof(block)+curr->size;
    curr=curr->free_next;
  }
  return datasize;
}

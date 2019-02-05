# Malloc-Free
Implement Malloc and Free functions from the C standard library.    
2 versions of malloc and free, each based on a different strategy for determining the memory region to allocate. The two strategies are:   
1. First Fit: Examine the free space tracker, and allocate an address from the first free region with enough space to fit the requested allocation size.   
2. Best Fit: Examine all of the free space information, and allocate an address from the free region which has the smallest number of bytes greater than or equal to the requested allocation size.   
    
For first fit policy:   
void *ff_malloc (size_t size)    
void ff_free (void *ptr)              

For best fit policy:    
void *bf_malloc (size_t size)    
void bf_free (void *ptr)   

To call these functions, include the code below before the main function:    
#ifdef FF    
#define MALLOC(sz) ff_malloc(sz).     
#define FREE(p)    ff_free(p)    
#endif    
#ifdef BF     
#define MALLOC(sz) bf_malloc(sz)    
#define FREE(p)    bf_free(p)     
#endif
    
    
     

Thread safe versions of malloc and free function.   

Thread safe malloc/free: locking version  (pthread_mutex)
void *ts_malloc_lock (size_t size);  
void *ts_free_lock (void *ptr);  
   
Thread safe malloc/free: no-locking version  (thread-local storage)
void *ts_malloc_nolock (size_t size);  
void *ts_free_nolock (void *ptr);

# Malloc-Free
Implement Malloc and Free functions from the C standard library. 
 
For first fit policy:
void *ff_malloc (size_t size) 
void ff_free (void *ptr)

For best fit policy:
void *bf_malloc (size_t size) 
void bf_free (void *ptr)

To call these functions, include the code below before the main function:
#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p)    ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p)    bf_free(p)
#endif

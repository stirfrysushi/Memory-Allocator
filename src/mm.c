#include <string.h>
#include <stdio.h>
#include <unistd.h> 

/* The standard allocator interface from stdlib.h.  These are the
 * functions you must implement, more information on each function is
 * found below. They are declared here in case you want to use one
 * function in the implementation of another. */
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

/* When requesting memory from the OS using sbrk(), request it in
 * increments of CHUNK_SIZE. */
#define CHUNK_SIZE (1<<12)

/*
 * This function, defined in bulk.c, allocates a contiguous memory
 * region of at least size bytes.  It MAY NOT BE USED as the allocator
 * for pool-allocated regions.  Memory allocated using bulk_alloc()
 * must be freed by bulk_free().
 *
 * This function will return NULL on failure.
 */
extern void *bulk_alloc(size_t size);

/*
 * This function is also defined in bulk.c, and it frees an allocation
 * created with bulk_alloc().  Note that the pointer passed to this
 * function MUST have been returned by bulk_alloc(), and the size MUST
 * be the same as the size passed to bulk_alloc() when that memory was
 * allocated.  Any other usage is likely to fail, and may crash your
 * program.
 */
extern void bulk_free(void *ptr, size_t size);

/*
 * This function computes the log base 2 of the allocation block size
 * for a given allocation.  To find the allocation block size from the
 * result of this function, use 1 << block_size(x).
 *
 * Note that its results are NOT meaningful for any
 * size > 4088!
 *
 * You do NOT need to understand how this function works.  If you are
 * curious, see the gcc info page and search for __builtin_clz; it
 * basically counts the number of leading binary zeroes in the value
 * passed as its argument.
 */
static inline __attribute__((unused)) int block_index(size_t x) {
    if (x <= 8) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}


/* create a free list */ 
typedef struct block {
    size_t block_header; 
    struct block *next;  
} block; 

/*array of pointers */ 
struct block **ptr; 

//creating a free list and return pointer to first block: 
static void *free_list(size_t size) {
   
    //create free list table for specific index 
    int index = 0; 
    size_t block_size = 0;
    int number_of_blocks = 0;
    void *sbrk_ptr; 
    struct block *head;  
    int i = 0;
    
    index = block_index(size);
    block_size = 1 << index;
    number_of_blocks = CHUNK_SIZE/block_size; 
    sbrk_ptr = sbrk(CHUNK_SIZE);  
    head = *(block*)sbrk_ptr; 
    //make pointer point to sbrk  
    ptr[index] = *(block*)sbrk_ptr; 
    
    while(i <= number_of_blocks) {
    	//for last block, next -> NULL:
	if(i == number_of_blocks) {
		ptr[index]-> block_header = block_size; 
		ptr[index] -> next = NULL; 
	}
	else {
		ptr[index] -> block_header = block_size; 
		sbrk_ptr += block_size; 
		ptr[index] -> next = *(block*)sbrk_ptr;
		i += 1;
	}
   }
   return head; 
} 

void *malloc(size_t size) {

    void *returned_ptr; 
    int index = 0;
    int bit = 0; 
    size_t header = 0; 
    struct block *head; 

    if(size == 0 || size < 0) {
    	return NULL; 
    }

    if(size <= 4088) {
    	index = block_index(size);
	head = ptr[index]; 
	//checking if list is made:
	if(head == NULL) {
		head = free_list(size);
		bit = head -> block_header; 
		if((bit & 1) == 0) { 
			returned_ptr = head;
			header = (head -> block_header) | 1; 
			//set flag
		        head -> block_header = header; 	
			returned_ptr += 8; 
			return returned_ptr; 
		
		//else if block is not free: 
		} else {
			while(head != NULL){
				bit = head -> block_header; 
				if((bit & 1) == 0) {
					returned_ptr = head;  
					header = (head -> block_header) | 1;
					head -> block_header = header; 
					returned_ptr += 8; 
					return returned_ptr; 
				} else {
					head = head -> next; 
				}
			}
		}
	}
      //if list has already been made: 
      else {
		while(head != NULL){
			bit = head -> block_header; 
			if((bit & 1) == 0) {
				returned_ptr = head; 
				header = (head -> block_header) | 1;
				head -> block_header = header; 
				returned_ptr += 8; 
				return returned_ptr; 
			} else {
				head = head -> next; 
			}
		}
	}
    } 
    return bulk_alloc(size);
}

/*
 * You must also implement calloc().  It should create allocations
 * compatible with those created by malloc().  In particular, any
 * allocations of a total size <= 4088 bytes must be pool allocated,
 * while larger allocations must use the bulk allocator.
 *
 * calloc() (see man 3 calloc) returns a cleared allocation large enough
 * to hold nmemb elements of size size.  It is cleared by setting every
 * byte of the allocation to 0.  You should use the function memset()
 * for this (see man 3 memset).
 */
void *calloc(size_t nmemb, size_t size) {
    void *ptr = bulk_alloc(nmemb * size);
    memset(ptr, 0, nmemb * size);
    return ptr;
}

/*
 * You must also implement realloc().  It should create allocations
 * compatible with those created by malloc(), honoring the pool
 * alocation and bulk allocation rules.  It must move data from the
 * previously-allocated block to the newly-allocated block if it cannot
 * resize the given block directly.  See man 3 realloc for more
 * information on what this means.
 *
 * It is not possible to implement realloc() using bulk_alloc() without
 * additional metadata, so the given code is NOT a working
 * implementation!
 */
void *realloc(void *ptr, size_t size) {
	if(ptr == NULL) {
		return malloc(size); 
	} else {
		return NULL; 
	}
}

/*
 * You should implement a free() that can successfully free a region of
 * memory allocated by any of the above allocation routines, whether it
 * is a pool- or bulk-allocated region.
 *
 * The given implementation does nothing.
 */
void free(void *ptr) {
    return;
}

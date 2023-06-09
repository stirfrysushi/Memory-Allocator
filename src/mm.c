#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

    /*array of *block pointers */ 
    static struct block *array_ptr[13] ={ NULL }; 

    //creating a free list and return pointer to first block: 
    static void *free_list(size_t size) {
       
        //create free list table for specific index 
        int index = 0; 
        size_t block_size = 0;
        int number_of_blocks = 0;
        void *sbrk_ptr; 
        struct block *first_block; 
        struct block *other_block; 
        int i = 0;
        
        index = block_index(size);
        block_size = 1 << block_index(size); 
        number_of_blocks = CHUNK_SIZE/block_size;

        //setting up first block  
        sbrk_ptr = sbrk(CHUNK_SIZE);
        first_block = (block*)sbrk_ptr;
        first_block -> block_header = size; 

        sbrk_ptr += block_size; 
        first_block -> next = (block*)sbrk_ptr;

        array_ptr[index] = first_block; 
        while(i <= number_of_blocks - 2){
            other_block = (block*)sbrk_ptr; 

           //last block 
            if(i == number_of_blocks - 2) {
                other_block -> block_header = size; 
                other_block -> next = NULL;
        }

            other_block -> block_header = size;
            sbrk_ptr += block_size; 
            other_block-> next = (block*)sbrk_ptr; 
            i += 1;  
        } 
           return array_ptr[index]; 
       }

    void *malloc(size_t size) {

        void *returned_ptr; 
        int index = 0; 
     
        if(size == 0 || size < 0) {
            return NULL; 
        }

        if(size <= 4088) {
            index = block_index(size); 
        
        //checking if free list is initialized : 
            if(array_ptr[index] == NULL) {

                //free list return a pointer pointing to the first block     
                array_ptr[index] = free_list(size);
                returned_ptr = (void*)array_ptr[index]; 
                returned_ptr += 8; 
            //change head
            array_ptr[index] = array_ptr[index] -> next; 
                return returned_ptr; 

            } else {    
                returned_ptr = (void*)array_ptr[index];
                returned_ptr += 8;
            array_ptr[index] = NULL;
                return returned_ptr; 
        }
        }
        //if size > 4088 
        else {
        returned_ptr = bulk_alloc(size + 8); 
        *(size_t*)returned_ptr = size + 8; 
        returned_ptr += sizeof(size_t); 
        return returned_ptr; 
        }
    }

    /*
      You must also implement calloc().  It should create allocations
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
        if(nmemb == 0 || size == 0) {
            return NULL; 
        } 
        if((nmemb * size) <= 4088) {
          void *ptr = malloc(nmemb * size);
            memset(ptr,0, nmemb * size); 
            return ptr; 
        } 
        else { 
            void *ptr = bulk_alloc((nmemb * size)+8);
	    *(size_t*)ptr = (nmemb * size) + 8; 
	    ptr = ptr + sizeof(size_t); 
            memset(ptr, 0, nmemb * size);
            return ptr;
        } 
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

        size_t user_size;  
        void *returned_ptr;
        void *temp;
         

        temp = ptr;

        if(ptr == NULL && size > 0) {
            returned_ptr = malloc(size); 
        return returned_ptr; 
        }

        if(size == 0 && ptr != NULL) {
            free(ptr + 8);
        return NULL;  
        } 

        ptr = ptr - 8; 
        user_size = *(size_t*)ptr; 
     

        if(size > user_size - 8) {
            returned_ptr = malloc(size); 
        returned_ptr = memcpy(returned_ptr, temp, user_size - 8); 
        free(ptr + 8);
        return returned_ptr; 
        } 
        else {
            return temp; 
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
        size_t user_size = 0;
        int index = 0;
        struct block *new_block; 
        struct block *temp; 

        if(ptr == NULL) {
            return; 
        } else {
            ptr = ptr - 8; 
            user_size = *(size_t*)ptr;
            if(user_size > CHUNK_SIZE){
                bulk_free(ptr, user_size); 
                return; 
            } else {
                index = block_index(user_size); 
            if(array_ptr[index] == NULL){
                array_ptr[index] = (block*)ptr;
            array_ptr[index] -> block_header = user_size; 
            array_ptr[index] -> next = NULL;
            return; 
            }
            else { 
                new_block = (block*)ptr; 

                if(array_ptr[index] -> next == NULL) { 
                new_block -> next = NULL; 
                new_block -> block_header = user_size; 
                array_ptr[index] -> next = new_block; 
                return; 
            } else {
                temp = array_ptr[index]; 

                array_ptr[index] = new_block; 
                array_ptr[index] -> block_header = user_size; 
                array_ptr[index] -> next = temp; 
                return;
                }
            }
        }
        }
    }

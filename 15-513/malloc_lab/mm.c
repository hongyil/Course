/*
 * Name: Hongyi Liang
 * Andrew ID:hongyil
 * Project: Malloc lab
 *
 ******************************************************************************
 *                               mm.c                                         *
 *           64-bit struct-based segregated free list memory allocator        *
 *                  15-213: Introduction to Computer Systems                  *
 *                                                                            *
 *  ************************************************************************  *
 * 
 * mm.c: a malloc simulator,which also support calloc method
 *  
 * the allocator uses segregated free lists with LIFO policy to hold the blocks 
 * there are 16 segregated lists which are associated with distinct size ranges
 * the min block size is 2*wsize(or dsize),and the chunk size is 2**8 bytes
 * heap consists of Prologue footer and Epilogue header,with 2*wsize empty space
 *
 * the header/footer for the blocks can be seen as follow:
 *
 *      header:  |block size|third bit|second bit|alloc bit|
 *      footer:  |            block size               |a/f|
 *
 * the info can be described as:
 * 1:third bit:to indicate if size is appropriate
 * 2:second bit: to indicate if previous block is allocated
 * 3:alloc bit: to indicate if current block is allocated
 * note that we don't need to write the footer for an allocated block
 *
 * reference:mm-baseline.c
 ******************************************************************************
 */

/* Do not change the following! */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <stddef.h>

#include "mm.h"
#include "memlib.h"

#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mem_memset
#define memcpy mem_memcpy
#endif /* def DRIVER */

/* You can change anything from here onward */

/*
 * If DEBUG is defined, enable printing on dbg_printf and contracts.
 * Debugging macros, with names beginning "dbg_" are allowed.
 * You may not define any other macros having arguments.
 */
// #define DEBUG // uncomment this line to enable debugging

#define DEBUG

#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#define dbg_checkheap(...) mm_checkheap(__VA_ARGS__)
#else
/* When debugging is disnabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#define dbg_checkheap(...)
#endif

#define NUM_LIST 16

/* Basic constants */
typedef uint64_t word_t;
static const size_t wsize = sizeof(word_t);   // word and header size (bytes)
static const size_t dsize = 2*wsize;          // double word size (bytes)
static const size_t min_block_size = dsize;   // Minimum block size
static const size_t chunksize = (1 << 9);     // requires (chunksize % 16 == 0)

static const word_t alloc_mask = 0x1;
static const word_t second_bit_mask = 0x2;
static const word_t third_bit_mask = 0x4;
static const word_t size_mask = ~(word_t)0xF;

typedef struct block
{
    /* Header contains size + allocation flag */
    word_t header;
    /*
     * We don't know how big the payload will be.  Declaring it as an
     * array of size 0 allows computing its starting address using
     * pointer notation.
     */
    char payload[0];
    /*
     * We can't declare the footer as part of the struct, since its starting
     * position is unknown
     */
} block_t;


/* Global variables */
/* Pointer to first block */
static block_t *heap_start = NULL;

/* initialize new segregated free list */
static block_t *segregated_list[NUM_LIST];

bool mm_checkheap(int lineno);

/* Function prototypes for internal helper routines */
static block_t *extend_heap(size_t size);
static void place(block_t *block, size_t asize);
static block_t *find_fit(size_t asize);
static block_t *coalesce(block_t *block);

static size_t max(size_t x, size_t y);
static size_t round_up(size_t size, size_t n);
static word_t pack(size_t size, bool alloc);

static size_t extract_size(word_t header);
static size_t get_size(block_t *block);
static size_t get_payload_size(block_t *block);

static bool extract_alloc(word_t header);
static bool get_alloc(block_t *block);

static void write_header(block_t *block, size_t size, bool alloc);
static void write_footer(block_t *block, size_t size, bool alloc);

static block_t *payload_to_header(void *bp);
static void *header_to_payload(block_t *block);

static word_t *find_prev_footer(block_t *block);
static block_t *find_next(block_t *block);
static block_t *find_prev(block_t *block);

static bool check_in_heap(void *bp);
static bool is_aligned(void *bp);
static bool check_block(block_t *block);
static void insert_list(block_t *block);
static void remove_list(block_t *block);
static void write_bit(block_t *block, size_t new_bit);
static void free_bit(block_t *block,int bit);
static size_t read_bit(block_t *block,int bit);
static size_t get_block_offset(size_t asize);

/*
 * mm_init:perform any necessary initializations,like allocating heap area
 *         
 * inintialize the segregated list,pointing to null
 * return false if there was a problem in performing initialization
 * source reference:csapp textbook and mm-baseline.c
 */
bool mm_init(void){

    // Create the initial empty heap
    word_t *start = (word_t *)(mem_sbrk(2*wsize));

    if (start == (void *)-1){
        return false;
    }

    start[0] = pack(dsize,true); // Prologue footer
    start[1] = pack(dsize,true); // Epilogue header
    start[1] |= second_bit_mask;
    // Heap starts with first "block header", currently the epilogue footer
    heap_start = (block_t *) &(start[1]);
    start += 2*wsize;

    for (int i = 0; i < NUM_LIST; i++){
        segregated_list[i] = NULL;
    }

    // Extend the empty heap with a free block of chunksize bytes
    if (extend_heap(chunksize) == NULL){
        return false;
    }
    return true;
}

/*
 * malloc:return a pointer to an allocated block of at least size bytes
 *
 * size aligned to 16 bytes and find a fit block within heap
 * and extend the heap when needed
 * source reference:mm-baseline.c
 */
void *malloc(size_t size)
{
    //dbg_requires(mm_checkheap(__LINE__));
    size_t asize;      // Adjusted block size
    size_t extendsize; // Amount to extend heap if no fit is found
    block_t *block;
    void *bp = NULL;

    if (heap_start == NULL) // Initialize heap if it isn't initialized
    {
        mm_init();
    }

    if (size == 0) // Ignore spurious request
    {
        dbg_ensures(mm_checkheap(__LINE__));
        return bp;
    }

    // Adjust block size to include overhead and to meet alignment requirements
    asize=round_up(size+wsize,16);
    
    // Search the free list for a fit
    block = find_fit(asize);

    // If no fit is found, request more memory, and then and place the block
    if (block == NULL)
    {  
        extendsize = max(asize, chunksize);
        block = extend_heap(extendsize);
        if (block == NULL) // extend_heap returns an error
        {
            return bp;
        }

    }

    place(block, asize);
    bp = header_to_payload(block);

    //dbg_ensures(mm_checkheap(__LINE__));
    return bp;
}

/*
 * free: free the block pointed to by *bp and return nothing
 *
 * free(NULL) has no effect;write the header/footer
 * target block should have been allocated earlier
 */
void free (void *bp){

    if (bp == NULL){
        return;
    }

    block_t *block = payload_to_header(bp);
    size_t size = get_size(block);
    size_t second_bit = read_bit(block, 2);
    size_t third_bit = read_bit(block, 3);
    write_header(block, size, false);
    write_footer(block, size, false);
    write_bit(block, second_bit);
    write_bit(block, third_bit);
    coalesce(block);

    return;
}

/*
 * realloc:return a pointer to an allocated region of
 *         at least size bytes
 *
 * if ptr==null,call malloc()
 * copy(and free) old data to new block
 * source reference:mm-baseline.c
 */
void *realloc(void *ptr, size_t size)
{
    block_t *block = payload_to_header(ptr);
    size_t copysize;
    void *newptr;

    // If size == 0, then free block and return NULL
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    // If ptr is NULL, then equivalent to malloc
    if (ptr == NULL)
    {
        return malloc(size);
    }

    // Otherwise, proceed with reallocation
    newptr = malloc(size);
    // If malloc fails, the original block is left untouched
    if (newptr == NULL)
    {
        return NULL;
    }

    // Copy the old data
    copysize = get_payload_size(block); // gets size of old payload
    if(size < copysize)
    {
        copysize = size;
    }
    memcpy(newptr, ptr, copysize);

    // Free the old block
    free(ptr);

    return newptr;
}

/*
 * calloc:allocate memory for an array and return a pointer
 *                 to the allocated memory
 *
 * the memory is set to 0 before returning
 * source reference:mm-baseline.c
 */
void *calloc(size_t elements, size_t size)
{
    void *bp;
    size_t asize = elements * size;

    if (asize/elements != size)
    // Multiplication overflowed
    return NULL;
    
    bp = malloc(asize);
    if (bp == NULL)
    {
        return NULL;
    }
    // Initialize all bits to 0
    memset(bp, 0, asize);

    return bp;
}


/******** The remaining content below are helper and debug routines ********/

/*
 * extend_heap:extending heap space when needed
 *
 * reset the block header/footer and create a new epilogue header
 * return a coalesced block of previously free space
 */
static block_t *extend_heap(size_t size){

    void *bp;

    // Allocate an even number of words to maintain alignment
    size = round_up(size,16);
    if ((bp = mem_sbrk(size)) == (void *)-1){
        return NULL;
    }

    // Initialize free block header/footer
    block_t *block = payload_to_header(bp);
    size_t second_bit = read_bit(block, 2);
    size_t third_bit = read_bit(block, 3);
    write_header(block, size, false);
    write_footer(block, size, false);
    write_bit(block, third_bit);
    write_bit(block, second_bit);
    // Create new epilogue header
    block_t *block_next = find_next(block);
    write_header(block_next, 0, true);

    // Coalesce in case the previous block was free
    return coalesce(block);
}

/*
 * coalesce: possibly join previous/current/next block
 *           and preform in constant time
 *
 * case 1: both adjacent blocks are allocated
 *         no coalescing is possible,free current block
 * case 2: current blocked is merged with next block
 *         free the block,size combined
 *         header:current block;footer:next block
 * case 3: previous blocked is merged with next block
 *         free the block,size combined
 *         header:previous block;footer:current block
 * case 4: three blocks are merged,size combined
 *         header:previous block;footer:next block
 */
static block_t *coalesce(block_t * block){

    block_t *block_next = find_next(block);
    block_t *block_prev = find_prev(block);

    size_t prev_alloc = read_bit(block, 2);
    size_t next_alloc = get_alloc(block_next); 
    size_t size = get_size(block);

    if (read_bit(block,3)){
        block_prev = (block_t *)((char*)block - dsize);
    }

    if (prev_alloc && next_alloc){ // Case 1

        insert_list(block);
        free_bit(block_next, 2);
        if (size == min_block_size){
            write_bit(block_next, third_bit_mask);
        }
        return block;

    }else if (prev_alloc && !next_alloc){ // Case 2
        size += get_size(block_next);
        remove_list(block_next);
        free_bit(find_next(block_next), 2);
        write_bit(block, prev_alloc);
        write_header(block, size, false);
        write_footer(block_next, size, false);

    }else if (!prev_alloc && next_alloc){ // Case 3     
        size += get_size(block_prev);
        remove_list(block_prev);
        free_bit(block_next, 2);
        write_bit(block_prev, second_bit_mask);
        write_header(block_prev, size, false);
        write_footer(block, size, false);
        block = block_prev;
        
    }else { // Case 4
        size += get_size(block_next) + get_size(block_prev);
        remove_list(block_prev);
        remove_list(block_next);
        free_bit(block_next, 2);
        free_bit(find_next(block_next), 2);
        write_bit(block_prev, second_bit_mask);
        write_header(block_prev, size, false);   
        write_footer(block_next, size, false); 
        block = block_prev;
    }
    insert_list(block);

    return block;
}

/*
 * place:place the asize-block to heap space
 *
 * if target block has enough space,split it for asize
 * else find next appropriate block
 * need to write previous the header/footer for both cases
 */
static void place(block_t *block, size_t asize){
    
    block_t *block_next;
    size_t csize=get_size(block);

    remove_list(block);
    if ((csize - asize) >= min_block_size){
        write_header(block, asize, true);
        write_footer(block, asize, true);          
        block_next = find_next(block);       
        write_header(block_next, csize-asize, false);  
        write_footer(block_next, csize-asize, false);
        write_bit(block, second_bit_mask);
        write_bit(block_next, second_bit_mask);
        coalesce(block_next);
    }else{
        block_next = find_next(block);
        write_header(block, csize, true);
        write_footer(block, csize, true);
        write_bit(block, second_bit_mask);
        write_bit(block_next, second_bit_mask);
    }

    return;
}

/*
 * find_fit:search the free list for a fit
 * 
 * locate a proper block in the free list according to asize
 * compare the asize to next 8 block sizes and update the info to find a fit 
 * in case splitting the block, return null if no fit found
 */
static block_t *find_fit(size_t asize){

    block_t *block;
    block_t *block_next;

    size_t i;
    size_t cur_size,next_size;
    size_t idx=get_block_offset(asize);

    for (;idx<NUM_LIST;++idx){
        //first locate target block
        block = segregated_list[idx];
        while (block){
            cur_size = get_size(block);
            if (asize<=cur_size){
                //best fit within next 8 blocks
                block_next = *(block_t **)((char *)block + wsize);
                for (i=idx;i<idx+8;i++){
                    if (block_next){
                        //update to closer block
                        next_size = get_size(block_next);
                        if ((asize<=next_size) && (next_size<=cur_size)){
                            block = block_next;
                            cur_size = next_size;
                        }
                        block_next = *(block_t **)((char *)block_next + wsize);
                    }
                }    
                return block;
            }
            block = *(block_t **)((char *)block + wsize);
        }
    }
    return NULL; // no fit found
}

/* 
 * insert_list:add a block to segregated free list
 * 
 * locate the the targeted block in the list based on size
 *
 * 1: block allocated & equal to min block size,update previous pointer
 * else update next pointer
 * 2: block not allocated & equal to min block size,update previous pointer
 * else update next pointer
 */
static void insert_list(block_t *block){

    //first locate the target block
    size_t size = get_size(block);
    size_t index = get_block_offset(size);
    block_t *block_cur = segregated_list[index];

    if (block_cur){
        *(block_t **)((char *)block+wsize) = block_cur;
        //update the next pointer
        if (size!=min_block_size){  
            *(block_t **)((char *)block+dsize) = NULL;
            *(block_t **)((char *)block_cur+dsize) = block;
        }
    }else{
        *(block_t **)((char *)block+wsize) = NULL;
        if (size!=min_block_size){
            *(block_t **)((char *)block+dsize) = NULL;
        }
    }
    segregated_list[index] = block;

    return;
}

/* 
 * remove_list:remove a block in segregated free list
 * 
 * locate the the targeted block in the list based on size
 * 1: if size not equal to min block size find next block
 *    else keep looking for a block which fits target block
 * 2: if target block's previous block is null,the block is ahead
 *    else if next block is null, the block is following
 */
static void remove_list(block_t *block){

    size_t size = get_size(block);
    size_t index=get_block_offset(size);

    block_t *block_prev = NULL;
    block_t *block_cur = segregated_list[index];
    block_t *block_next = *(block_t **)((char *)block + wsize);

    if (size!=min_block_size){
        block_prev = *(block_t **)((char *)block+dsize);
    }else{
        //loop until proper block is found
        while(block_cur!=block){
            block_prev = block_cur;
            block_cur = *(block_t **)((char *)block_cur+wsize);
        }
    }
    
    if ((size!=min_block_size)&&(block_next)){
        *(block_t **)((char *)block_next + dsize) = block_prev;
    }

    if (!block_prev){
        segregated_list[index] = block_next;
    }else{
        *(block_t **)((char *)block_prev + wsize) = block_next;
    }
    return;
}

/*
 * mm_checkheap:scan the heap and check it for possible error
 *
 * count free blocks,to see if pointers matched
 * segregated list consists of an array of block
 * loop over the list and check each block
 * to check if the number of free block matches the number of free list
 */
bool mm_checkheap(int line){

    printf("checking heap\n");

    size_t i,size;
    size_t free_block = 0;
    block_t *block,*block_cur;
    
    if (!heap_start){
        printf("null heap root\n");
        return false;
    }else{
        block=heap_start;
    }

    i=0;
    size = get_size(block);

    while (size>0){
        if (!check_block(block)){
            printf("invalid block\n");
            return false;
        }

        if (!get_alloc(block)){
            free_block+=1;
        }

        block = find_next(block);
        size = get_size(block);
    }
    
    while (i<NUM_LIST){
        block_cur = segregated_list[i];
        while(block_cur){
            free_block-=1;
            block_cur = *(block_t **)((char *)block_cur + wsize);
        }
        i+=1;
    }

    if (free_block){
        printf("number of free block not matching free list\n");
        return false;
    }

    return true;

}

/**********************************************
 check_heap helper function
***********************************************/

//check heap boundary
static bool check_in_heap(void *bp){
    return ((bp <= mem_heap_hi()) && (bp >= mem_heap_lo()));
}

//check address alignment
static bool is_aligned(void *bp){
    return ((size_t)(bp))%16==0;
}

/*
 * check_block:check each single block
 * check: address alignment;heap boundary;minimum size;consecutive blocks
 */
static bool check_block(block_t *block){

    bool valid = false;
    size_t size = get_size(block);

    if (!is_aligned(block)){
        printf("Payload address (%p) not aligned to 16 bytes\n", block);

    }else if(size<min_block_size){
        printf("Not appropriate block size\n");

    }else if (!check_in_heap(block)){
        printf("Payload (%p:%p) lies outside heap\n", mem_heap_hi(),mem_heap_lo());

    }else if (!get_alloc(block)){
        //no two consecutive free blocks
        block_t *block_next = find_next(block);
        block_t *next_free = *(block_t **)((char *)block + wsize);

        if (block_next!=next_free){
            valid=false;
            printf("Two consecutive free blocks\n");
        }
    }else{
        valid=true;
    }

    return valid;
}


/*
 *****************************************************************************
 * The functions below are short wrapper functions to perform                *
 * bit manipulation, pointer arithmetic, and other helper operations.        *
 *                                                                           *
 * We've given you the function header comments for the functions below      *
 * to help you understand how this baseline code works.                      *
 *                                                                           *
 * Note that these function header comments are short since the functions    *
 * they are describing are short as well; you will need to provide           *
 * adequate details within your header comments for the functions above!     *
 *                                                                           *
 *                                                                           *
 * Do not delete the following super-secret(tm) lines!                       *
 *                                                                           *
 * 53 6f 20 79 6f 75 27 72 65 20 74 72 79 69 6e 67 20 74 6f 20               *
 *                                                                           *
 * 66 69 67 75 72 65 20 6f 75 74 20 77 68 61 74 20 74 68 65 20               *
 * 68 65 78 61 64 65 63 69 6d 61 6c 20 64 69 67 69 74 73 20 64               *
 * 6f 2e 2e 2e 20 68 61 68 61 68 61 21 20 41 53 43 49 49 20 69               *
 *                                                                           *
 *                                                                           *
 * 73 6e 27 74 20 74 68 65 20 72 69 67 68 74 20 65 6e 63 6f 64               *
 * 69 6e 67 21 20 4e 69 63 65 20 74 72 79 2c 20 74 68 6f 75 67               *
 * 68 21 20 2d 44 72 2e 20 45 76 69 6c 0a de ad be ef 0a 0a 0a               *
 *                                                                           *
 *****************************************************************************
 */


/*
 * max: returns x if x > y, and y otherwise.
 */
static size_t max(size_t x, size_t y)
{
    return (x > y) ? x : y;
}

/*
 * round_up: Rounds size up to next multiple of n
 */
static size_t round_up(size_t size, size_t n)
{
    return (n * ((size + (n-1)) / n));
}

/*
 * pack: returns a header reflecting a specified size and its alloc status.
 *       If the block is allocated, the lowest bit is set to 1, and 0 otherwise.
 */
static word_t pack(size_t size, bool alloc)
{
    return alloc ? (size | alloc_mask) : size;
}

/*
 * extract_size: returns the size of a given header value based on the header
 *               specification above.
 */
static size_t extract_size(word_t word)
{
    return (word & size_mask);
}

/*
 * get_size: returns the size of a given block by clearing the lowest 4 bits
 *           (as the heap is 16-byte aligned).
 */
static size_t get_size(block_t *block)
{
    return extract_size(block->header);
}

/*
 * get_payload_size: returns the payload size of a given block, equal to
 *                   the entire block size minus the header and footer sizes.
 */
static word_t get_payload_size(block_t *block)
{
    size_t asize = get_size(block);
    return asize - wsize;
}

/*
 * extract_alloc: returns the allocation status of a given header value based
 *                on the header specification above.
 */
static bool extract_alloc(word_t word)
{
    return (bool)(word & alloc_mask);
}

/*
 * get_alloc: returns true when the block is allocated based on the
 *            block header's lowest bit, and false otherwise.
 */
static bool get_alloc(block_t *block)
{
    return extract_alloc(block->header);
}

/*
 * write_header: given a block and its size and allocation status,
 *               writes an appropriate value to the block header.
 */
static void write_header(block_t *block, size_t size, bool alloc)
{
    block->header = pack(size, alloc);
}

/*
 * write_footer: given a block and its size and allocation status,
 *               writes an appropriate value to the block footer by first
 *               computing the position of the footer.
 */
static void write_footer(block_t *block, size_t size, bool alloc)
{
    word_t *footerp = (word_t *)((block->payload) + get_size(block) - dsize);
    *footerp = pack(size, alloc);
}

/*
 * find_next: returns the next consecutive block on the heap by adding the
 *            size of the block.
 */
static block_t *find_next(block_t *block)
{
    dbg_requires(block != NULL);
    block_t *block_next = (block_t *)(((char *)block) + get_size(block));
    
    dbg_ensures(block_next != NULL);
    return block_next;
}

/*
 * find_prev_footer: returns the footer of the previous block.
 */
static word_t *find_prev_footer(block_t *block)
{
    // Compute previous footer position as one word before the header
    return (&(block->header)) - 1;
}

/*
 * find_prev: returns the previous block position by checking the previous
 *            block's footer and calculating the start of the previous block
 *            based on its size.
 */
static block_t *find_prev(block_t *block)
{
    word_t *footerp = find_prev_footer(block);
    size_t size = extract_size(*footerp);
    return (block_t *)((char *)block - size);
}


/*
 * payload_to_header: given a payload pointer, returns a pointer to the
 *                    corresponding block.
 */
static block_t *payload_to_header(void *bp)
{
    return (block_t *)(((char *)bp) - offsetof(block_t, payload));
}

/*
 * header_to_payload: given a block pointer, returns a pointer to the
 *                    corresponding payload.
 */
static void *header_to_payload(block_t *block)
{
    return (void *)(block->payload);
}

/*************************************************
 *additional helper functions
 *************************************************/

/*
 * read_bit:get the bit of a block in some positions
 */
static size_t read_bit(block_t *block, int bit){

    switch (bit){
    case 2:
        return ((word_t)(block->header)) & second_bit_mask;
    default:
        return ((word_t)(block->header)) & third_bit_mask; 
    }
}

/*
 * write_bit:re-write the bit of a block in some positions
 */
static void write_bit(block_t *block, size_t new_bit){

    block->header = (block->header) | new_bit;
}

/*
 * free_bit:reset the bit of a block in some positions to 0
 */
static void free_bit(block_t *block,int bit){

    switch (bit){
    case 2:
        block->header = (block->header) & (~second_bit_mask);
    default:
        block->header = (block->header) & (~third_bit_mask);
    }
}

/*
 * get_block_offset: return the index in the segregated list by size
 *                   require i<NUM_LIST
 */
static size_t get_block_offset(size_t asize){

    size_t i;

    if(asize<32){
        i=0;
    }else if(asize<64){
        i=1;
    }else if(asize<128){
        i=2;
    }else if(asize<256){
        i=3;
    }else if(asize<512){
        i=4;
    }else if(asize<1024){
        i=5;
    }else if(asize<2048){
        i=6;
    }else if(asize<4096){
        i=7;
    }else if(asize<9192){
        i=8;
    }else if(asize<18384){
        i=9;
    }else if(asize<36768){
        i=10;
    }else if(asize<73536){
        i=11;
    }else if(asize<147072){
        i=12;
    }else if(asize<294144){
        i=13;
    }else if(asize<588288){
        i=14;
    }else{
        i=15;
    }
    return i;
}
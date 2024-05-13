#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define PAGE_SIZE 4096
#pragma GCC diagnostic ignored "-Wunused-variable"

int scribble = 0;

struct mem_block {
  /**
   * The name of this memory block. If the user doesn't specify a name for the
   * block, it should be left empty (a single null byte).
   */
  char name[8];

  /** Size of the block */
  uint64 size;

  /** Links for our doubly-linked list of blocks: */
  struct mem_block *next_block;
  struct mem_block *prev_block;
} __attribute__((packed));

struct free_block {
  struct mem_block block_header;
  struct mem_block *next_free;
  struct mem_block *prev_free;
} __attribute__((packed));

// Block list
struct mem_block *blh = NULL;
struct mem_block *blt = NULL;

// Free list
struct free_block *flh = NULL;
struct free_block *flt = NULL;

// Function pointer to fsm algorithm implementation
struct mem_block* (*fsm_algorithm)(uint) = NULL;

uint 
align(uint orig_size, uint alignment)
{
  uint new_size = (orig_size / alignment) * alignment;
  if (orig_size % alignment != 0) {
      new_size += alignment;
  }
  return new_size;
}

uint 
real_size(uint size)
{
  return size & ~(0x01);
}

/**
 * Lets user scribble malloced blocks
 */
void
malloc_scribble(void)
{
  scribble = 1;
}

void
block_scribble (struct mem_block *block)
{
  char *data = (char *)(block + 1);
  uint size = block->size - sizeof(struct mem_block);
  memset(data, 0, size);

  while (size) {
    *data = 0xAA;
    data++;
  	size = size >> 8;
  }  
}

void
malloc_name(void *b, char *name)
{ 
  struct mem_block *block = (struct mem_block *)b - 1;
  strcpy(block->name, name);
}

// Prints block list
void
block_print(void)
{
  char *status = NULL;
  struct mem_block *walk = blh;

  printf("\n-- Current Memory State --\n");

  while (walk) {
    if (walk->size & 1) {
      status = "FREE";
    } else {
      status = "USED";
    }

    uint size = real_size(walk->size);
  
    printf("[BLOCK 0x%x-0x%x] %d [%s] '%s'\n", walk, (char *)walk + size, size, status, walk->name);
  	walk = walk->next_block;
  }
}

// Prints block list in reverse
void
block_reverse(void)
{
  struct mem_block *walk = blt;

  printf("Reverse malloc: \n");
  
  while (walk) {
    printf("{%x:%s:%d} -> ", walk, walk->name, walk->size);
    walk = walk->prev_block;
  }
  printf("NULL\n");
}

// Adds to end of block list
void
add_block(struct mem_block *block)
{
  if (blh == NULL) {
    // Case 1: Head is empty
  	blh = block;
  	blt = block;
  	return;
  } 

  struct mem_block *walk = blh;

  while (walk) {
    // Block already exists in list, update
    if (block == walk) {
      block->next_block = walk->next_block;
      block->prev_block = walk->prev_block;
      walk = block;
      return;
    }
    walk = walk->next_block;
  }
  blt->next_block = block;
  block->prev_block = blt;
  blt = block;
  blt->next_block = NULL;
}

// Prints free list
void
free_print(void)
{
  printf("\n-- Free List --\n");
  struct free_block *walk = flh;

  while (walk) {
  	printf("[%x:%s:%d] -> ", walk, walk->block_header.name, walk->block_header.size);
  	walk = (struct free_block *)walk->next_free;
  }
  printf("NULL\n"); 	
}

// Prints free list in reverse
void
free_reverse(void)
{
  printf("Reverse free: \n");
  struct free_block *walk = flt;

  while (walk) {
  	printf("[%x:%s:%d] -> ", walk, walk->block_header.name, walk->block_header.size);
  	walk = (struct free_block *)walk->prev_free;
  }
  printf("NULL\n");   
}

// Adds to end of free list
void
add_free(struct free_block *fblk) 
{
  if (!flh && !flt) {
    // Case 1: Head is empty
  	flh = fblk;
  	flt = fblk;
  	flh->next_free = NULL;
  	flt->prev_free = NULL;
  	return;
  } 

  struct free_block *walk = flh;

  while (walk) {
    // Block already exists in list, update
    if (fblk == walk) {
      fblk->next_free = walk->next_free;
      fblk->prev_free = walk->prev_free;

      if (walk->next_free) {
        ((struct free_block *)walk->next_free)->prev_free = (struct mem_block *)fblk;
      }

      if (fblk->prev_free) {
      	((struct free_block *)walk->prev_free)->next_free = (struct mem_block *)fblk;
      }

      walk = fblk;
      return;
    }
    walk = (struct free_block *)walk->next_free;
  }
  flt->next_free = (struct mem_block *)fblk;
  fblk->next_free = NULL;
  fblk->prev_free = (struct mem_block *)flt;
  flt = fblk;
}

void
remove_free(struct free_block *fblk)
{
  if (flh == fblk && flt == fblk) {
    // Case 1: fblk is only item in list
	flh = NULL;
	flt = NULL;
	fblk->next_free = NULL;
	fblk->prev_free = NULL;
  } else if (flh == fblk) {
    // Case 2: fblk is at head
	flh = (struct free_block *)flh->next_free;
	fblk->next_free = NULL;
	flh->prev_free = NULL;
  } else if (flt == fblk) {
    // Case 3: fblk is at end
    flt = (struct free_block *)flt->prev_free;
    fblk->prev_free = NULL;
    flt->next_free = NULL;
  } else {
    struct free_block *next = NULL;
    struct free_block *prev = NULL;
    
    struct free_block *walk = flh;    
    
    while (walk) {
      if (fblk == walk) {
        // Block already exists in list, update
        next = (struct free_block *)fblk->next_free;
        prev = (struct free_block *)fblk->prev_free;

		if (next && prev) {
          next->prev_free = (struct mem_block *)prev;
          prev->next_free = (struct mem_block *)next;			
		} else if (next) {
		  next->prev_free = NULL;
		} else {
		  prev->next_free = NULL;
		}

        fblk->next_free = NULL;
        fblk->prev_free = NULL;

        return;
      }
      walk = (struct free_block *)walk->next_free;
    }
    // Do nothing if not in list
  }
  fblk->next_free = NULL;
  fblk->prev_free = NULL;
}

int
blt_free(void)
{
  uint size = blt->size;

  if ((size & 1) && size >= PAGE_SIZE) {
    remove_free((struct free_block *)blt);
    struct mem_block *prev = blt->prev_block;

    if (prev) {
      prev->next_block = NULL;
      blt->prev_block = NULL;
      blt = prev;	
    } else {
      blh = NULL;
      blt = NULL;
    }
    
  	sbrk(-(real_size(size)));

  	return 1;
  }

  // No changes to blt list
  return 0;
}

void
malloc_print()
{
  block_print();
  block_reverse();
  free_print();
  free_reverse();
}

void 
malloc_leaks(void)
{
  uint lblks = 0, lbytes = 0;
  struct mem_block *walk = blh;

  printf("-- Leak Check --\n");

  while (walk) {
    if (!(walk->size & 1)) {
      uint size = real_size(walk->size);
      printf("[BLOCK 0x%x] %d '%s'\n", walk, size, walk->name);
      lbytes += size;
      lblks++;
    }
  	walk = walk->next_block;
  }  

  printf("-- Summary --\n");
  printf("%d blocks lost (%d bytes)\n", lblks, lbytes);
}


struct free_block
*merge_blocks(struct mem_block *block)
{
  // If prev block is free
  if (block->prev_block && block->prev_block->size & 1) {
  
    struct mem_block *prev = block->prev_block;
    
    remove_free((struct free_block *)block);
    
    prev->size += real_size(block->size);
    prev->next_block = block->next_block;
    strcpy(prev->name, block->name);

    if (blt == block) {
      blt = prev;
    } else if (block->next_block) {
	  block->next_block->prev_block = prev;    	
    } 

    // Clear out header from block
    memset(block, 0, sizeof(struct mem_block));

    block = prev;
  }
  
  if (block->next_block && block->next_block->size & 1) {
  	struct mem_block *next = block->next_block;
  	
  	block->size += real_size(next->size);
  	block->next_block = next->next_block;

  	remove_free((struct free_block *)next);
  	
    if (blt == next) {
      blt = block;
      next->prev_block = NULL;
      blt->next_block = NULL;
    } else if (next->next_block) {
      next->next_block->prev_block = block;   	
    }

    // Clear out header from next
    memset(next, 0, sizeof(struct mem_block));  
  }

  int changed = blt_free();

  if (changed) {
    // Current block has merged to end, no longer exists
  	return NULL;
  }
  
  return (struct free_block *)block;
}

/**
 * Splits block if necessary
 * Adds split block after block in block list
 * Adds split block in free list
 */
struct mem_block
*split_block(struct mem_block *block, uint req_sz) 
{
  struct mem_block *split = NULL;
  add_block(block);

  // Align block, will always be multiple of 16
  uint offset = align(req_sz + sizeof(struct free_block), 16);

  // Checking if there is ample size for splitting
  if (real_size(block->size) > offset) {
  	// Make split block
  	split = (struct mem_block *)((char *)block + req_sz);

  	// Adding split block after block in block list
  	split->size = (block->size - req_sz) | 1;
  	split->next_block = NULL;
  	split->prev_block = block;

    struct mem_block *next = block->next_block;

    if (next) {
	  split->next_block = next;    
	  next->prev_block = split;	
    } else {
      // Split block is at the end of the list
      blt = split;
    }
	block->next_block = split;
  } 

  // Change size for block, clear free flag
  remove_free((struct free_block *)block);

  if (split) {
    block->size = req_sz; 
    add_free((struct free_block *)split);
  	merge_blocks(split);  	
  }
  
  return block;
}

struct mem_block
*first_fit(uint total_bytes)
{
  struct free_block *next = flh;

  while (next) {
    uint nsz = real_size(next->block_header.size);
  	if (nsz >= total_bytes) {
  	  return (struct mem_block *)next;
  	}
  	next = (struct free_block *)next->next_free;
  }
  // Reaches here if empty list
  return NULL;
}

struct mem_block
*best_fit(uint total_bytes)
{
  struct free_block *next = flh;
  
  struct free_block *best = NULL;

  while (next) {
    uint nsz = real_size(next->block_header.size);
    if (!best) {
      best = next;
    } else if (nsz == total_bytes) {
      // Early return for perfect fit
      return (struct mem_block *)next;
    } else if (nsz < real_size(best->block_header.size)
    			&& nsz > total_bytes) {
	  best = next;
    }
  	next = (struct free_block *)next->next_free;
  }
  return (struct mem_block *)best;
}

struct mem_block
*worst_fit(uint total_bytes)
{
  struct free_block *next = flh;
  
  struct free_block *worst = NULL;

  while (next) {
    uint nsz = real_size(next->block_header.size);
    if (!worst) {
      worst = next;
    } else if (nsz > real_size(worst->block_header.size)
        		&& nsz >= total_bytes) {
	  worst = next;
    }
  	next = (struct free_block *)next->next_free;
  }
  return (struct mem_block *)worst;
}

void
malloc_setfsm(int fsm) {
  switch (fsm) {
  	case FSM_FIRST_FIT:
  	  fsm_algorithm = &first_fit;
  	  break;
  	case FSM_BEST_FIT:
  	  fsm_algorithm = &best_fit;
  	  break;
  	case FSM_WORST_FIT:
  	  fsm_algorithm = &worst_fit;
  	  break;
  }
}

void
free(void *ap)
{ 
  struct mem_block *block = ((struct mem_block *)ap) - 1;	// Get header
  block->size = block->size | 1;							// Set the free flag
  
  // Merge blocks if needed
  struct free_block *to_add = merge_blocks(block);

  if (to_add) {
    remove_free((struct free_block *)to_add);
    add_free(to_add); 	
  }
}

void
*malloc(uint nbytes)
{
  struct mem_block *block = NULL;
  int req_bytes = align(nbytes + sizeof(struct mem_block), 16);

  if (fsm_algorithm == NULL) {
  	fsm_algorithm = &first_fit;
  }

  block = fsm_algorithm(req_bytes);

  if (!block) {
    uint total_bytes = align(req_bytes, PAGE_SIZE);
    block = (struct mem_block *)sbrk(total_bytes);
    block->size = total_bytes;
  }
  block->size = real_size(block->size);
  block = split_block(block, req_bytes);

  add_block(block);

  if (scribble) {
  	block_scribble(block);
  }
  
  return block + 1;	
}

/**
 * Zeroes out the memory
 * To figure out the size, nmemb * size (Eg. 30 * some struct)
 */
void
*calloc(uint nmemb, uint size)
{
  struct mem_block *block = malloc(nmemb * size);
  memset(block, 0, nmemb * size);
  return block;
}

/**
 * Resize an allocation
 */
void
*realloc(void *ptr, uint size)
{
  if (!ptr) {
  	return malloc(size);
  }

  if (!size) {
  	free(ptr);
  	return NULL;											// Since it's freed
  }

  struct mem_block *block = (struct mem_block *)ptr - 1;	// Skip past header
  int req_bytes = align(size + sizeof(struct mem_block), 16);

  if (real_size(block->size) < req_bytes) {
    // Too small, attempt to merge blocks
    block->size = block->size | 1;
    block = (struct mem_block *)merge_blocks(block); 
  }

  if (real_size(block->size) < req_bytes) {
  	// Still too small, allocate another address
  	free(block + 1);
    block = (struct mem_block *)malloc(size) - 1;
  }  

  block->size = real_size(block->size);
  split_block(block, req_bytes);

  if (scribble) {
  	block_scribble(block);
  }

  return block + 1;
}


/* Copyright (C) 2008-2009 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* Encoding:   US_ASCII    Tab Size:   8   Indentation:    4  */

/* fs_bestfit.h */

#ifndef FS_BESTFIT_H
#define FS_BESTFIT_H

#include "fs_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

/* set to 1, to include leading and trailing dead space in each block */
/* makes the heap more robust at a cost of 8 bytes per allocation */
#define USE_DEAD_SPACE 0

/******************************************************************************
 * a size segregated (ie: multiple free list) dynamic memory sub-allocator
 * uses boundary tags for search-less combining of free blocks
 * uses bitarrays for seach-less finding of best-fit small blocks
 * larger blocks are grouped by size and selected using FIFO first fit
 *
 * swp 7/21/08
 *
 ******************************************************************************
 * the boundary tags occupy the first and last words of a block
 * 
 * free block
 *   uint size
 *   block *fwd
 *   block *bak  -- these links connect the FREE blocks
 *   --unused--
 *   uint size
 *
 * if USE_DEAD_SPACE
 * used block
 *   uint size|1
 *   uint DEAD
 *   --user data--
 *   uint DEAD
 *   uint size|1
 *
 * otherwise
 * used block
 *   uint size|1
 *   --user data--
 *   uint size|1
 *
 * NOTE: since the size is the number of 32 bit words in the block, we can access the
 * leading and trailing (size|flag) entries around the current block with the following:
 *    uint *p = (uint *)blk;
 *    size = p[0] & ~BLOCK_IN_USE;
 *    in_use = p[0] & BLOCK_IN_USE;
 *    prev_size = p[-1] & ~BLOCK_IN_USE;
 *    next_size = p[size] & ~BLOCK_IN_USE;
 *    prev_in_use = p[-1] & BLOCK_IN_USE;
 *    next_in_use = p[size] & BLOCK_IN_USE;
 *
 * So when freeing a block, all the data needed to merge the block with its free neighbors
 * is immediately (no searching) available.
 *
 * IMPORTANT: the first and last possible blocks of size==2 are *permanently* in use.
 * So we never need be paranoid about prev_in_use and next_in_use going off the ends
 * of the heap. This saves OODLES of testing, at the cost of 8 bytes of heap.
 *
 * NOTE: all the free lists are FIFO's. head->bak == tail, and tail->fwd == NULL
 * This enables us to quickly add to the end of a list.  FIFO free lists are most
 * often superior to LIFO lists with respect to fragmentation.
 *
 ******************************************************************************
 * set this between 5 and 10 please.
 * 10 gives best performance and has the most overhead.
 */
#define LG2_NUM_SMALL_LISTS 10

#define NUM_SMALL_LISTS (1<<LG2_NUM_SMALL_LISTS)
#define LARGE_SLOT_ADJUST (1+LG2_NUM_SMALL_LISTS)
#define NUM_LARGE_LISTS (31-LG2_NUM_SMALL_LISTS)

#define SMALL_SLOT(num) (((num) - OVERHEAD_WORDS - 2) >> 1)
#define LARGE_SLOT(num) MAX(0, 31 - MSB32((num) - OVERHEAD_WORDS - 2) - LARGE_SLOT_ADJUST)

/******************************************************************************
 * We maintain some constant block->size linked lists
 * slot k <= 16+k*8 bytes 
 *
 * We mave a bit array and a bit vector 
 * uint small_bits[NUM_SMALL_LISTS >> 5],small_bit_rows;
 *
 * think of small_bits[] as an Nx32 array of bits, so
 *    row = slot >> 5;
 *    col = slot & 31;
 *    small_bits[row,col] == 0 IFF small_lists[slot] == NULL
 *
 * think of small_bit_rows, as a 32 element bit vector, so
 *    small_bit_rows[k] == 0 IFF small_bits[row] == {0,,,,0}
 * 
 * these help us find free blocks quite quickly, for instance
 * to find the first non-empty index into small_lists[]
 *    row = MSB(small_bit_rows);
 *    mask = small_bits[row];
 *    col = MSB(mask);
 *    index = (row << 5) | col;
 *
 * MSB(x) is a table lookup which returns the Most Significant Bit in x
 *
 * We maintain some variable block->size linked lists, and a bitvector
 * uint large_bits;
 * large_bits[k]==0 IFF large_lists[k]==NULL;
 *
 * the smallest non-empty element of large_lists[] is MSB(large_bits);
 */

/*
 * all allocations are rounded up to 8N bytes with
 * size = (size + GRANULARITY) & ~GRANULARITY;
 */
#define GRANULARITY 7

#define BLOCK_IN_USE 1

#define USER_TO_BLOCK(x) ((x) ? (_block *)(((FS_BYTE *)(x)) - offsetof(_block, bak)) : NULL)
#define BLOCK_TO_USER(x) ((x) ? (void *)(((FS_BYTE *)(x)) + offsetof(_block, bak)) : NULL)

#define LP_TO_BLOCK(x) ((_block *)(x))
#define BLOCK_TO_LP(x) ((FS_ULONG *)(x))

#define USER_TO_LP(x) BLOCK_TO_LP(USER_TO_BLOCK(x))
#define LP_TO_USER(x) BLOCK_TO_USER(LP_TO_BLOCK(x))

#if USE_DEAD_SPACE
#define DEAD (0xDeadDead)
#define SET_DEAD(p,num) ((p)[1] = (p)[(num)-2] = DEAD)
#else
#define SET_DEAD(p,num)
#endif

#define OVERHEAD (offsetof(_block, bak) << 1)

#define OVERHEAD_WORDS (offsetof(_block, bak) >> 1)

#define MIN_BLOCK_SIZE_BYTES (sizeof(_block))
#define MIN_BLOCK_SIZE (MIN_BLOCK_SIZE_BYTES >> 2)

#define MAX_SLOT_INCREMENT (MIN_BLOCK_SIZE >> 1)

/*
 * We use a typedef so we can get to linked list pointers easily.
 *
 * Linked list blocks require, at a minimum, an FS_ULONG "num1",
 * two pointers ("bak" and "fwd"), and an FS_ULONG "num2". (DEAD is
 * never used nor matters in linked list blocks.)
 *
 * In-use blocks require, at a minimum, an FS_ULONG "num1", if using
 * DEAD space an FS_ULONG "dead1", enough user bytes to bring
 * granularity to 8, if using DEAD space an FS_ULONG "dead2", and an
 * FS_ULONG "num2".  In-use block will have to be >= linked list block.
 *
 * When pointers are 32 bits each:
 *  NOT using DEAD:
 *   linked list:  4 + 4 + 4 + 4 = 16 bytes
 *   in-use:       4 + userb + 4 = 16 bytes, userb = 8
 *  DEAD used:
 *   linked list:  4 + 4 + 4 + 4 = 16 bytes
 *   in-use:       4 + 4 + userb + 4 + 4 = 24 bytes, userb = 8
 *
 * When pointers are 64 bits each:
 *  NOT using DEAD:
 *   linked list:  4 + 4 (align) + 8 + 8 + 4 (align) + 4 = 32 bytes
 *   in-use:       4 + userb + 4 = 32 bytes, userb = 24 (or userb=16 w/8 waste)
 *  DEAD used:
 *   linked list:  4 + 4 (align) + 8 + 8 + 4 (align) + 4 = 32 bytes
 *   in-use:       4 + 4 (dead1) + userb + 4 (dead2) + 4 = 32 bytes, userb = 16
 *
 * When using 64-bit pointers, for alignment purposes, the two
 * FS_ULONG "num" values will be padded to 8 bytes each.  The "DEAD" will
 * not require more space as it will fill that padding.  So, on platforms
 * with 64-bit pointers, the minimum block size will be 4*8=32 bytes whether
 * DEAD space is being used or not.
 *
 * We use a (FS_ULONG *)p to get to the leading/trailing size
 * and DEAD elements easily.
 */
typedef struct _block
{
    FS_ULONG num;
#if USE_DEAD_SPACE
    FS_ULONG dead1;
#endif
    struct _block *bak;
    struct _block *fwd;
#if USE_DEAD_SPACE
    FS_ULONG dead2;
#endif
    FS_ULONG num2;

} _block;

typedef struct
{
    FS_ULONG *mem;
    FS_ULONG heapsize;
    FS_ULONG small_bits[NUM_SMALL_LISTS >> 5];
    FS_ULONG small_bit_rows;
    FS_ULONG large_bits;
    _block *small_lists[NUM_SMALL_LISTS];
    _block *large_lists[NUM_LARGE_LISTS];
    _block *last_split_block;
    _block *first_block;
    _block *last_block;
} _heap;

/*
 * most significant bit in a 32 bit word ... msb==0, lsb==31 ... all 0's gets a 32
 * some chips have a "find_first_one" instruction
 * some compilers support these instructions
 */
/* GCC supports built-ins on many chips .. x86, arm, etc. */
#if defined(__GCC__)
#define MSB32(x) (((x)==0) ? 32 : __builtin_clz(x))

/* ARM RealView Compiler versions 3.1 and up do too */
#elif defined(__arm__) && (__ARMCC_VERSION >= 310000)
#define MSB32(x) (((x)==0) ? 32 : __clz(x))

/* Windows CE 5.0 does as well */
#elif defined(_WINCE_WCE) && (_WINCE_CE > 501)
#define MSB32(x) (((x)==0) ? 32 : _CountLeadingZeros(x))

/* here's one for Win32 on Pentium Pro and better
 * on plain pentiums, 486's and 386's the assembly
 * is  S L O W E R  than the generic implementation
 */
#elif defined(_MSC_VER) && defined(_M_IX86) && _M_IX86 >= 600
extern int __fastcall clz(unsigned int x);
#define MSB32(x) (((x)==0) ? 32 : clz(x))

/*
 * add #elif statements for other cpus/compilers here
 */

#else
#define MSB32_GENERIC_TABLE_LOOKUP
/* use a generic table lookup implementation */
#define MSB32(x) (((x)>>16) ? (((x)>>24) ? msb8[(x)>>24] : 8+msb8[(x)>>16]) : (((x)>>8) ? 16+msb8[(x)>>8] : 24+msb8[x]))

#endif /* MSB related */


#ifdef __cplusplus
    }
#endif

#endif /* FS_BESTFIT_H */

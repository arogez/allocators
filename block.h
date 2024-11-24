/* block.h -- Implementation of the block allocation strategy
 * 
 * MIT License
 * Copyright (c) 2024 arogez
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BLOCK_H
#define BLOCK_H

#include <limits.h>

#include "heap.h"

enum block_limits {
        BLOCK_HEAP_MAX = UCHAR_MAX
};

struct block_heap {
        size_t          block_size;
        uint8_t         nblocks;
        int             first_free_block;
        void            *data;
};

void block_heap_reset(void *ptr, size_t nbytes, uint8_t n)
{
        *(uint8_t*)ptr = (uint8_t)(BLOCK_HEAP_MAX - (--n));
        
        if (n > 0) 
                block_heap_reset(ptr + (uintptr_t)nbytes, nbytes, n);
}

int block_heap_init(struct block_heap *b, struct heap *h, size_t nbytes, size_t alignment)
{
        b->nblocks = BLOCK_HEAP_MAX;
        b->first_free_block = 0;
        b->block_size = nbytes; 
        b->data = heap_aligned_alloc(h, nbytes * BLOCK_HEAP_MAX, alignment);
 
        block_heap_reset(b->data, nbytes, BLOCK_HEAP_MAX);
}

void *block_alloc(struct block_heap *a)
{
        if (a->nblocks == BLOCK_HEAP_MAX)
                return NULL;

        void *ptr = (void *)((uintptr_t)a->data + (a->block_size * a->first_free_block));
        
        a->first_free_block = *(uint8_t *)ptr;
        a->nblocks--;

        return ptr;
}

int block_is_valid(void *ptr, void *head, int nblocks, size_t block_size) 
{
        void *tail = (void *)((uintptr_t)head + (BLOCK_HEAP_MAX * block_size));
        uintptr_t offset = (uintptr_t)ptr - (uintptr_t)head;
        
        return (ptr >= head && ptr < tail && ((offset % block_size) == 0)) ? 1 : 0;
}

void block_free(struct block_heap *al, void *ptr) 
{
        if (ptr == NULL)
                return;
       
        /* check if ptr address is in correct address range and multiple of block_size */
        if (block_is_valid(ptr, al->data, al->nblocks, al->block_size) == 0) {
                return;
        }
        
        uint8_t index = (uint8_t)((uintptr_t)(ptr - al->data) / al->block_size);
        
        *(uint8_t *)ptr = al->first_free_block;
        al->first_free_block = index;
        al->nblocks--;
}

void block_heap_term(struct block_heap *b, struct heap *h)
{
        if (b == NULL) 
                return;

        heap_aligned_free(h, b->data);
}

#endif

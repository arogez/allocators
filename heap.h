/* heap.h - memory allocation header */
/* Primitive functions to keep track of heap allocated memory */

#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bit.h"

enum heap_flag : unsigned {
        H_COUNT,
        H_CLEAR,
        H_DEBUG
};

/* Bitmasks */
#define HEAP_COUNT bit(H_COUNT) 
#define HEAP_CLEAR bit(H_CLEAR) 
#define HEAP_DEBUG bit(H_DEBUG) 

struct heap {
        enum heap_flag hft;
        unsigned alloc_count;
};

void heap_init(struct heap *h, const enum heap_flag hft)
{
        h->alloc_count = 0;
        h->hft = hft;
}

void *heap_alloc(struct heap *h, const size_t nbytes) 
{
        void *ptr;
        
        if (nbytes == 0) 
                return NULL;

        ptr = malloc(nbytes);
         
        if (!ptr) {
                if (h->hft & HEAP_DEBUG) 
                        printf("heap info: could not allocate requested size\n"); 
                return NULL;
        }

        if (h->hft & HEAP_COUNT) {
                h->alloc_count++;
        }
        if (h->hft & HEAP_CLEAR)
                memset(ptr, 0, nbytes);
        if (h->hft & HEAP_DEBUG)
                printf("heap_alloc @%p size(%zu)\n", ptr, nbytes); 
        
        return ptr;
}

void *heap_aligned_alloc(struct heap *h, const size_t nbytes, const size_t alignment)
{
        void *ptr_0, **ptr_1;
        
        /* check if alignment is a power of 2 */
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
                if(h->hft & HEAP_DEBUG) {
                        printf("heap_aligned info: alignment not a power of 2\n");
                }
                return NULL;
        }

        /* check if alignment is a factor of requested size */
        if (nbytes & (alignment - 1) != 0) {
                if(h->hft & HEAP_DEBUG) {
                        printf("heap_aligned info: requested size not a multiple of alignment\n");
                }
                return NULL;
        }
       
        const size_t offset = alignment - 1 + sizeof(void*);
        ptr_0 = heap_alloc(h, nbytes + offset); 
        
        if (ptr_0 == NULL)
                return NULL;

        ptr_1 = (void*)(((uintptr_t)ptr_0 + offset) & ~(alignment - 1)); 
        
        if (h->hft & HEAP_DEBUG) 
                printf("heap_aligned_alloc @%p\n", ptr_1);

        ptr_1[-1] = ptr_0;

        return ptr_1;
}

void heap_free(struct heap *h, void *ptr)
{
        if (ptr == NULL) return;
        
        if (h->hft & HEAP_COUNT) {
                assert(h->alloc_count != 0);
                h->alloc_count--;
        }

        if (h->hft & HEAP_DEBUG) printf("heap_free  @%p\n", ptr);

        free(ptr);
}

void heap_aligned_free(struct heap *h, void *ptr)
{
        heap_free(h, ((void**)ptr)[-1]);
}

void heap_term(struct heap *h) {
  
        if ((h->hft & HEAP_COUNT) && (h->hft & HEAP_DEBUG)) {
                if (h->alloc_count) 
                        printf("heap info: (%i) allocs not freed\n", h->alloc_count);
                else 
                        printf("heap info: all allocs freed\n");
        }
}

#endif

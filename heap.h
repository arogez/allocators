/* heap.h - memory allocation header */
/* Primitive functions to keep track of heap allocated memory */

#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bit.h"

enum heap_flag_t : unsigned {
        COUNT,
        CLEAR,
        DEBUG
};

/* Bitmasks */
#define HEAP_COUNT bit(COUNT) 
#define HEAP_CLEAR bit(CLEAR) 
#define HEAP_DEBUG bit(DEBUG) 

struct heap {
        enum heap_flag_t hft;
        unsigned alloc_count;
};

void heap_init(struct heap *h, const enum heap_flag_t hft)
{
        h->alloc_count = 0;
        h->hft = hft;
}

void *heap_alloc(struct heap *h, const size_t nbytes) 
{
        void *ptr;
        
        if (nbytes == 0) return NULL;

        ptr = malloc(nbytes);

        if (ptr) {
                if (h->hft & HEAP_COUNT) h->alloc_count++;
                if (h->hft & HEAP_DEBUG) printf("heap_alloc @x%lx size(%zu)\n", ptr, nbytes); 
                if (h->hft & HEAP_CLEAR) memset(ptr, 0, nbytes);
        }
        
        return ptr;
}

void heap_free(struct heap *h, void *ptr)
{
        if (ptr == NULL) return;
        
        if (h->hft & HEAP_COUNT) {
                assert(h->alloc_count != 0);
                h->alloc_count--;
        }

        if (h->hft & HEAP_DEBUG) printf("heap_free  @x%lx\n", ptr);

        free(ptr);
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

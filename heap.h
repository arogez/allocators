/* heap.h - memory allocation header */
/* primitive functions to keep track of heap allocated memory */

#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* TODO : implementation of calloc and realloc functions */

struct heap {
        unsigned alloc_count;
};

void heap_init(struct heap *h)
{
        h->alloc_count = 0;
}

void *heap_alloc(struct heap *h, const unsigned nbytes) 
{
        void *ptr;
        
        if (nbytes == 0) return NULL;

        ptr = malloc(nbytes);
        h->alloc_count++;

        if (ptr != NULL) {
                printf("heap_alloc @x%lx: size(%zu), n_alloc(%i)\n", ptr, nbytes, h->alloc_count); 
        }

        return ptr;
}

void heap_free(struct heap *h, void *ptr)
{
        if (ptr == NULL) return;

        printf("heap_free  @x%lx: n_alloc(%i)\n", ptr, h->alloc_count); 
        
        assert(h->alloc_count != 0);
        h->alloc_count--;

        free(ptr);

        if (h->alloc_count == 0) {
                printf("heap info: all heap allocated memory freed\n");
        }
}

#endif

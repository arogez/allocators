#ifndef SCRATCH_ALLOCATOR_H
#define SCRATCH_ALLOCATOR_H

#include "heap.h"

struct scratch_heap {
        void *head;
        void *tail;
        void *mem;
};

void scratch_heap_init(struct scratch_heap *scr, struct heap *h, size_t nbytes, size_t alignment) 
{
        if (nbytes == 0)
                return;

        scr->mem = heap_aligned_alloc(h, nbytes, alignment);
        
        if (scr->mem == NULL)
                return;

        scr->head = scr->mem;
        scr->tail = (void *)((uintptr_t)(scr->head + nbytes));
}

void *scratch_alloc(struct scratch_heap *scr, size_t nbytes, size_t alignment) 
{
        void *ptr_0, *ptr_1;

        if (alignment == 0 || (alignment & (alignment - 1)) != 0)
                return NULL;

        const size_t alloc = nbytes + alignment - 1;

        if ((void*)((uintptr_t)scr->head + alloc) > scr->tail){ 
                printf("no allocation\n");
                return NULL;
        }
        
        printf("test scr : %p\n", scr->head);
        
        ptr_0 = scr->head;
        ptr_1 = (void*)(((uintptr_t)ptr_0 + (alignment - 1)) & ~(alignment - 1));

        scr->head = (void *)((uintptr_t)scr->head + alloc);
        return ptr_1;
}

void scratch_heap_term(struct scratch_heap *scr, struct heap *h) 
{
        if (scr == NULL) 
                return;

        heap_aligned_free(h, scr->mem);
}

#endif

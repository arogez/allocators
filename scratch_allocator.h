/* scratch_allocator.h -- Implementation of the scratch allocation strategy
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
        
        const size_t alloc = ((uintptr_t)scr->head % alignment == 0) ? nbytes : (nbytes + alignment - 1);

        if ((void*)((uintptr_t)scr->head + alloc) > scr->tail) 
                return NULL;
        
        ptr_0 = scr->head;
        scr->head = (void *)((uintptr_t)scr->head + alloc);

        if (alloc > nbytes) {
                ptr_0 = (void*)(((uintptr_t)ptr_0 + (alignment - 1)) & ~(alignment - 1));
        }
        
        return ptr_0;
}

void scratch_heap_term(struct scratch_heap *scr, struct heap *h) 
{
        if (scr == NULL) 
                return;

        heap_aligned_free(h, scr->mem);
}

void scratch_heap_reset(struct scratch_heap *scr)
{
        if (scr == NULL)
                return;

        scr->head = scr->mem;
}

#endif

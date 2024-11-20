/* buddy.h - implementation of the buddy allocation strategy */

#ifndef BUDDY_H
#define BUDDY_H

#include <limits.h>
#include <stdint.h>

#include "heap.h"
#include "bit.h"
#include "list.h"

enum buddy_range : uint8_t {

        BUDDY_MAX_RANGE = 28,
        /* minimum size of 1 block: 128 bytes */
        BUDDY_MIN_RANGE = 8
};

struct buddy_heap {
        size_t heap_size;
        uint8_t exp;
        struct list *chunks[1];
        void* meta;
        void* data;
};

uint32_t buddy_heap_init(struct buddy_heap *h, const unsigned exp)
{
        /* evaluate requested size against heap limits */
        if (!(exp > BUDDY_MIN_RANGE && exp <= BUDDY_MAX_RANGE)) return -1;
        
        /* compute size of allocation: metadata + allocation */
        const unsigned bits_count = bit(exp - BUDDY_MIN_RANGE) + 1; 
        h->heap_size = (bits_count / (CHAR_BIT)) + bit(exp); 

        h->exp = exp;
        return 0;
}

#endif

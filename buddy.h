/* buddy.h -- Implementation of the buddy allocation strategy
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


#ifndef BUDDY_H
#define BUDDY_H

#include <limits.h>
#include <stdint.h>

#include "heap.h"
#include "bit.h"
#include "list.h"
#include "pow.h"

#define META_ALIGNMENT 32

enum buddy_order : uint8_t {
        BUDDY_MAX_K = 28,
        BUDDY_MIN_K = 6
};

/* Description of the approach ("buddy system reservation & liberation") can be found in 
 * Knuth's The Art of Computer Programming.
 *
 * Limits of the system: K_MAX_ORDER (1 << K), K_MIN_ORDER (1 << K_MIN_ORDER) 
 * Design of the system: 
 *      At init (buddy_heap_init()), allocation of 2 heap allocated areas: meta(data) & data.
 *      Metadata is an array of bits for tracking the availability of each buddy in a pair of 
 *      identical size ranging from 2^K_MAX_ORDER TO 2^K_MIN_ORDER.
 *      1 bit overhead per two blocks of same size and buddies.
 *      Bit are set and unset when allocated and deallocated using helper macros found in bit.h.
 *      The value of the bit controls reservation and coalescion at deallocation.
 *
 *      +--------------+-------+--------------------------------------------------+
 *      |  function    |  bit  |  result                                          | 
 *      +--------------+-------+--------------------------------------------------+
 *      |  buddy_alloc |   0   |  split buddies. return location. switch bit.     |  
 *      |              |   1   |  return location. switch bit.                    |
 *      +--------------+-------+--------------------------------------------------+
 *      |  buddy_free  |   0   |  buddy is allocated. reserve adress. switch bit. |
 *      |              |   1   |  buddy is reserved. coalesce. switch bit.        |
 *      +--------------+-------+--------------------------------------------------+           
 *
 *      We use a prefix on each allocated block of memory return by the buddy allocator.
 *      The size of the prefix depends on the alignment provided at initialization.
 *      The prefix store the order(k) of the allocated block and the address of the
 *      block to be used at deallocation. 
 *           
 *      ** Block of size 2^k
 *      +----------------+--------------------------------------------------------+
 *      |  Prefix        | Aligned memory block                                   | 
 *      +----------------+--------------------------------------------------------+
 *                       |
 *                       `-> address return to user
 *
 */

struct buddy_block_prefix {
        enum buddy_order        k;
        void                    *ptr;
};

struct buddy_heap {
        struct heap             *h;
        enum buddy_order        k;
        size_t                  alignment; 
        struct list             nodes[BUDDY_MAX_K];
        void                    *bits;
        void                    *data;
};

int buddy_heap_init(struct buddy_heap *b, struct heap *h, uint8_t k, size_t alignment)
{
        if (!(k > BUDDY_MIN_K && k <= BUDDY_MAX_K)) return -1;
        
        /* check if alignment is a power of 2 */
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
                if(h->hft & HEAP_DEBUG) {
                        printf("buddy_heap info: alignment not a power of 2\n");
                }

                return -1;
        }
         
        const int nbits = bit(k - BUDDY_MIN_K);
        size_t meta_sz = (size_t)(((nbits + CHAR_BIT - 1) & -CHAR_BIT) / CHAR_BIT);
       
        b->h = h;
        b->k = k;
        b->alignment = alignment;
        b->bits = heap_aligned_alloc(h, meta_sz, META_ALIGNMENT);

        if (b->bits == NULL) 
                return -1;

        b->data = heap_aligned_alloc(h, bit(k), alignment);
        
        if (b->data == NULL)
                return -1;
        
        list_push(&b->nodes[0].head, b->data);

        return 0;
}

void buddy_heap_term(struct buddy_heap *b, struct heap *h)
{
        if (b == NULL) 
                return;

        heap_aligned_free(h, b->data);
        heap_aligned_free(h, b->bits);
}

uint8_t buddy_nbytes_query_to_index(const size_t nbytes, const enum buddy_order k)
{
        const size_t ceil = ((nbytes & (nbytes - 1)) != 0) ? pow2_roundup(nbytes) : nbytes; 
        return (k - trailing_zeros_count(ceil));  
}

int8_t buddy_first_splittable_node_index(const uint8_t req, struct list *nodes)
{
        for (int i = 1; i <= req; i++) {
                if (nodes[req - i].head != NULL) {
                        return (req - i);
                }
        }
        
        return -1; 
}

void buddy_node_update(uint8_t k, uint8_t max_k, struct list *nodes, void* ptr, int split)
{
        list_pop(&nodes[k].head);

        if (split) {
                const int offset = (1 << (max_k - k)) >> 1;
                list_push(&nodes[k + 1].head, ptr);
                list_push(&nodes[k + 1].head, (char *)ptr + offset);
        }
}

int buddy_block_index(uint8_t k, uint8_t max_k, int offset)
{
        double mult = 1.f / (1 << (max_k - k));
        int index = (mult * offset) + ((1 << k) - 1);

        return index; 
}

unsigned buddy_bit_position(uint8_t k, uint8_t max_k, const int offset) 
{
        int block_index = buddy_block_index(k, max_k, offset); 
        unsigned bit_index = (block_index / 2 + block_index % 2);
        
        return bit_index; 
}

void buddy_bit_update(uint8_t k, uint8_t max_k, uint32_t *bitset, void *data, void *ptr)
{
        const int offset = (char *)ptr - (char *)data;
        const unsigned bit = buddy_bit_position(k, max_k, offset);

        bit_switch(bitset, bit);
}

void buddy_update(struct buddy_heap *b, int index, void *ptr, int split) 
{
        if (ptr == NULL) {
                //log error here
                return;
        }

        buddy_node_update(index, b->k, b->nodes, ptr, split);
        buddy_bit_update(index, b->k, b->bits, b->data, ptr);
}

void *buddy_block_split(struct buddy_heap *b, int index, int split)
{
        void *ptr;
        const int8_t node_index = buddy_first_splittable_node_index(index, b->nodes);

        if (node_index == -1) {
                //log error here
                return NULL;
        }

        for (int i = node_index; i < index; i++) {
                ptr = (void *)b->nodes[i].head;
                buddy_update(b, i, ptr, split);
        }
}

void *buddy_alloc(struct buddy_heap *b, size_t nbytes) 
{
        void *ptr_0, **ptr_1;

        if (nbytes == 0)
                return NULL;

        const size_t offset = (b->alignment - 1) + sizeof(struct buddy_block_prefix);
        const uint8_t index = buddy_nbytes_query_to_index(nbytes + offset, b->k);

        int split = (b->nodes[index].head == NULL);

        if (split) {
                buddy_block_split(b, index, split); 
                split = 0; 
        }

        ptr_0 = b->nodes[index].head;

        if (ptr_0 == NULL) {
                printf("NO BUENO_0\n");
        }

        ptr_1 = (void *)(((uintptr_t)ptr_0 + offset) & ~(b->alignment - 1));

        struct buddy_block_prefix *m = (void *)ptr_1;
        m[-1].k = index;
        m[-1].ptr = ptr_0;

        buddy_update(b, index, ptr_0, split);

        return ptr_1; 
}

void buddy_free(struct buddy_heap *b, void *ptr) 
{
        if(ptr == NULL)
                return;
        
        void *ptr_0, *ptr_1;
        struct buddy_block_prefix *m = ptr;
        
        uint8_t k = m[-1].k;
        ptr_0 = m[-1].ptr;

        const int offset = (uintptr_t)ptr_0 - (uintptr_t)b->data; 
        const unsigned bit_index = buddy_bit_position(k, b->k, offset);
        
        uint32_t *bitset = b->bits;

        if (bit_check(bitset, bit_index) && bit_index != 0) {

                const int buddy_offset = offset ^ (1 << b->k - k);
                void *buddy = (void *)((uintptr_t)(b->data + buddy_offset));
                
                if (list_delete(&b->nodes[k].head, buddy) == 0) {
                        //log error here
                }

                if (offset > buddy_offset) {
                        ptr_0 = buddy;
                } 
                
                const size_t align_offset = b->alignment - 1 + sizeof(struct buddy_block_prefix);
                ptr_1 = (void *)(((uintptr_t)ptr_0 + align_offset) & ~(b->alignment - 1));
                
                m = (void *)ptr_1;

                m[-1].k = k - 1;
                m[-1].ptr = ptr_0;

                buddy_free(b, ptr_1);

                bit_switch(bitset, bit_index);
        }
        
        if (b->h->hft & HEAP_DEBUG) {
                if (k == 0) printf("buddy_heap info: all memory blocks coalesced\n"); 
        }

        list_push(&b->nodes[k].head, ptr_0); 
        bit_switch(bitset, bit_index);
}


#endif

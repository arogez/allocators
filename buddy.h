/* buddy.h - implementation of the buddy allocation strategy */

#ifndef BUDDY_H
#define BUDDY_H

#include <limits.h>
#include <stdint.h>

#include "heap.h"
#include "bit.h"
#include "list.h"
#include "pow.h"

enum buddy_order : uint8_t {

        K_MAX_ORDER = 28,
        K_MIN_ORDER = 6
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
 *      +----------------+-------------------------------------------------------+
 *      |  Prefix        | Aligned memory block                                  | 
 *      +----------------+-------------------------------------------------------+
 *                       |
 *                       `-> address return to user
 *
 */

struct buddy_block_prefix {
        uint8_t                 k;
        void*                   address;
};

struct buddy_heap {
        enum buddy_order        order;
        size_t                  alignment; 
        struct list             nodes[K_MAX_ORDER];
        void                    *meta;
        void                    *data;
};

uint32_t buddy_heap_init(struct buddy_heap *b, 
                         struct heap *h, 
                         const uint8_t order, 
                         const size_t alignment)
{
        if (!(order > K_MIN_ORDER && order <= K_MAX_ORDER)) return -1;
        
        const unsigned bits_count = bit(order - K_MIN_ORDER);
        size_t meta_sz = (size_t)(((bits_count + CHAR_BIT - 1) & -CHAR_BIT) / CHAR_BIT);
        
        b->order = order;

        /* todo : check if alignment is power of 2 here */
        b->alignment = alignment;

        b->meta = heap_aligned_alloc(h, meta_sz, 32);

        if (!b->meta) 
                return -1;

        b->data = heap_aligned_alloc(h, bit(order), alignment);
        
        if (!b->data)
                return -1;
        
        list_push(&b->nodes[0].head, b->data);

        if (h->hft & HEAP_DEBUG) {
                printf("buddy heap allocation:\n");
                printf("_____________________\n");
                printf("max order: %i\n", b->order);
                printf("meta pool adress: %p\n", b->meta);
                printf("data pool adress: %p\n", b->data);
                printf("_____________________\n");
        }

        return 0;
}

void buddy_heap_term(struct buddy_heap *b, struct heap *h)
{
        if (!b) return;

        heap_aligned_free(h, b->data);
        heap_aligned_free(h, b->meta);
}

const uint8_t buddy_nbytes_query_to_index(const size_t nbytes, const enum buddy_order order)
{
        const size_t ceil = ((nbytes & (nbytes - 1)) != 0) ? pow2_ceil(nbytes) : nbytes; 
        return (order - lowest_bit_index(ceil));  
}

const int8_t buddy_first_splittable_node_index(const uint8_t req, struct list *nodes)
{
        for (int i = 1; i <= req; i++) {
                if (nodes[req - i].head != NULL) {
                        return (req - i);
                }
        }
        
        return -1; 
}

void buddy_node_update(const uint8_t order, 
                       const uint8_t max_order, 
                       struct list *nodes,
                       void* ptr, 
                       const uint32_t split)
{
        list_pop(&nodes[order].head);

        if (split) {
                const uint32_t offset = (1 << (max_order - order)) >> 1;
                list_push(&nodes[order + 1].head, ptr);
                list_push(&nodes[order + 1].head, (char *)ptr + offset);
        }
}

const uint32_t buddy_block_index(const uint8_t order, const uint8_t max_order, const uint32_t offset)
{
        const double mult = 1.f / (1 << (max_order - order));
        const uint32_t index = (mult * offset) + ((1 << order) - 1);

        return index; 
}

const uint32_t buddy_bit_position(const uint8_t order, const uint8_t max_order, const uint32_t offset) 
{
        const uint32_t block_index = buddy_block_index(order, max_order, offset); 
        const uint32_t bit_index = (block_index / 2 + block_index % 2);
        
        return bit_index; 
}

void buddy_bit_update(const uint8_t order, 
                      const uint32_t max_order, 
                      uint32_t *bitset, 
                      void *data, 
                      void *ptr)
{
        const uint32_t offset = (char *)ptr - (char *)data;
        const uint32_t bit = buddy_bit_position(order, max_order, offset);

        bit_switch(bitset, bit);
}

void buddy_update(struct buddy_heap *b, uint32_t index, void *ptr, uint32_t split) 
{
        if (ptr == NULL) {
                //log error here
                return;
        }

        buddy_node_update(index, b->order, b->nodes, ptr, split);
        buddy_bit_update(index, b->order, b->meta, b->data, ptr);
}

void *buddy_block_split(struct buddy_heap *b, uint32_t index, uint32_t split)
{
        void *ptr;
        const int8_t node_index = buddy_first_splittable_node_index(index, b->nodes);

        if (node_index == -1) {
                //log error here
                return NULL;
        }

        for (int i = 0; i < index; i++) {
                ptr = (void *)b->nodes[i].head;
                buddy_update(b, i, ptr, split);
        }
}

void *buddy_alloc(struct buddy_heap *b, const size_t nbytes) 
{
        void *ptr_0, **ptr_1;
         
        if (nbytes == 0)
                return NULL;

        const size_t align_offset = (b->alignment - 1) + sizeof(struct buddy_block_prefix);
        const uint8_t index = buddy_nbytes_query_to_index(nbytes + align_offset, b->order);
        
        uint32_t split = (b->nodes[index].head == NULL);
        
        if (split) {
                buddy_block_split(b, index, split); 
                split = 0; 
        }
        
        ptr_0 = b->nodes[index].head;
        ptr_1 = (void *)(((uintptr_t)ptr_0 + align_offset) & ~(b->alignment - 1));
        
        struct buddy_block_prefix *m = (void *)ptr_1;
        m[-1].k = index;
        m[-1].address = ptr_0;
        
        buddy_update(b, index, ptr_0, split);

        return ptr_1; 
}

void buddy_free(struct buddy_heap *b, void *ptr) 
{
        if(ptr == NULL)
                return;
        
        void *ptr_0, *ptr_1;
        struct buddy_block_prefix *m = ptr;
        
        uint8_t order = m[-1].k;
        ptr_0 = m[-1].address;

        const uint32_t offset = (uintptr_t)ptr_0 - (uintptr_t)b->data; 
        const uint32_t bit_index = buddy_bit_position(order, b->order, offset);
        
        uint32_t *bitset = b->meta;

        if (bit_check(bitset, bit_index) && bit_index != 0) {

                const uint32_t buddy_offset = offset ^ (1 << b->order - order);
                void *buddy = (void *)((uintptr_t)(b->data + buddy_offset));
                
                if (list_delete(&b->nodes[order].head, buddy) == 0) {
                        //log error here
                }

                if (offset > buddy_offset) {
                        ptr_0 = buddy;
                } 
                
                const size_t align_offset = b->alignment - 1 + sizeof(struct buddy_block_prefix);
                ptr_1 = (void *)(((uintptr_t)ptr_0 + align_offset) & ~(b->alignment - 1));
                
                m = (void *)ptr_1;

                m[-1].k = order - 1;
                m[-1].address = ptr_0;

                buddy_free(b, ptr_1);

                bit_switch(bitset, bit_index);
        }
        
        list_push(&b->nodes[order].head, ptr_0); 
        bit_switch(bitset, bit_index);
}


#endif

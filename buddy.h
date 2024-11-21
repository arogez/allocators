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

        BUDDY_MAX_ORDER = 28,
        /* minimum size of 1 block: 128 bytes */
        BUDDY_MIN_ORDER = 6
};

struct buddy_heap {
        enum buddy_order        order;
        size_t                  alignment; 
        struct list             nodes[BUDDY_MAX_ORDER];
        void                    *meta;
        void                    *data;
};

uint32_t buddy_heap_init(struct buddy_heap *b, 
                         struct heap *h, 
                         const uint8_t order, 
                         const size_t alignment)
{
        if (!(order > BUDDY_MIN_ORDER && order <= BUDDY_MAX_ORDER)) return -1;
        
        const unsigned bits_count = bit(order - BUDDY_MIN_ORDER);
        size_t meta_sz = (size_t)(((bits_count + CHAR_BIT - 1) & -CHAR_BIT) / CHAR_BIT);
        
        b->order = order;
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
        if (!b)
                return;

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
        for (unsigned i = 1; i <= req; i++) {
                if (nodes[req - i].head != NULL) 
                        return (req - i);
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

void *buddy_alloc(struct buddy_heap *b, const size_t nbytes) 
{
        void *ptr;
        
        if (nbytes == 0)
                return NULL;

        //const size_t offset = b->alignment - 1 + sizeof(uint8_t) + sizeof(void *);
        
        const uint8_t index = buddy_nbytes_query_to_index(nbytes + offset, b->order);
        uint32_t split = (b->nodes[index].head == NULL);
        
        if (split) {
                const int8_t node_index = buddy_first_splittable_node_index(index, b->nodes);      
                printf("node index = %i\n", node_index); 
                
                if (node_index == -1) {
                        //log error here
                        return NULL;
                }

                for (int i = node_index; i < index; i++) {
                        ptr = (void *)b->nodes[i].head;
                        buddy_node_update(i, b->order, b->nodes, ptr, split);
                        buddy_bit_update(i, b->order, b->meta, b->data, ptr);
                }
                
                split = 0; 
        }

        ptr = b->nodes[index].head;

        buddy_node_update(index, b->order, b->nodes, ptr, split);
        buddy_bit_update(index, b->order, b->meta, b->data, ptr);

        return ptr;
}

#endif

/* trait_allocator.h -- custom allocation strategy
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

#ifndef TRAI_ALLOCATOR_H
#define TRAIT_ALLOCATOR_H

#include "block_allocator.h"
#include "list.h"

typedef void ctor();

enum trait_cache_flags : unsigned {
        TRAIT_HWCACHE_ALIGN,
        TRAIT_NOCOLLECT,
        TRAIT_BUDDYALLOC,
        TRAIT_PACKED
} tmem_cache_flags;

#define TMEM_CACHE_COMMON_FLAGS (TRAIT_HWCACHE_ALIGN | TRAIT_NOCOLLECT | TRAIT_BUDDYALLOC) 

struct t list tmem_caches;

struct tmem_cache {
        
        struct list     traits;
        const char      *name;
        size_t          trait_size;
        //int           is_packed;
        //size_t        color;
        //int           color_offset;
};

void *trait_cache_new(const char *name, size_t nbytes, void (*ctor)(), void *allocator) 
{
        struct tmem_cache *cache;
        
        if (allocator != NULL) {
                cache = block_alloc(allocator);
        }
        
        cache->name = name; 
        cache->trait_size = nbytes;

        return cache;
}

void trait_cache_delete() 
{

}

#endif

# C Allocators

A collection of header files implementing different C allocation strategies and helper functions.

> [!WARNING]
> This library is a work in progress.

The creation of these memory allocators is both of a didactic nature and is intented for 
other personal projects.

### List of headers with functions to allocate memory: 
- heap.h: allocates and tracks chunks of memory allocated with malloc
- buddy.h: partitions memory in power-of-2 sized blocks, merges blocks on deallocation
- block.h: partitions memory in fixed-size blocks, returns blocks to pool on deallocation

> [!NOTE]
> [IN PROGRESS] future additions: scratch allocator, stack allocator, slab allocator,

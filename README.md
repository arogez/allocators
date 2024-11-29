# c_allocators

A collection of header files implementing different C allocation strategies and helper functions.

> [!WARNING]
> This library is in beta development.

The creation of these memory allocators is both of a didactic nature and is intented for 
use in other personal projects.

### List of headers with functions to allocate memory: 
- heap.h: allocates and tracks chunks of memory allocated with malloc
- buddy.h: partitions memory in power-of-2 sized blocks, merges blocks on deallocation
- block.h: partitions memory in 255 fixed-size blocks, returns blocks to pool on deallocation
- scratch.h: stacks consecutive allocations of user-defined sizes. no deallocation.

> [!NOTE]
> [IN PROGRESS] future additions: slab allocator, stack allocator

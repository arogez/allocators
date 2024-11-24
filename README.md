# C Allocators
==============
A collection of header files implementing different C allocation strategies and helper functions.

> [!WARNING]
> This library is a Work in Progress.

The creation of these memory allocators is both of a didactic nature and is intented for 
other personal projects.

##List of headers with functions to allocate memory: 
- heap.h: the system allocates a big chunk of memory with malloc. Chunk can be aligned
- buddy.h: the system partitions memory in power-of-2 sized blocks, merges blocks on deallocation
- block.h: the system partitions fixed-size blocks, returns blocks to pool on deallocation

> [!NOTE]
> [IN PROGRESS] future additions: scratch allocator, stack allocator

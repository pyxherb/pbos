# PbOS Universal Kernel Layer

Here is the universal kernel layer of the PbOS.

## Issues

### mm_pgalloc reference count

`mm_pgalloc` references the page once, the `mm_mmap` also does, so we have to call `mm_pgfree` after the `mm_mmap` call.

# PbOS Universal Kernel Layer

Here is the universal kernel layer of the PbOS.

## Issues

### mm_pgalloc reference count

`mm_pgalloc` references the page once, the `mm_mmap` also does, so we have to call `mm_pgfree` after the `mm_mmap` call.

## mm_seal

`mm_seal` permanently seals mapping status of a memory region, which makes read-only memory regions can be safely compressed after the region is sealed.

# PbOS Universal Kernel Layer

Here is the universal kernel layer of the PbOS.

## Issues

### mm_alloc_single_page reference count

`mm_alloc_single_page` references the page once, the `mm_mmap` also does, so we have to call `mm_unref_page` after the `mm_mmap` call.

## mm_seal

`mm_seal` permanently seals mapping status of a memory region, which makes read-only memory regions can be safely compressed after the region is sealed.

## Glob Pattern Support

Add glob pattern facilities to support `fs_enum_first_child_file` and `fs_enum_next_file`, etc.

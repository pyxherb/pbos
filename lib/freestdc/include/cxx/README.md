# About C++ Standard Library Part of FreeSTDC

FreeSTDC provides a minimal version of C++ standard library for freestanding environments.

Note that container and atomic operation libraries are not provided.

## Current Development Status

Following components still need to be completed:

* Adapators of iterators
* Ranges library (Including range conversions in C++23, range factories, range adaptors...)

## Special Thanks

Thanks to [cppreference.com](https://cppreference.com), they provided a lot of example implementations of the standard library, some parts of this library is even from this website (it is also stronly recommended to be used as a reference when you are implementing a C++ compiler or having any doubt about the standard).

The C++ standard library part of FreeSTDC is licensed under CC-BY-SA 3.0 unported (we previously put LGPL v3.0 wrongly, now we fixed it).

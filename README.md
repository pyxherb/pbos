# PbOS Project

An open-source experimental operating system.

PbOS stands for Paging-based Operating System.

## Building

### Windows

To build PbOS on Windows, you will need:

* Clang or other compatible compilers that is compatible with C99
* Clang++ or other compatible compilers that is compatible with C++17
* libc++

Note that you have to set paths to libc++'s include directories manually.

Then, run Windows Powershell with Administrator Privileges and run ./configure to configure the project.

The typical command to build the project after the project is configured:

```powershell
cmake -S . -B build
```

Note that the default build configuration is `Debug`, you may change it by your want.

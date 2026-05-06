# PbOS Project

An open-source operating system for experiment purposes.

PbOS stands for Paging-based Operating System.

## Building

### Windows

To build PbOS on Windows, you will need:

* Clang or other compatible compilers that is compatible with C99
* Clang++ or other compatible compilers that is compatible with C++20

Then, run Windows Powershell with Administrator Privileges and run ./configure to configure the project.

The typical command to build the project after the project is configured:

```powershell
cmake --build build --config Debug --target pbkrnl
```

## License

PbOS is licensed under GNU General Public License v3.0.

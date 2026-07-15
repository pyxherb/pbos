# PbOS Project

<img src="./resources/common/pbos_text_logo.svg" />

An open-source experimental (currently) operating system.

PbOS stands for Paging-based Operating System.

## Building

To build PbOS, you should first set the `PBOS_ARCH` and `PBOS_TRIPLET` environment variables like following:

```sh
export PBOS_ARCH=x86_64
export PBOS_TRIPLET=x86_64-pc
```

**Note: DO NOT USE CMakePresets.json for initial configuration!**

### Windows

To build PbOS on Windows, you will need:

* Clang or other compatible compilers that is compatible with C99
* Clang++ or other compatible compilers that is compatible with C++20

Then, run Windows Powershell with Administrator Privileges and run ./configure to configure the project.

The typical command to build the project after the project is configured:

```powershell
cmake --build build --config Debug --target pboskrnl
```

## License

PbOS components in this repository is licensed under GNU General Public License v3.0 (may be changed later).

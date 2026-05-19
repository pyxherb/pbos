#
# Toolchain script for PbOS kernel components on AArch64.
#
# Copyright (C) 2026 PbOS Contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../")
set(CMAKE_SYSTEM_NAME PbOSKernel)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
set_property(GLOBAL PROPERTY CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(CMAKE_AR NAMES aarch64-elf-ar ar llvm-ar REQUIRED)
find_program(CMAKE_LINKER NAMES aarch64-elf-ld gold ld ld.lld REQUIRED)
find_program(CMAKE_OBJCOPY NAMES aarch64-elf-objcopy objcopy llvm-objcopy REQUIRED)
find_program(CMAKE_RANLIB NAMES aarch64-elf-ranlib ranlib llvm-ranlib REQUIRED)
find_program(CMAKE_SIZE NAMES aarch64-elf-size size llvm-size REQUIRED)
find_program(CMAKE_STRIP NAMES aarch64-elf-strip strip llvm-strip REQUIRED)

set(CMAKE_LINK_DEF_FILE_FLAG "")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".kx")
set(CMAKE_LINK_LIBRARY_SUFFIX ".a")
set(CMAKE_EXECUTABLE_SUFFIX "")
set(CMAKE_EXE_LINK_FLAGS "")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

if(CMAKE_HOST_WIN32)
	include(Platform/WindowsPaths)
endif()

#
# C setup
#
set(C_STANDARD 99)
find_program(CMAKE_C_COMPILER NAMES aarch64-elf-gcc gcc clang REQUIRED)
set(CMAKE_C_COMPILER_TARGET x86_64-elf)

set(CMAKE_C_FLAGS "-fvisibility=hidden -march=armv8-a -nostdlib -fno-stack-protector -fno-builtin -ffreestanding -fPIE")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELWITHDBGINFO "-O2 -g")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os")

set(CMAKE_C_LINK_FLAGS "-m elf_aarch64 -pie")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> <LINK_FLAGS> <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -shared")

#
# C++ setup
#
set(CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_program(CMAKE_CXX_COMPILER NAMES aarch64-elf-g++ g++ clang++ REQUIRED)
set(CMAKE_CXX_COMPILER_TARGET x86_64-elf)

set(CMAKE_CXX_FLAGS "-fvisibility=hidden -march=armv8-a -nostdlib -fno-stack-protector -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-builtin -ffreestanding -fPIE")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELWITHDBGINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")

set(CMAKE_CXX_LINK_FLAGS "-m elf_aarch64 -pie")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> <LINK_FLAGS> <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -shared")

#
# Assembly setup
#
find_program(CMAKE_ASM_COMPILER NAMES aarch64-elf-as as clang gcc REQUIRED)
set(CMAKE_ASM_COMPILER_TARGET x86_64-elf)

if (${CMAKE_ASM_COMPILER} STREQUAL CMAKE_C_COMPILER)
    set(CMAKE_ASM_FLAGS "-march=armv8-a -fno-builtin -ffreestanding -fPIE")
else()
    set(CMAKE_ASM_FLAGS "-march=aarch64")
endif()
set(CMAKE_ASM_FLAGS_DEBUG "-O0 -g")
set(CMAKE_ASM_FLAGS_RELWITHDBGINFO "-O2 -g")
set(CMAKE_ASM_FLAGS_RELEASE "-O2")
set(CMAKE_ASM_FLAGS_MINSIZEREL "-Os")

set(CMAKE_ASM_LINK_FLAGS "-m elf_aarch64 -pie")
set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <CMAKE_ASM_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> <LINK_FLAGS> <CMAKE_ASM_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -shared")

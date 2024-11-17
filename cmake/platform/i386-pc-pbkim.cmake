#
# Toolchain script for PbOS kernel components on i386.
#
# Copyright (C) 2022 PbOS Contributors
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
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i386)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
set_property(GLOBAL PROPERTY CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(CMAKE_AR NAMES ar llvm-ar REQUIRED)
find_program(CMAKE_LINKER NAMES gold ld ld.lld REQUIRED)
find_program(CMAKE_OBJCOPY NAMES objcopy llvm-objcopy REQUIRED)
find_program(CMAKE_RANLIB NAMES ranlib llvm-ranlib REQUIRED)
find_program(CMAKE_SIZE NAMES size llvm-size REQUIRED)
find_program(CMAKE_STRIP NAMES strip llvm-strip REQUIRED)

set(CMAKE_LINK_DEF_FILE_FLAG "")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_LINK_LIBRARY_SUFFIX ".a")
set(CMAKE_EXECUTABLE_SUFFIX "")
set(CMAKE_EXE_LINK_FLAGS "")

#
# C setup
#
set(C_STANDARD 11)
find_program(CMAKE_C_COMPILER NAMES gcc clang REQUIRED)
set(CMAKE_C_COMPILER_TARGET i386-elf)

set(CMAKE_C_FLAGS "-m32 -fno-stack-protector -fno-exceptions -mno-stack-arg-probe -mno-red-zone -mno-mmx -mno-sse -fno-builtin -ffreestanding -fno-pic")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELWITHDBGINFO "-O2 -g")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os")

set(CMAKE_C_LINK_FLAGS "-m elf_i386")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <CMAKE_C_LINK_FLAGS> <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

#
# C++ setup
#
set(CXX_STANDARD 17)
find_program(CMAKE_CXX_COMPILER NAMES g++ clang++ REQUIRED)
set(CMAKE_CXX_COMPILER_TARGET i386-elf)

set(CMAKE_CXX_FLAGS "-m32 -fno-stack-protector -mno-stack-arg-probe -mno-mmx -mno-sse -fno-builtin -ffreestanding -fno-pic")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELWITHDBGINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")

set(CMAKE_CXX_LINK_FLAGS "-m elf_i386")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

#
# Assembly setup
#
find_program(CMAKE_ASM_COMPILER NAMES as clang gcc REQUIRED)
set(CMAKE_ASM_COMPILER_TARGET i386-elf)

if (${CMAKE_ASM_COMPILER} STREQUAL CMAKE_C_COMPILER)
    set(CMAKE_ASM_FLAGS "-m32 -fno-builtin -ffreestanding -fno-pic")
else()
    set(CMAKE_ASM_FLAGS "-march=i386")
endif()
set(CMAKE_ASM_FLAGS_DEBUG "-O0 -g")
set(CMAKE_ASM_FLAGS_RELWITHDBGINFO "-O2 -g")
set(CMAKE_ASM_FLAGS_RELEASE "-O2")
set(CMAKE_ASM_FLAGS_MINSIZEREL "-Os")

set(CMAKE_ASM_LINK_FLAGS "-m elf_i386")
set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_ASM_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

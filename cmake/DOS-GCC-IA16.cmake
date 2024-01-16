find_path(TOOLCHAIN_ROOT
	NAMES ia16-elf-gcc
	PATHS /usr/bin /usr/local/bin /bin)

if(NOT TOOLCHAIN_ROOT)
	message(FATAL_ERROR "Cannot find toolchain root directory")
endif()


file(GLOB TOOLCHAIN_CRTBEGIN
	/usr/lib/*/gcc/ia16-elf/*/crtbegin.o)
if(NOT TOOLCHAIN_CRTBEGIN)
	message(FATAL_ERROR "Cannot find toolchain library directory")
else()
	get_filename_component(TOOLCHAIN_LIBS ${TOOLCHAIN_CRTBEGIN} DIRECTORY CACHE)
endif()


find_path(I86_LIBS
	NAMES libdos-t.a
	PATHS /usr/ia16-elf/lib
	NO_DEFAULT_PATH)

if(NOT I86_LIBS)
	message(FATAL_ERROR "Cannot find target architecture library directory")
endif()


set(CMAKE_SYSTEM_NAME DOS)
set(CMAKE_SYSTEM_PROCESSOR I86)
set(CMAKE_CROSS_COMPILING 1)

set(CMAKE_ASM_COMPILER "${TOOLCHAIN_ROOT}/ia16-elf-gcc"     CACHE PATH "gcc"     FORCE)
set(CMAKE_C_COMPILER   "${TOOLCHAIN_ROOT}/ia16-elf-gcc"     CACHE PATH "gcc"     FORCE)
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_ROOT}/ia16-elf-g++"     CACHE PATH "g++"     FORCE)
set(CMAKE_AR           "${TOOLCHAIN_ROOT}/ia16-elf-ar"      CACHE PATH "ar"      FORCE)
set(CMAKE_LINKER       "${TOOLCHAIN_ROOT}/ia16-elf-ld"      CACHE PATH "linker"  FORCE)
set(CMAKE_NM           "${TOOLCHAIN_ROOT}/ia16-elf-nm"      CACHE PATH "nm"      FORCE)
set(CMAKE_OBJCOPY      "${TOOLCHAIN_ROOT}/ia16-elf-objcopy" CACHE PATH "objcopy" FORCE)
set(CMAKE_OBJDUMP      "${TOOLCHAIN_ROOT}/ia16-elf-objdump" CACHE PATH "objdump" FORCE)
set(CMAKE_STRIP        "${TOOLCHAIN_ROOT}/ia16-elf-strip"   CACHE PATH "strip"   FORCE)
set(CMAKE_RANLIB       "${TOOLCHAIN_ROOT}/ia16-elf-ranlib"  CACHE PATH "ranlib"  FORCE)

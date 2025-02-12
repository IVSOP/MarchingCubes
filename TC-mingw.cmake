# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)
set(CROSS_COMPILE_TOOLCHAIN_PREFIX "x86_64-w64-mingw32")
set(CMAKE_SYSTEM_PROCESSOR x86_64)


# which compilers to use for C and C++
set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
# 32-bit
# set(CMAKE_C_COMPILER   i686-w64-mingw32-gcc)
# set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)

# since I am cross compiling its important for libc pthread etc to be statically linked to avoid problems
# TODO this is an ugly hack
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static") # --plugin,/usr/lib/gcc/x86_64-w64-mingw32/12-posix/liblto_plugin.so") # -static -DGLEW_STATIC

# find_program(GCC_AR NAMES x86_64-w64-mingw32-gcc-ar)
# find_program(GCC_RANLIB NAMES x86_64-w64-mingw32-gcc-ranlib)
# find_library(LIBLTO_PLUGIN NAMES liblto_plugin.so PATHS /usr/lib/gcc/x86_64-w64-mingw32/13.1.0)

# where is the target environment located
# set(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc
#     /home/alex/mingw-install)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# # search headers and libraries in the target environment
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


# had to do sudo update-alternatives --config x86_64-w64-mingw32-gcc and sudo update-alternatives --config x86_64-w64-mingw32-g++ to change to posix threads implementation

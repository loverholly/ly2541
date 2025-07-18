# x86_64-linux-gnu.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# 使用系统自带的 gcc/g++
set(CMAKE_C_COMPILER   gcc)
set(CMAKE_CXX_COMPILER g++)

# 不设置 sysroot，让 CMake 直接依赖 /usr/lib /usr/include
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

cmake_minimum_required(VERSION 3.10)

project(linecmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include/)

add_executable(line
    main.cpp
    linker.cpp
    module.cpp 
    stubs.cpp
    plugin.cpp
    whitelist.cpp
    windows.cpp

    stubs/dirent.cpp
    stubs/dlfcn.cpp
    stubs/epoll.cpp
    stubs/fcntl.cpp
    stubs/ioctl.cpp
    stubs/libc.cpp
    stubs/mqueue.cpp
    stubs/netdb.cpp
    stubs/poll.cpp
    stubs/pthread.cpp
    stubs/select.cpp
    stubs/semaphore.cpp
    stubs/socket.cpp
    stubs/stat.cpp
    stubs/stdio.cpp
    stubs/termios.cpp
    stubs/time.cpp
    stubs/unistd.cpp
    stubs/wchar.cpp

    MinHook/buffer.c
    MinHook/hook.c
    MinHook/trampoline.c
    MinHook/hde/hde32.c
)
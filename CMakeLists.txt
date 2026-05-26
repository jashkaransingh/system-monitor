cmake_minimum_required(VERSION 3.16)
project(system-monitor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(system-monitor
    src/main.cpp
    src/monitor.cpp
    src/filesystem_watcher.cpp
    src/logger.cpp
)

target_include_directories(system-monitor PRIVATE src)

target_compile_options(system-monitor PRIVATE -Wall -Wextra -O2)

install(TARGETS system-monitor DESTINATION /usr/local/bin)

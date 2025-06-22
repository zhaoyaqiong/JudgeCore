
cmake_minimum_required(VERSION 3.10)
project(judgecore C)

set(CMAKE_C_STANDARD 11)
include_directories(include)
find_library(SECCOMP_LIB seccomp)
find_library(JSON_LIB jansson)
add_executable(judgecore
    src/main.c
    src/config.c
    src/judge.c
    src/log.c
    src/result.c
    src/sandbox.c
    src/seccomp.c
    src/syscall_profiles.c
)

add_library(judgecore_shared SHARED
    src/config.c
    src/judge.c
    src/log.c
    src/result.c
    src/sandbox.c
    src/seccomp.c
    src/syscall_profiles.c
)

target_link_libraries(judgecore ${SECCOMP_LIB} ${JSON_LIB})

cmake_minimum_required(VERSION 2.8.4)
project(rpc003)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(STATE_THREADS
    ~/Libs/CLibs/state-threads/obj)

set(SOURCE_FILES
    rpc_log.c
    rpc_log.h
    rpc_main.c
    rpc_peer.c
    rpc_peer.h
    rpc_protocol.c
    rpc_protocol.h
    rpc_queue.c
    rpc_queue.h)

include_directories(${STATE_THREADS}/)
link_directories(${STATE_THREADS}/)
link_libraries(st)

add_executable(rpc ${SOURCE_FILES})
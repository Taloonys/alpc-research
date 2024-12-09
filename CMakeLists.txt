#
# Head
#

cmake_minimum_required(VERSION 3.8.0)

set(CMAKE_CXX_STANDARD              17)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(alpc-example
        LANGUAGES C CXX)

set(PROJECT_ROOT_DIR        ${CMAKE_CURRENT_LIST_DIR})

#
# Directories
#

set(DETAILS_DIR     ${PROJECT_ROOT_DIR}/details)

#
# Server
#

add_executable(alpc-server server.cpp)

target_include_directories(alpc-server
    PRIVATE
        ${DETAILS_DIR})

target_link_libraries(alpc-server
    PRIVATE
        $<$<BOOL:${WIN32}>:ntdll>)

#
# Client
#

add_executable(alpc-client client.cpp)

target_include_directories(alpc-client
    PRIVATE
        ${DETAILS_DIR})

target_link_libraries(alpc-server
    PRIVATE
        $<$<BOOL:${WIN32}>:ntdll>)
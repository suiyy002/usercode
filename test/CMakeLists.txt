cmake_minimum_required(VERSION 2.8)

project(usercode_test)

add_executable(usercode_test test.cpp)

include_directories(${CMAKE_SOURCE_DIR}/../include)

find_library(LIBRARY_DIR usercode ${CMAKE_SOURCE_DIR}/../lib)
if(NOT LIBRARY_DIR)
    message(FATAL_ERROR "libusercode not found")
endif(NOT LIBRARY_DIR)

target_link_libraries(usercode_test ${LIBRARY_DIR} ${CMAKE_SOURCE_DIR}/../lib/libfftw3f.dll)

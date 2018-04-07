cmake_minimum_required(VERSION 2.8)

project(usercode)

aux_source_directory(./ SOURCE_FILES)

add_library(usercode SHARED ${SOURCE_FILES})

include_directories(${CMAKE_SOURCE_DIR}/../include)

link_directories(${CMAKE_SOURCE_DIR}/../lib)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../lib)

target_link_libraries(usercode ${CMAKE_SOURCE_DIR}/../lib/libfftw3f.dll)

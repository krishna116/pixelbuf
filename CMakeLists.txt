cmake_minimum_required(VERSION 4.0)

project(pixelbuf VERSION 1.0.0 LANGUAGES C CXX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.txt ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

add_library(pixelbufLib STATIC
src/PixelBuf.cpp
)

target_include_directories(pixelbufLib PUBLIC
src
)

add_subdirectory(tests)
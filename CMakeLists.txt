cmake_minimum_required(VERSION 3.13)

project(ylems)

enable_testing()

option(YLEMS_NO_TESTS "Do not build test modules." OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)

add_compile_options(
	"$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fPIC;-Wpedantic;-Wall;-Wextra;-Wno-long-long;-std=c++03>"
	"$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:$<$<CONFIG:Release,RelWithDebInfo>:-O3>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release,RelWithDebInfo>:-Ox;-W4>>"
)
add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)

add_library(ylems INTERFACE)
target_include_directories(ylems INTERFACE ./include/.)

if (NOT YLEMS_NO_TESTS)
	add_subdirectory(tests/ylems)
	add_subdirectory(tests/ylems.benchmark)
endif()

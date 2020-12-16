cmake_minimum_required(VERSION 3.16)
project(memoization VERSION 0.1.0)

add_library(memoization INTERFACE)
target_include_directories(memoization INTERFACE include/) 
target_compile_features(memoization INTERFACE cxx_std_17)

enable_testing()
add_subdirectory (test)
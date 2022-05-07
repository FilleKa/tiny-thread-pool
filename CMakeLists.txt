cmake_minimum_required(VERSION 3.10)

project(
    tiny-thread-pool 
    VERSION 0.1.0 
    DESCRIPTION "Tiny but powerful threadpool using std" 
    LANGUAGES CXX
)

add_library(${PROJECT_NAME} src/thread_pool.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)

add_subdirectory(test)

set(CMAKE_CXX_STANDARD 17)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

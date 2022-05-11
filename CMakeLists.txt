cmake_minimum_required(VERSION 3.10)

project(
    tiny-thread-pool 
    VERSION 0.1.0 
    DESCRIPTION "Tiny but powerful thread pool using std" 
    LANGUAGES CXX
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE include)

add_subdirectory(test)

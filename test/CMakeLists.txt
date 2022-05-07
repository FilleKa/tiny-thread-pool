enable_testing()

add_executable(unit-tests
  main.cpp
  
  src/blocking_queue_tests.cpp
  src/thread_pool_tests.cpp
)

target_link_libraries(unit-tests tiny-thread-pool gtest gtest_main pthread)

target_include_directories(unit-tests PRIVATE ../include)
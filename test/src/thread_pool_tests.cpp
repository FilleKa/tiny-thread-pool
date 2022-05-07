#include <gtest/gtest.h>

#include "thread_pool.hpp"

TEST(ThreadPool, ShouldExecuteTask) {
    // Setup
    ttp::ThreadPool thread_pool(4);
    bool success = false;

    // Execute
    thread_pool.Enqueue([&success]() { success = true; }, nullptr);
    thread_pool.Wait();

    // Verify
    EXPECT_TRUE(success);
}

TEST(ThreadPool, ShouldExecuteMultipleTasks) {
    // Setup
    ttp::ThreadPool thread_pool(4);
    int execution_count = 0;

    std::mutex mtx;

    // Execute
    for (int i = 0; i < 1024; i++) {
        thread_pool.Enqueue(
            [&execution_count, &mtx]() {
                std::lock_guard<std::mutex> lock(mtx);
                execution_count++;
            },
            []() {});
    }

    thread_pool.Wait();

    // Verify
    EXPECT_EQ(execution_count, 1024);
}

TEST(ThreadPool, ShouldExecuteCallback) {
    // Setup
    ttp::ThreadPool thread_pool(4);
    bool success = false;

    // Execute
    thread_pool.Enqueue([]() {}, [&success]() { success = true; });
    thread_pool.Wait();
    thread_pool.ExecuteCallbacks();

    // Verify
    EXPECT_TRUE(success);
}

TEST(ThreadPool, ShouldProcessCallback) {
    // Setup
    ttp::ThreadPool thread_pool(4);
    bool success = false;

    // Execute
    thread_pool.Enqueue([]() {}, [&success]() { success = true; });
    thread_pool.Wait();
    thread_pool.ExecuteCallbacks();

    // Verify
    EXPECT_TRUE(success);
}

TEST(ThreadPool, ShouldProcessMultipleCallbacks) {
    // Setup
    ttp::ThreadPool thread_pool(4);
    int execution_count = 0;

    // Execute
    for (int i = 0; i < 1024; i++) {
        thread_pool.Enqueue([]() {}, [&execution_count]() { execution_count++; });
    }
    thread_pool.Wait();
    thread_pool.ExecuteCallbacks();

    // Verify
    EXPECT_EQ(execution_count, 1024);
}

TEST(ThreadPool, ShouldProvideFutureValue) {
    // Setup
    ttp::ThreadPool thread_pool(4);

    // Execute
    auto future_value = thread_pool.Enqueue([]() { return 42; });

    // Verify
    EXPECT_EQ(future_value.get(), 42);
}

TEST(ThreadPool, ShouldProvideMultipleFutureValues) {
    // Setup
    ttp::ThreadPool thread_pool(4);
    int execution_count = 0;

    // Execute
    auto future_value_1 = thread_pool.Enqueue([]() { return 1; });
    auto future_value_2 = thread_pool.Enqueue([]() { return 2; });
    auto future_value_3 = thread_pool.Enqueue([]() { return 3; });
    auto future_value_4 = thread_pool.Enqueue([]() { return; });

    // Verify
    EXPECT_EQ(future_value_1.get(), 1);
    EXPECT_EQ(future_value_2.get(), 2);
    EXPECT_EQ(future_value_3.get(), 3);
}

TEST(ThreadPool, ShouldFinishWorkUponDestruction) {
    // Setup
    bool success = false;
    bool callback_success = false;

    // Execute
    {
        ttp::ThreadPool thread_pool(4);
        thread_pool.Enqueue([&success]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            success = true;
        }, [&callback_success](){
            callback_success = true;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    // Verify
    EXPECT_TRUE(success);
    EXPECT_TRUE(callback_success);
}

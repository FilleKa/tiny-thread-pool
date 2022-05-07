#include <gtest/gtest.h>

#include "blocking_queue.hpp"

TEST(BlockingQueue, ShouldStartEmpty) {
    // Setup
    ttp::BlockingQueue<int> q;

    // Verify
    EXPECT_EQ(q.Size(), 0);
    EXPECT_TRUE(q.Empty());
}

TEST(BlockingQueue, ShouldCorrectlyUpdateSize) {
    // Setup
    ttp::BlockingQueue<int> q;

    // Execute
    q.Push(1);

    // Verify
    EXPECT_EQ(q.Size(), 1);
    EXPECT_FALSE(q.Empty());
}

TEST(BlockingQueue, ShouldPopObjectsInCorrectOrder) {
    // Setup
    ttp::BlockingQueue<int> q;

    // Execute
    q.Push(1);
    q.Push(2);
    q.Push(3);

    // Verify
    EXPECT_EQ(q.Peek(), 1);
    q.Pop();
    EXPECT_EQ(q.Peek(), 2);
    q.Pop();
    EXPECT_EQ(q.Peek(), 3);
    q.Pop();
    EXPECT_EQ(q.Size(), 0);
}

TEST(BlockingQueue, ShouldThrowExceptionIfPoppingAnEmptyQueue) {
    // Setup
    ttp::BlockingQueue<int> q;

    // Execute & Verify
    try {
        q.Pop();
        FAIL();
    } catch (const std::out_of_range& e) {
        EXPECT_EQ(e.what(), std::string("Out of range"));
    } catch (...) {
        FAIL();
    }
}

TEST(BlockingQueue, ShouldThrowExceptionIfPeekingAnEmptyQueue) {
    // Setup
    ttp::BlockingQueue<int> q;

    // Execute & Verify
    try {
        q.Peek();
        FAIL();
    } catch (const std::out_of_range& e) {
        EXPECT_EQ(e.what(), std::string("Out of range"));
    } catch (...) {
        FAIL();
    }
}

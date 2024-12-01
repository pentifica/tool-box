#include    <RingBuffer.h>

#include    <gtest/gtest.h>

#include    <thread>
#include    <vector>
#include    <string>

namespace {
    struct TestObject {
        TestObject() = default;
        TestObject(size_t key, std::string value)
            : key_{key}
            , value_{std::move(value)}
        {}
        TestObject(TestObject const&) = default;
        TestObject(TestObject&&) = default;
        ~TestObject() = default;
        TestObject& operator=(TestObject const&) = default;
        TestObject& operator=(TestObject&&) = default;
        size_t key_{};
        std::string value_{};
    };
};

TEST(Test_RingBuffer, test_init) {
    using namespace pentifica::tbox;
    constexpr size_t capacity{10};

    RingBuffer<TestObject> buffer(capacity);

    ASSERT_EQ(capacity, buffer.Capacity());
    ASSERT_EQ(0, buffer.Size());
    ASSERT_TRUE(buffer.Empty());
}

TEST(Test_RingBuffer, test_push) {
    using namespace pentifica::tbox;

    std::vector<TestObject> test_values {
        {0, "zero"},
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {4, "four"},
        {5, "five"},
        {6, "six"},
        {7, "seven"},
        {8, "eight"},
        {9, "nine"},
    };
    RingBuffer<TestObject> buffer(2 * test_values.size());

    for(auto value : test_values) {
        buffer.Push(value);
    }

    ASSERT_EQ(test_values.size(), buffer.Size());

    for(auto const& value : test_values) {
        buffer.Push(value);
    }

    ASSERT_EQ(2 * test_values.size(), buffer.Size());
}

TEST(Test_RingBuffer, test_try_push) {
    using namespace pentifica::tbox;

    std::vector<TestObject> test_values {
        {0, "zero"},
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {4, "four"},
        {5, "five"},
        {6, "six"},
        {7, "seven"},
        {8, "eight"},
        {9, "nine"},
    };
    RingBuffer<TestObject> buffer(test_values.size());

    for(auto const& value : test_values) {
        buffer.TryPush(value);
    }

    ASSERT_EQ(test_values.size(), buffer.Size());

    for(auto const& value : test_values) {
        auto const actual = buffer.TryPush(value);
        ASSERT_FALSE(actual);
    }
}

TEST(Test_RingBuffer, test_pop) {
    using namespace pentifica::tbox;

    std::vector<TestObject> test_values {
        {0, "zero"},
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {4, "four"},
        {5, "five"},
        {6, "six"},
        {7, "seven"},
        {8, "eight"},
        {9, "nine"},
    };
    RingBuffer<TestObject> buffer(test_values.size());

    for(auto const& value : test_values) {
        buffer.Push(value);
    }

    for(auto const& expected : test_values) {
        auto const& actual = buffer.Pop();
        ASSERT_EQ(actual.key_, expected.key_);
        ASSERT_STREQ(actual.value_.c_str(), expected.value_.c_str());
    }
    ASSERT_TRUE(buffer.Empty());
}

TEST(Test_RingBuffer, test_try_pop) {
    using namespace pentifica::tbox;

    std::vector<TestObject> test_values {
        {0, "zero"},
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {4, "four"},
        {5, "five"},
        {6, "six"},
        {7, "seven"},
        {8, "eight"},
        {9, "nine"},
    };
    RingBuffer<TestObject> buffer(test_values.size());

    for(auto const& value : test_values) {
        buffer.Push(value);
    }

    for(auto const& expected : test_values) {
        auto const& actual = buffer.TryPop();
        ASSERT_TRUE(actual);
        ASSERT_EQ(actual.value().key_, expected.key_);
        ASSERT_STREQ(actual.value().value_.c_str(), expected.value_.c_str());
    }
    ASSERT_TRUE(buffer.Empty());

    for(auto const& expected : test_values) {
        auto const& actual = buffer.TryPop();
        ASSERT_FALSE(actual);
    }
}
TEST(Test_RingBuffer, test_multithread) {
    using namespace pentifica::tbox;
    constexpr size_t nbr_threads{10};
    constexpr size_t nbr_events{10000};
    constexpr size_t capacity{100};
    using TestBuffer = RingBuffer<TestObject>;

    TestBuffer buffer(capacity);

    auto server = [&buffer, &nbr_events](size_t id) {
        for(size_t event = 0; event < nbr_events; ++event) {
            buffer.Push({id, std::to_string(event)});
        }
    };

    auto client = [&buffer, &nbr_events]() {
        for(size_t event = 0; event < nbr_events; ++event) {
            auto const& obj = buffer.Pop();
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(2 * nbr_threads);

    for(size_t i = 0; i < nbr_threads; ++i) {
        threads.emplace_back(std::thread(server, i));
        threads.emplace_back(std::thread(client));
    }

    for(auto& thread : threads) {
        if(thread.joinable()) {
            thread.join();
        }
    }

    ASSERT_TRUE(buffer.Empty());
}
#include    <Generator.h>

#include    <gtest/gtest.h>

#include    <vector>

namespace {
    using namespace pentifica::tbox;

    Generator<int> simple_sequence(int start, int end) {
        for(int i = start; i <= end; i++) {
            co_yield i;
        }
    }
    struct Data {
        int value_{};
        std::string description_{};
        Data(int value, std::string description)
            : value_(value)
            , description_(description)
        {}
        Data() = default;
        Data(Data const&) = default;
        Data(Data&&) = default;
        ~Data() = default;
        Data& operator=(Data const&) = default;
        Data& operator=(Data&&) = default;
    };

    Generator<Data> object_sequence(Data seed, size_t count) {
        for(; count > 0; count--) {
            seed.value_ *= 2;
            seed.description_ = std::to_string(seed.value_);
            co_yield seed;
        }
    }
}

TEST(Test_Generator, test_simple_sequence) {
    using namespace pentifica::tbox;

    std::vector<int> results = {1, 2, 3, 4, 5};
    auto sequence = simple_sequence(1, 5);

    size_t expected{};

    for(auto actual = sequence.next(); actual; actual = sequence.next()) {
        ASSERT_LT(expected, results.size());
        ASSERT_EQ(results[expected], actual.value());
        expected++;
    }

    ASSERT_EQ(expected, results.size());
}

TEST(Test_Generator, test_iterator) {
    using namespace pentifica::tbox;

    std::vector<int> results = {1, 2, 3, 4, 5};
    auto sequence = simple_sequence(1, 5);

    size_t expected{};

    for(auto actual : sequence) {
        ASSERT_LT(expected, results.size());
        ASSERT_EQ(results[expected], actual);
        expected++;
    }

    ASSERT_EQ(expected, results.size());
}

TEST(Test_Generator, test_object_sequence) {
    using namespace pentifica::tbox;

    std::vector<Data> results = {
        { 2, "2"},
        { 4, "4"},
        { 8, "8"},
        {16, "16"},
        {32, "32"},
    };
    auto sequence = object_sequence(Data(1, "one"), 5);

    size_t expected{};

    for(auto actual : sequence) {
        ASSERT_LT(expected, results.size());
        auto const& check = results[expected];
        ASSERT_EQ(check.value_, actual.value_);
        ASSERT_STREQ(check.description_.c_str(), actual.description_.c_str());
        expected++;
    }

    ASSERT_EQ(expected, results.size());
}
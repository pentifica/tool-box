#include    <SkipList.h>
#include    <SkipListGen.h>

#include    <gtest/gtest.h>

#include    <sstream>
#include    <tuple>
#include    <algorithm>
#include    <unordered_set>
#include    <format>
#include    <iostream>

namespace {
    using namespace pentifica::tbox;

    constexpr int max_level{5};
    std::function<int(int)> level_generator = SkipListLevelGenerator(.5);

    /// @brief Encapsulates a key
    /// @tparam T 
    template<typename T>
    struct Wrapper {
        using value_type = T;
        Wrapper() {}
        Wrapper(value_type value) : value_(std::move(value)) {}
        bool operator==(Wrapper const& other) const { return value_ == other.value_; }
        bool operator>(Wrapper const& other) const { return value_ > other.value_; }
        Wrapper& operator=(Wrapper const& other) { value_ = other.value_; return *this; }
        Wrapper& operator=(value_type const& update) { value_ = update; return *this; }
        value_type value_{};
    };
    struct SKey : public Wrapper<std::string> {
        using Wrapper<std::string>::Wrapper;
    };

    struct SValue : public Wrapper<std::string> {
        using Wrapper<std::string>::Wrapper;
    };

    using Key = std::string;
    using Value = std::string;

    /// @brief  Random word builder
    struct RndWordGen {
        /// @brief Initialization
        /// @param len  Generate words of length 1..len
        /// @param no_dup   If true, do not return duplicates
        RndWordGen(size_t len, bool no_dup = false)
            : len_(len)
            , no_dup_(no_dup)
        {}
        /// @brief Returns the next random character
        /// @return 
        char NextChar() {
            return pool_[pool_distribution_(pool_rng_)];
        }
        /// @brief  Returns the next unique word
        /// @return 
        std::string NextUniqueWord() {
            auto word_len = distribution_(rng_);
            std::string word(word_len, ' ');
            for(size_t i = 0; i < word_len; i++) {
                word[i] = NextChar();
            }
            if(!generated_words_.emplace(word).second) {
                return NextUniqueWord();
            }
            return word;
        }
        /// @brief Returns the next random word
        /// @return 
        std::string NextAnyWord() {
            auto word_len = distribution_(rng_);
            std::string word(word_len, ' ');
            for(size_t i = 0; i < word_len; i++) {
                word[i] = NextChar();
            }
            return word;
        }
        /// @brief  Returns the next generated word
        /// @return 
        std::string NextWord() {
            return no_dup_ ? NextUniqueWord() : NextAnyWord();
        }
        size_t const len_{};
        bool const no_dup_{};
        std::unordered_set<std::string> generated_words_{};
        std::mt19937 rng_{std::mt19937(std::time(nullptr))};
        std::uniform_int_distribution<size_t>
            distribution_{std::uniform_int_distribution<size_t>(1, len_)};
        static constexpr char pool_[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 _-";
        static constexpr size_t pool_len_{sizeof(pool_)-1};
        std::mt19937 pool_rng_{std::mt19937(std::time(nullptr))};
        std::uniform_int_distribution<size_t>
            pool_distribution_{std::uniform_int_distribution<size_t>(0, pool_len_-1)};
    };

    /// @brief  Function to generate random kv pairs of strings
    /// @param  pair_count  Number of kv_pairs to generate
    /// @param  key_len     Key string length range (1..key_len)
    /// @param  value_len   Value sring length range (1..value_len)
    /// @param  no_dups     If set to true, no duplicate keys are generated
    ///                     (default = false)
    /// @return Container of key/value pairs
    std::vector<std::pair<std::string, std::string>>
    GenKvPairs(size_t pair_count, size_t key_len, size_t value_len, bool no_dups = false) {
        RndWordGen key_gen(key_len, no_dups);
        RndWordGen value_gen(value_len);
        std::vector<std::pair<std::string, std::string>> kv_pairs;
        for(size_t i = 0; i < pair_count; i++) {
            kv_pairs.emplace_back(std::make_pair(key_gen.NextWord(), value_gen.NextWord()));
        }
        return kv_pairs;
    }
    //  stream operator for SkipListNode
    //
    template<typename K, typename V>
    std::ostream&
    operator<<(std::ostream& os, SkipListNode<K, V> const& node) {
        os << std::format("SkipListNode[{}] [{}:{}] @ {}",
            node.Level(), node.Key(), node.Value(), (void*)(&node))
            << std::endl;

        return os;            
    }
    //  stream operator for SkipList
    //
    template<typename K, typename V>
    std::ostream&
    operator<<(std::ostream& os, SkipList<K, V> const& skip_list) {
        os << "===== Printing SkipList" << std::endl;
    
        for(auto const& node : skip_list) {
            os << node;
        }
    
        os << "===== END Printing SkipList" << std::endl;
    
        return os;
    }
}

TEST(Test_SkipListLevelGenerator, test_values) {
    using namespace pentifica::tbox;

    SkipListLevelGenerator generator(.5);

    constexpr int loops = 1000;
    for(int i = 0; i < loops; i++) {
        auto level = generator(max_level);
        ASSERT_LE(0, level);
        ASSERT_GT(max_level, level);
    }
}
TEST(Test_SkipListNode, test_init) {
    using namespace pentifica::tbox;
    using SkipListNodeType = SkipListNode<Key, Value>;

    const Key key{"key"};
    const Value value{"value"};
    constexpr size_t current_level{5};

    SkipListNodeType node(current_level, key, value);

    ASSERT_EQ(current_level, node.Level());
    ASSERT_EQ(key, node.Key());
    ASSERT_EQ(value, node.Value());
}

TEST(Test_SkipList, test_init) {
    using namespace pentifica::tbox;
    using SkipListType = SkipList<Key, Value>;

    auto sl = SkipListType(max_level, level_generator);
    ASSERT_TRUE(sl.Empty());
    ASSERT_EQ(0, sl.Size());
}

TEST(Test_SkipList, test_insert) {
    using namespace pentifica::tbox;
    using SkipListType = SkipList<Key, Value>;

    //  set up the test
    std::clog << std::format("Initializing SkipList with level {}", max_level)
              << std::endl;            
    auto sl = new SkipListType(max_level, level_generator);

    std::vector<std::tuple<std::string, std::string, size_t>> kv_pairs = {
        {"hello", "world", 1},         {"something", "else", 2},
        {"enter", "exit", 3},           {"the red fox", "jumped and played", 4},
        {"hello", "world2", 4},        {"something", "other", 4},
        {"on a sunny day", "you can see for miles", 5}
    };

    for(auto const& [key, value, count] : kv_pairs) {
        auto const& added = sl->Insert(key, value);
        ASSERT_EQ(added, value);
        ASSERT_EQ(sl->Size(), count);
        std::clog << *sl;
    }
}

TEST(Test_SkipList, test_insert_and_find) {
    using namespace pentifica::tbox;
    using SkipListType = SkipList<Key, Value>;

    //  initialize test
    std::clog << std::format("Initializing SkipList with level {}", max_level)
              << std::endl;
    auto skip_list = SkipListType(max_level, level_generator);
    std::clog << skip_list;

    //  load data into the skip list
    std::vector<std::tuple<std::string, std::string>> kv_pairs {
        {"hello", "world"},
        {"something", "else"},
        {"enter", "exit"},
        {"hello", "world2"},
    };

    for(auto const& [key, value] : kv_pairs) {
        ASSERT_EQ(value, skip_list.Insert(key, value));
    }

    //  find elements in the SkipList
    std::vector<std::tuple<std::string, std::optional<std::string>>> test_cases = {
        {"something", "else"},
        {"enter", "exit"},
        {"this isn't there", std::nullopt},
        {"hello", "world2"},
        {"nonexistent", std::nullopt},
    };

    for(auto const& [key, expected] : test_cases) {
        ASSERT_EQ(skip_list.Find(key), expected);
    }
}

TEST(Test_SkipList, test_delete) {
    using namespace pentifica::tbox;
    using SkipListType = SkipList<Key, Value>;

    std::clog << std::format("initializing SkipList with levels {}\n", max_level);
    auto skip_list = SkipListType(max_level, level_generator);

    //  populate the skip list
    //
    constexpr size_t num_kv_pairs{20};
    constexpr size_t key_len{10};
    constexpr size_t value_len{20};
    auto const& kv_pairs = GenKvPairs(num_kv_pairs, key_len, value_len, true);
    ASSERT_EQ(num_kv_pairs, kv_pairs.size());

    for(auto const& kv_pair : kv_pairs) {
        auto const& [key, value] = kv_pair;
        skip_list.Insert(key, value);
    }

    auto count{skip_list.Size()};
    for(auto [key, _] : kv_pairs) {
        std::clog << std::format("Deleting {}\n", key);
        auto actual = skip_list.Delete(key);
        std::clog << skip_list;
        ASSERT_EQ(actual, SkipListError::NOERR);
        --count;
        ASSERT_EQ(count, skip_list.Size());
    }

    for(auto [key, _] : kv_pairs) {
        std::clog << std::format("Checking deleted key '{}'\n", key);
        auto actual = skip_list.Delete(key);
        ASSERT_EQ(actual, SkipListError::KEY_NOT_FOUND);
    }
}

TEST(Test_SkipList, test_iter_init) {
    using namespace pentifica::tbox;
    using SkipListType = SkipList<Key, Value>;

    std::clog << std::format("initializing SkipList with levels {}\n", max_level);
    auto skip_list = SkipListType(max_level, level_generator);

    auto begin = skip_list.begin();
    auto end = skip_list.end();

    ASSERT_EQ(begin, end);
    ASSERT_EQ(++begin, end);
    ASSERT_EQ(begin++, end);
    ASSERT_EQ(begin, end++);
    ASSERT_EQ(begin, ++end);
}

TEST(Test_SkipList, test_iter_traverse) {
    using namespace pentifica::tbox;
    using SkipListType = SkipList<std::string, std::string>;

    std::clog << std::format("initializing SkipList with levels {}\n", max_level);
    auto skip_list = SkipListType(max_level, level_generator);

    //  populate the skip list
    //
    constexpr size_t num_kv_pairs{1000};
    constexpr size_t key_len{10};
    constexpr size_t value_len{20};
    auto const& kv_pairs = GenKvPairs(num_kv_pairs, key_len, value_len, true);
    ASSERT_EQ(num_kv_pairs, kv_pairs.size());

    for(auto const& kv_pair : kv_pairs) {
        auto const& [key, value] = kv_pair;
        skip_list.Insert(key, value);
    }
    
    //  verify the list iterator transverses the list in sorted order
    auto sorted_kv_pairs{kv_pairs};

    std::sort(sorted_kv_pairs.begin(), sorted_kv_pairs.end(),
        [](auto const& a, auto const& b) {
            return a.first < b.first;
        }
    );

    size_t index{};
    for(auto const& actual : skip_list) {
        auto const& [expected_key, expected_value] = sorted_kv_pairs[index++];
        ASSERT_EQ(actual.Key(), expected_key);
        ASSERT_EQ(actual.Value(), expected_value);
    }
}
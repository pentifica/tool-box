#include    <SkipList.h>
#include    <SkipListGen.h>

#include    <gtest/gtest.h>

#include    <sstream>
#include    <tuple>
#include    <algorithm>

namespace {
    using namespace pentifica::tbox;

    constexpr int max_level{5};
    std::function<int(int)> level_generator = SkipListLevelGenerator(.5);
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

    std::string key{"key"};
    std::string value{"value"};
    size_t current_level{5};

    SkipListNode node(current_level, key, value);

    std::ostringstream actual;
    std::ostringstream expected;
    expected << std::format("SkipListNode[{}] [{}:{}] @ {}",
        current_level, key, value, (void*)(&node)) << std::endl;
    
    actual << node;
    EXPECT_STREQ(actual.str().c_str(), expected.str().c_str());
}

TEST(Test_SkipList, test_init) {
    using namespace pentifica::tbox;

    auto sl = SkipList(max_level, level_generator);
    for(auto const& link : sl.start_sentinel_->links_) {
        ASSERT_EQ(link, sl.end_sentinel_);
    }
}

TEST(Test_SkipList, test_insert) {
    using namespace pentifica::tbox;

    //  linear search for the key in the skip list
    auto linear_search = [](std::string key, SkipList* sl) {
        auto current_node = sl->start_sentinel_;
        while(current_node != sl->end_sentinel_) {
            if(current_node->key_ == key) {
                return SkipListError::ErrorVariant::NOERR;
            }
            else {
                current_node = current_node->links_[0];
            }
        }
        return SkipListError::ErrorVariant::KEY_NOT_FOUND;
    };

    //  Check the monotonicity of the keys
    auto monotonicity_check = [](SkipList* sl) {
        auto current_node = sl->start_sentinel_->links_[0];
        std::string prev_key{};
        std::string curr_key{};
        while(current_node != sl->end_sentinel_) {
            prev_key = curr_key;
            curr_key = current_node->key_;
            if(!(curr_key > prev_key)) {
                return SkipListError::ErrorVariant::BAD_ACCESS;
            }
            current_node = current_node->links_[0];
        }
        return SkipListError::ErrorVariant::NOERR;
    };

    //  set up the test
    std::clog << std::format("Initializing SkipList with level {}", max_level)
              << std::endl;            
    auto sl = new SkipList(max_level, level_generator);

    std::vector<std::tuple<std::string, std::string>> kv_pairs = {
        {"hello", "world"},         {"something", "else"},
        {"enter", "sandman"},       {"the struts", "could have been me"},
        {"hello", "world2"},        {"END_KEY", "SYSTEM BROKEN!!!"}
    };

    for(auto const& [key, value] : kv_pairs) {
        auto const& added = sl->Insert(key, value);
        ASSERT_STREQ(added.c_str(), value.c_str());
        std::clog << *sl;
    }
}

TEST(Test_SkipList, test_insert_and_search) {
    using namespace pentifica::tbox;

    //  initialize test
    std::clog << std::format("Initializing SkipList with level {}", max_level)
              << std::endl;
    auto skip_list = SkipList(max_level, level_generator);
    std::clog << skip_list;

    //  load data into the skip list
    std::vector<std::tuple<std::string, std::string>> kv_pairs {
        {"hello", "world"},
        {"something", "else"},
        {"enter", "sandman"},
        {"martin garrix", "under pressure"},
        {"the smiths", "please please please let me get what I want"},
        {"the struts", "could have been me"},
        {"hello", "world2"},
    };

    for(auto const& [key, value] : kv_pairs) {
        ASSERT_STREQ(value.c_str(), skip_list.Insert(key, value).c_str());
    }

    //  search for elements in the SkipList
    std::vector<std::tuple<std::string, std::optional<std::string>>> test_cases = {
        {"something", "else"},
        {"enter", "sandman"},
        {"the struts", "could have been me"},
        {"martin garrix", "under pressure"},
        {"this isn't there", std::nullopt},
        {"the smiths", "please please please let me get what I want"},
        {"hello", "world2"},
        {"nonexistent", std::nullopt},
    };

    for(auto [key, search_result] : test_cases) {
        ASSERT_EQ(skip_list.Search(key), search_result);
    }
}

TEST(Test_SkipList, test_delete) {
    using namespace pentifica::tbox;

    std::clog << std::format("initializing SkipList with levels {}\n", max_level);
    auto skip_list = SkipList(max_level, level_generator);

    //  load some data into the SkipList
    std::vector<std::tuple<std::string, std::string>> kv_pairs {
        {"hello", "world"},
        {"something", "else"},
        {"enter", "sandman"},
        {"martin garrix", "under pressure"},
        {"the smiths", "please please please let me get what I want"},
        {"the struts", "could have been me"},
        {"hello", "world2"},
    };

    for(auto const& [key, value] : kv_pairs) {
        ASSERT_STREQ(value.c_str(), skip_list.Insert(key, value).c_str());
    }

    //  delete some nodes
    std::vector<std::tuple<std::string, SkipListError::ErrorVariant>> test_cases = {
        {"hello", SkipListError::NOERR},
        {"enter", SkipListError::NOERR},
        {"this will break", SkipListError::KEY_NOT_FOUND},
    };

    for(auto [key, expected] : test_cases) {
        std::clog << std::format("Deleting {}\n", key);
        auto actual = skip_list.Delete(key);
        std::clog << skip_list;
        ASSERT_EQ(actual, expected);
    }
}

TEST(Test_SkipList, test_scan) {
    using namespace pentifica::tbox;

    std::clog << std::format("initializing SkipList with levels {}\n", max_level);
    auto skip_list = SkipList(max_level, level_generator);

    //  Verify an empty list
    auto const& empty_list = skip_list.Scan();
    ASSERT_TRUE(empty_list.empty());

    //  load some data into the SkipList
    std::vector<std::pair<std::string, std::string>> kv_pairs {
        {"hello", "world"},
        {"something", "else"},
        {"enter", "sandman"},
        {"martin garrix", "under pressure"},
        {"the smiths", "please please please let me get what I want"},
        {"the struts", "could have been me"},
    };

    for(auto const& pair : kv_pairs) {
        auto const& [key, value] = pair;
        ASSERT_STREQ(value.c_str(), skip_list.Insert(key, value).c_str());
    }

    //  get a sorted list of the pairs
    auto expected = kv_pairs;

    std::sort(expected.begin(), expected.end(),
        [](auto const& a, auto const& b) {
            return a.first < b.first;
        }
    );

    std::clog << "Printing out sorted list:\n";
    for(auto& pair : expected) {
        auto const& [key, value] = pair;
        std::clog << std::format("Key {}, Value {}\n", key, value);
    }

    auto actual = skip_list.Scan();
    ASSERT_EQ(actual, expected);
}
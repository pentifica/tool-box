#include    <Utility.h>

#include    <gtest/gtest.h>

#include    <string>
#include    <sstream>

namespace {

    std::string const expected{"Functor fired"};
    struct Functor {
        explicit Functor(std::string& s) : s_{s} {}
        void operator()() { s_ = expected; }
    private:
        std::string& s_;
    };
}

TEST(Test_Utility, Test_RAII) {
    using namespace pentifica::tbox;

    std::string result{};
    {
        RAII raii(Functor{result});
        EXPECT_TRUE(result.empty());
    }
    EXPECT_FALSE(result.empty());
    EXPECT_STREQ(result.c_str(), expected.c_str());

    result.clear();
    {
        RAII raii([&result]() { result = expected; });
        EXPECT_TRUE(result.empty());
    }
    EXPECT_FALSE(result.empty());
    EXPECT_STREQ(result.c_str(), expected.c_str());
}

TEST(Test_Utility, Test_PrintTuple) {
    using namespace pentifica::tbox;

    auto data = std::make_tuple(5, 6.0, "This is a test");
    std::ostringstream oss;
}
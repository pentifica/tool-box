#include    <StrSwitch.h>

#include    <gtest/gtest.h>

#include    <string>
#include    <sstream>
#include    <iostream>

TEST(Test_StrSwitch, Test_String) {
    using namespace pentifica::tbox;
    using namespace pentifica::tbox::literals;

    const std::string value = "a";

    std::ostringstream oss;

    switch(pentifica::tbox::hash(value)) {
        case "a"_sh:
            oss << "a";
            break;
        case "b"_sh:
            oss << "b";
        default:
            oss << "Seriously dude, I don't know...";
    }

    EXPECT_TRUE(value == oss.str());
}

TEST(Test_StrSwitch, Test_Char) {
    using namespace pentifica::tbox;
    using namespace pentifica::tbox::literals;

    const char* value = "a";

    std::ostringstream oss;

    switch(pentifica::tbox::hash(value)) {
        case "a"_sh:
            oss << "a";
            break;
        case "b"_sh:
            oss << "b";
        default:
            oss << "Seriously dude, I don't know...";
    }

    EXPECT_STREQ(value, oss.str().c_str());
}

TEST(Test_StrSwitch, Test_Literal) {
    using namespace pentifica::tbox;
    using namespace pentifica::tbox::literals;

    std::ostringstream oss;

    switch(pentifica::tbox::hash("a 1")) {
        case "a 1"_sh:
            oss << "a 1";
            break;
        case "b"_sh:
            oss << "b";
        default:
            oss << "Seriously dude, I don't know...";
    }

    EXPECT_STREQ("a 1", oss.str().c_str());
}
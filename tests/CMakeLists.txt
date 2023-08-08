include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(test_toolbox
    Test_Factory.cpp
    )

target_link_libraries(test_toolbox
    PRIVATE
        GTest::GTest
        toolbox
)

target_include_directories(test_toolbox PUBLIC "${PROJECT_BINARY_DIR}/../src")

add_test(NAME example_test COMMAND test_logging)
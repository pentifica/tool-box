#include    <Factory.h>

#include    <gtest/gtest.h>

#include    <thread>
#include    <atomic>
#include    <mutex>
#include    <chrono>
#include    <vector>
#include    <iostream>
#include    <random>

namespace {
    static std::atomic<size_t> count{};

    struct Product {
        Product() = default;
        Product(std::string name) : name_{std::move(name)} {
            ++count;
        }
        ~Product() { --count; }

        std::string const& Name() const { return name_; }

    private:
        std::string name_{};
    };

    struct Test_Factory : public ::testing::Test {
        void SetUp() override {
            using namespace pentifica::tbox;
            initial_capacity = Factory<Product>::Capacity();
            initial_available = Factory<Product>::Available();
        }
        size_t initial_capacity{};
        size_t initial_available{};
    };

    using namespace pentifica::tbox;
    using ProductFactory = Factory<Product>;
}

TEST_F(Test_Factory, CheckDefaults) {
    EXPECT_EQ(ProductFactory::Capacity(), 0);
    EXPECT_EQ(ProductFactory::Available(), 0);
    {
        auto a = ProductFactory::Create("This is a test");
        EXPECT_EQ(ProductFactory::Capacity(), 1);
        EXPECT_EQ(ProductFactory::Available(), 0);
        EXPECT_EQ(count.load(), 1);
    }
    EXPECT_EQ(ProductFactory::Available(), 1);
    EXPECT_EQ(count.load(), 0);
}

TEST_F(Test_Factory, CheckCapacity) {
    constexpr size_t capacity {20};
    ProductFactory::AddCapacity(capacity);
    EXPECT_EQ(ProductFactory::Capacity(), capacity + initial_capacity);
    EXPECT_EQ(ProductFactory::Available(), capacity + initial_available);
}

TEST_F(Test_Factory, Usage) {
    constexpr size_t additional_capacity{20};
    ProductFactory::AddCapacity(additional_capacity);
    EXPECT_EQ(ProductFactory::Capacity(), additional_capacity + initial_capacity);
    EXPECT_EQ(ProductFactory::Available(), additional_capacity + initial_available);

    using Cache = std::vector<ProductFactory::ProductRef>;
    Cache cache;
    auto const create_count = ProductFactory::Available();
    for(size_t i = 0; i < create_count; i++) {
        cache.emplace_back(ProductFactory::Create("test"));
    }
    EXPECT_EQ(ProductFactory::Capacity(), initial_capacity + additional_capacity);
    EXPECT_EQ(ProductFactory::Available(), 0);

    cache.clear();
    EXPECT_EQ(ProductFactory::Capacity(), ProductFactory::Available());
}

TEST_F(Test_Factory, multiuser) {
    using namespace pentifica::tbox;
    using namespace std::chrono_literals;

    constexpr size_t additional_capacity{20};
    constexpr size_t users{80};
    constexpr size_t cycles{10000};

    std::mutex check_mutex;

    ProductFactory::AddCapacity(additional_capacity);
    EXPECT_EQ(ProductFactory::Capacity(), additional_capacity + initial_capacity);
    EXPECT_EQ(ProductFactory::Available(), additional_capacity + initial_available);

    std::random_device dev;

    using Engine = std::mt19937_64;
    std::uniform_int_distribution<Engine::result_type> dist(10, 75);

    auto user = [&] () {
        thread_local Engine rng(dev());
        std::ostringstream oss;
        oss << "Thread " << std::this_thread::get_id();
        std::this_thread::sleep_for(std::chrono::microseconds(dist(rng)));

        for(size_t cycle = 0; cycle < cycles; ++cycle) {
            auto product{ProductFactory::Create(oss.str())};
            std::this_thread::sleep_for(std::chrono::microseconds(dist(rng)));
            std::lock_guard<std::mutex> lock(check_mutex);
            EXPECT_TRUE(static_cast<Product*>(product.get())->Name() == oss.str());
        }
    };

    std::vector<std::thread> threads(users);
    for(auto& thread : threads) thread = std::thread(user);
    for(auto& thread : threads) thread.join();

    EXPECT_EQ(ProductFactory::Capacity(), ProductFactory::Available());
    EXPECT_LE(ProductFactory::Capacity(), users);
}
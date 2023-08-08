#pragma once
/// @copyright {2023, Russell J. Fleming. All rights reserved.}
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#include    <memory>
#include    <atomic>
#include    <forward_list>
#include    <mutex>

namespace pentifica::tbox {
    /// @brief  Defines a Factory for creating instances of the Product
    ///         class. When a created instance is released, it is returned
    ///         to the Factory to be used when creating another instance.
    /// @tparam Product A class that must support the following minimal interface:
    ///             - default ctor
    template<typename Product>
    class Factory {
        /// @brief  Frees the cached storage for an unused instance of Product
        /// @param  product A reference to the unused instance of Product
        static constexpr auto memory_order = std::memory_order_relaxed;
        
    public:
        /// @brief This class contains only static methods
        ~Factory() = delete;
        using ProductRef = std::unique_ptr<Product, void(*)(Product*)>;
        using ProductList = std::forward_list<ProductRef>;
        /// @brief  Creates an instance of product using the ctor matching the supplied
        ///         arguments. The instance will be created either from an deleted
        ///         instance from the cache or created by allocating an instance from
        ///         the heap.
        /// @tparam ...Ts       The parameter pack definition for the Product ctor.
        /// @param ...params    The parameter pack values
        /// @return An instance of Event based on Product.
        template<typename... Ts>
        static ProductRef Create(Ts&&... params) {
            static_assert(std::is_constructible_v<Product, Ts...>, "No ctor defined");

            if constexpr(std::is_constructible_v<Product, Ts...>) {
                Product* product{};
    
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if(!free_products_.empty()) {
                        product = free_products_.front().release();
                        free_products_.pop_front();
                    }
                }
    
                if(product != nullptr) {
                    product = new(product) Product(std::forward<Ts>(params)...);
                }
    
                else {
                    product = new Product(std::forward<Ts>(params)...);
                    capacity_.fetch_add(1, memory_order);
                }
    
                in_use_.fetch_add(1, memory_order);
                return {product, &ReclaimProduct};
            }
            
            else {
                return {nullptr, nullptr};
            }
        }
        /// @brief  Increases the size of the cache by the amount specified.
        /// @param  increase    The amount to grow the cache
        static void AddCapacity(size_t increase) {
            static_assert(std::is_default_constructible_v<Product>, "No default ctor defined");
            if(increase == 0) return;

            if constexpr(std::is_default_constructible_v<Product>) {
                ProductList additional {};
                for(size_t i = 0; i < increase; i++) {
                    auto product = static_cast<Product*>(aligned_alloc(alignof(Product), sizeof(Product)));
                    additional.emplace_front(product, &ProductDeleter);
                }
    
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    free_products_.splice_after(free_products_.before_begin(), additional);
                }
    
                capacity_.fetch_add(increase, memory_order);
            }
        }

        static auto Capacity() { return capacity_.load(memory_order); }

        static auto Available() { return capacity_.load(memory_order) - in_use_.load(memory_order); }

    private:
        static void ProductDeleter(Product* product) { std::free(product); }
        /// @brief  Resets (via dtor) the Product instance and returns it to the
        ///         internal cache of the factory.
        /// @param  product Instance to recover.
        static void ReclaimProduct(Product* product) {
            product->~Product();
            {
                std::lock_guard<std::mutex> lock(mutex_);
                free_products_.emplace_front(product, &ProductDeleter);
            }
            in_use_.fetch_sub(1, memory_order);
        }

    private:
        static ProductList free_products_;
        static std::atomic<size_t> capacity_;
        static std::atomic<size_t> in_use_;
        static std::mutex mutex_;
    };

    template<typename T>
    Factory<T>::ProductList Factory<T>::free_products_{};
    
    template<typename T>
    std::atomic<size_t> Factory<T>::capacity_{};
    
    template<typename T>
    std::atomic<size_t> Factory<T>::in_use_{};

    template<typename T>
    std::mutex Factory<T>::mutex_;
}
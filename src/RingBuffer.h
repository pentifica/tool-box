#pragma once
/// @copyright {2024, Russell J. Fleming. All rights reserved.}
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
//======================================================================
//  HEADER FILES
//======================================================================
#include    <atomic>
#include    <memory>
#include    <optional>
#include    <mutex>
#include    <vector>
#include    <concepts>
#include    <type_traits>
//======================================================================
//  RingBuffer DEFINITIONS
//======================================================================
namespace pentifica::tbox {

    template<typename T, typename M>
    concept RB_type_traits = requires(T, M mutex) {
        requires std::is_default_constructible_v<T>;
        requires std::is_trivially_copy_constructible_v<T>
              || std::is_copy_constructible_v<T>;
        requires std::is_move_constructible_v<T>;
        requires std::is_assignable_v<T, T>;
    };
    /** This class encapsulates the base characteristics for a circular buffer */
    template<typename T, typename M = std::mutex, typename S = size_t>
    requires RB_type_traits<T, M>
    class RingBuffer {
    protected:
        using Buffer = std::vector<T>;
        using PopResult = std::optional<T>;
        using IndexResult = std::optional<S>;
        using mutex_type = M;

    public:
        /// @brief  Prepare an instance
        /// @param  size    The capacity of the ring buffer
        RingBuffer(S size)
            : capacity_{size}
            , ring_buffer_{size}
        {}
        //  deleted operations
        RingBuffer(RingBuffer const&) = delete;
        RingBuffer(RingBuffer&&) = delete;
        RingBuffer& operator=(RingBuffer const&) = delete;
        RingBuffer& operator=(RingBuffer&&) = delete;
        /// @brief  Release all resources
        virtual ~RingBuffer() = default;
        /// @brief Returns the number of items in the ring
        /// @return 
        auto Size() const { return size_.load(std::memory_order_relaxed); }
        /// @brief  Returns the status of the buffer
        /// @return Returnss true if the buffer is empty
        bool Empty() const { return Size() == 0; }
        /// @brief Returns the capacity of the ring
        /// @return 
        auto Capacity() const { return capacity_; }
        /// @brief  Add an instance to the end of the ring. If the ring is at
        ///         capacity, the thread is blocked until the instance can be
        ///         added.
        /// @param obj  The instance to add
        void Push(T const& obj) {
            std::unique_lock<mutex_type>  lck(push_mutex_);
            while(size_.load(std::memory_order_relaxed) == capacity_);
            ring_buffer_[Normalize(write_next_++)] = obj;
            size_.fetch_add(1, std::memory_order_acq_rel);
        }
        void Push(T& obj) {
            std::unique_lock<mutex_type>  lck(push_mutex_);
            while(size_.load(std::memory_order_relaxed) == capacity_);
            ring_buffer_[Normalize(write_next_++)] = std::move(obj);
            size_.fetch_add(1, std::memory_order_acq_rel);
        }
        /// @brief Try to add an instance to the end of the ring. If the ring is at
        ///        capacity, the instance is not added and control flow returns to
        ///        the caller
        /// @param obj  The instance to add
        /// @return True if the instance added.
        bool TryPush(T const& obj) {
            if(!push_mutex_.try_lock()) return false;
            if(size_.load(std::memory_order_relaxed) == capacity_) return false;
            ring_buffer_[Normalize(write_next_++)] = obj;
            size_.fetch_add(1, std::memory_order_acq_rel);
            push_mutex_.unlock();
            return true;
        }
        /// @brief Try to add an instance to the end of the ring. If the ring is at
        ///        capacity, the instance is not added and control flow returns to
        ///        the caller
        /// @param obj  The instance to add
        /// @return True if the instance added.
        bool TryPush(T& obj) {
            if(!push_mutex_.try_lock()) return false;
            if(size_.load(std::memory_order_relaxed) == capacity_) return false;
            ring_buffer_[Normalize(write_next_++)] = std::move(obj);
            size_.fetch_add(1, std::memory_order_acq_rel);
            push_mutex_.unlock();
            return true;
        }
        /// @brief  Returns the next item from the ring. If no item is available
        ///         the thread is blocked until an item is available.
        /// @return 
        T Pop() {
            std::unique_lock<mutex_type> lck(pop_mutex_);
            while(size_.load(std::memory_order_relaxed) == 0);
            auto obj{std::move(ring_buffer_[Normalize(read_next_++)])};
            size_.fetch_sub(1, std::memory_order_acq_rel);
            return std::move(obj);
        }
        /// @brief  Optionally returns the next item from the ring if available.
        ///         If no next item available, std::nullopt is returned.
        /// @return 
        PopResult TryPop() {
            if(size_.load(std::memory_order_relaxed) == 0) return std::nullopt;
            if(!pop_mutex_.try_lock()) return std::nullopt;
            auto obj{std::move(ring_buffer_[Normalize(read_next_++)])};
            size_.fetch_sub(1, std::memory_order_acq_rel);
            pop_mutex_.unlock();
            return std::move(obj);
        }

    protected:
        /// @brief  Normalize the provided into to be in the range 0 .. capacity_-1
        /// @param index    The index to normalize
        /// @return     The normailized index
        auto Normalize(S index) const { return index % capacity_; }

        S read_next_{};         //!< Next location to read from
        S write_next_{};        //!< Next location to write to
        std::atomic<S> size_{}; //!< The number of elements queued
        S const capacity_;      //!< The size of the circular buffer
        mutex_type push_mutex_; //!< gatekeeper for push
        mutex_type pop_mutex_;  //!< gatekeeper for pop
        Buffer ring_buffer_;    //!< The ring buffer
    };
}

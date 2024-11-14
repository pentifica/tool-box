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
///
/// This code is based on the article https://rowjee.com/blog/skiplists
#include    <vector>
#include    <concepts>
#include    <type_traits>

namespace pentifica::tbox {
    template<typename K, typename V>
    concept SkipNodeArgs = requires(K, V) {
        requires std::is_move_constructible_v<K>;
        requires std::is_move_constructible_v<V>;
        requires std::is_default_constructible_v<K>;
        requires std::is_default_constructible_v<V>;
    };

    //  forward declarations
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    class SkipList;

    /// @brief  Node representative in the skip list
    /// @tparam K   The key type
    /// @tparam V   The value type  
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    class SkipListNode {
    public:
        friend class SkipList<K, V>;
        friend typename SkipList<K, V>::iterator;
        friend typename SkipList<K, V>::const_iterator;
        
        using key_type = K;
        using value_type = V;
        SkipListNode() = default;
        SkipListNode(SkipListNode const&) = default;
        SkipListNode(SkipListNode&&) = default;
        /// @brief  Prepare an instance for use
        /// @param  current_level   The level for the node (node level is zero-based)
        /// @param  key The node identifier
        /// @param  value   The node's value
        SkipListNode(int current_level, K key, V value)
            : current_level_(current_level)
            , key_(std::move(key))
            , value_(std::move(value))
            , links_(current_level + 1, nullptr)
        {
        }
        bool operator==(SkipListNode const& other) const {
            return key_ == other.key_
                && value_ == other.value_
                && current_level_ == other.current_level_
                && links_ == other.links_;
        }
        bool operator!=(SkipListNode const& other) const {
            return !operator==(other);
        }
        SkipListNode& operator=(SkipListNode const& other) = default;
        SkipListNode& operator=(SkipListNode&& other) = default;
        /// @brief Returns the level this node was added to the skip list aat
        /// @return     
        auto Level() const noexcept { return current_level_; }
        /// @brief  The node's key is returned
        /// @return 
        auto const& Key() const noexcept { return key_; }
        /// @brief Returns the node's value as a modifiable rreference
        /// @return 
        auto& Value() { return value_; }
        /// @brief  The node's value is returned
        /// @return 
        auto const& Value() const noexcept { return value_; }
        /// @brief  The links to the next nodes at level are returned
        /// @return
        auto const& Links() const noexcept { return links_; }

    private:
        /// @brief  The level the node was added to the dkip list
        int const current_level_{};
        /// @brief  The key associated with the node
        key_type const key_{};
        /// @brief  The value associated with the node
        value_type value_{};
        /// @brief  Links to the next nodes in the skip list levels
        std::vector<SkipListNode*> links_;
    };
}
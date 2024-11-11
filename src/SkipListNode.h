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
#include    <string>
#include    <format>
#include    <iostream>
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

    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    struct SkipListNode {
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

        int const current_level_{};
        K const key_{};
        V value_{};
        /// @brief  The links to other nodes
        std::vector<SkipListNode*> links_;
    };

    template<typename K, typename V>
    std::ostream&
    operator<<(std::ostream& os, SkipListNode<K, V> const& node) {
        os << std::format("SkipListNode[{}] [{}:{}] @ {}",
            node.current_level_, node.key_, node.value_, (void*)(&node))
            << std::endl;

        return os;            
    }
}
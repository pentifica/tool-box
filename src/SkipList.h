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
/// This code is based on:
///     https://rowjee.com/blog/skiplists
///     https://en.wikipedia.org/wiki/Skip_list#:~:text=The%20expected%20number,against%20storage%20costs.
#include    "SkipListNode.h"
#include    "SkipListError.h"

#include    <optional>
#include    <expected>
#include    <random>
#include    <memory>
#include    <functional>
#include    <vector>

namespace pentifica::tbox {
    /// @brief  Defines an iterator that does a forward traversal of the
    ///         SkipLisstNode(s) contained by a SkipLlist instance.
    /// @tparam SL  The SkipList type to iterate over
    template<typename SL>
    class SkipListIterator {
    public:
        using category = std::forward_iterator_tag;

        using container_type = SL;
        using container_ptr = container_type*;

        using value_type = typename SL::value_type;
        using reference = value_type&;
        using pointer = value_type*;

        /// @brief  Prepare an instance
        /// @param container 
        /// @param node 
        SkipListIterator(container_ptr container, pointer node) noexcept
            : container_(container)
            , node_(node)
        {}
        /// @brief  Prepare an instance from and existing
        /// @param other    The instance to use for initialization
        SkipListIterator(SkipListIterator const& other) noexcept = default;
        /// @brief  Cleanup
        ~SkipListIterator() = default;
        /// @brief  Pre-increment  advance
        /// @return The advanced iterator
        SkipListIterator& operator++() noexcept {
            if(node_ != container_->end_sentinel_) {
                node_ = node_->links_[0];
            }
            return *this;
        }
        /// @brief  Post-increment advance
        /// @param  
        /// @return 
        SkipListIterator operator++(int) noexcept {
            auto temp{*this};
            ++(*this);
            return temp;
        }
        bool operator==(SkipListIterator const& other) const noexcept {
            return node_ == other.node_;
        }
        bool operator!=(SkipListIterator const& other) const noexcept {
            return !operator==(other);
        }
        reference operator*() const noexcept { return *node_; }
        pointer operator->() const noexcept { return node_; }

    private:
        container_ptr container_{};
        pointer node_{};
    };
    /// @brief  Defines a skip list
    /// @tparam K   The key type
    /// @tparam V   The value type
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    class SkipList {
    public:
        using value_type = SkipListNode<K, V>;
        using iterator = SkipListIterator<SkipList<K, V>>;
        using const_iterator = SkipListIterator<const SkipList<K, V>>;
        friend iterator;

    public:
        /// @brief  Basic setup of an instance
        /// @param  max_level   Number of levels in the skiplist
        /// @param  gen_next_skip_level  Generates a skip level (0 .. max_level - 1)
        /// @note Levels are numbered from 0 .. max_level-1
        SkipList(int max_level, std::function<int(int)> gen_next_skip_level);
        /// @brief  Instance cleanup
        ~SkipList();

        /// @brief  Get the value associated with a particular key
        /// @param  key     The lookup key
        /// @return An optional value referencing the value associated with the key
        std::optional<V>
        Find(K const& key);

        /// @brief  Insert a key-value pair
        /// @param key 
        /// @param value 
        /// @return 
        V
        Insert(K const& key, V const& value);

        /// @brief Delete a key-value pair from the list
        /// @param key The key to delete
        /// @return 
        SkipListError::ErrorVariant
        Delete(K const& key);

        /// @brief Returns true if the list is empty.
        /// @return 
        bool Empty() const {
            return begin_sentinel_->links_[0] == end_sentinel_;
        }

        /// @brief  Returns the number of key/value pairs in the list
        /// @return
        auto Size() const { return count_; }

        /// @brief  Returns an iterator initialized to the start of the list
        /// @return
        iterator begin() { return iterator(this, begin_sentinel_->links_[0]); }

        /// @brief  Returns an iterator initialized to 1 past the end of the l;ist
        /// @return
        iterator end() { return iterator(this, end_sentinel_); }

        /// @brief  Returns a const iterator initialized o the start of the list
        /// @return
        const_iterator begin() const { return const_iterator(this, begin_sentinel_->links_[0]); }

        /// @briefReeeeturns a const iterator initialized to 1 past the end of the list
        const_iterator end() const { return const_iterator(this, end_sentinel_); }

    private:
        std::pair<value_type*, std::vector<value_type*>>
        IdentifyPredecessorNode(K const& key);

    public:
        size_t count_{};
        value_type* begin_sentinel_{};
        value_type* end_sentinel_{};
        const int max_level_{};
        std::function<int(int)> gen_next_skip_level_{};
    };

    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    SkipList<K, V>::SkipList(int max_level, std::function<int(int)> gen_next_skip_level)
        : max_level_(max_level)
        , gen_next_skip_level_(gen_next_skip_level)
    {
        begin_sentinel_ = new SkipListNode<K, V>(max_level_ - 1, {}, {});
        end_sentinel_ = new SkipListNode<K, V>(max_level_ - 1, {}, {});
    
        //  connect start and end nodes
        for(auto& link : begin_sentinel_->links_) {
            link = end_sentinel_;
        }
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    SkipList<K, V>::~SkipList() {
        for(auto node = begin_sentinel_->links_[0];
            node != end_sentinel_;
            node = begin_sentinel_->links_[0]) {
    
            //  move node links to begin_sentinel_
            for(int i = 0; i < max_level_; i++) {
                if(begin_sentinel_->links_[i] != node) {
                    break;
                }
                begin_sentinel_->links_[i] = node->links_[i];
            }
    
            delete node;
        }
    
        delete begin_sentinel_;
        delete end_sentinel_;
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    std::pair<SkipListNode<K, V>*, std::vector<SkipListNode<K, V>*>>
    SkipList<K, V>::IdentifyPredecessorNode(K const& key) {
        //  initialize the update vector
        auto update = std::vector<SkipListNode<K, V>*>(max_level_, nullptr);
    
        //  see if the node already exists
        auto current_node{begin_sentinel_};
    
        for(auto current_level = max_level_ - 1; current_level >= 0; current_level--) {
    
            //  check if the next node in the level has a key that comes
            //  before our search key
            auto next_node = current_node->links_[current_level];
            while(next_node != end_sentinel_ && next_node->key_ < key) {
                current_node = next_node;
                next_node = current_node->links_[current_level];
            }
    
            update[current_level] = current_node;
        }
    
        current_node = current_node->links_[0];
    
        return {current_node, update};
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    V
    SkipList<K, V>::Insert(K const& key, V const& value) {
    
        //  Figure out where to insert the node: This is either the node
        //  with the same key, so we can update the value, or we found
        //  the node right before the insertion point so that we can
        //  insert after it.
        auto [current_node, update] = IdentifyPredecessorNode(key);
    
        //  if the key exists at the curent node, update its value
        if(current_node->key_ == key) {
            current_node->value_ = value;
        }
    
        //  The key doesn't exist at the current node, insert it
        //  Update all pointers in the reachability chain to reach this node
        else {
            auto level = gen_next_skip_level_(max_level_);
            auto new_node = new SkipListNode<K, V>(level, key, value);
            for(int i = 0; i <= level; i++) {
                new_node->links_[i] = update[i]->links_[i];
                update[i]->links_[i] = new_node;
            }
            ++count_;
        }
    
        return value;
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    std::optional<V>
    SkipList<K, V>::Find(K const& key) {
        auto current{begin_sentinel_};
    
        for(auto search_level = max_level_ - 1; search_level >= 0; search_level--) {
            while(current->links_[search_level] != end_sentinel_
                && current->links_[search_level]->key_ < key) {
                current = current->links_[search_level];
            }
        }
    
        current = current->links_[0];
        if(current->key_ == key) {
            return current->value_;
        }
        else {
            return std::nullopt;
        }
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    SkipListError::ErrorVariant
    SkipList<K, V>::Delete(K const& key) {
        auto [node, update] = IdentifyPredecessorNode(key);
    
        //  if the node was found, update all necessary pointers
        //
        if(node->key_ == key) {
            for(int i = 0; i < max_level_; i++) {
                if(update[i]->links_[i] != node) {
                    break;
                }
                update[i]->links_[i] = node->links_[i];
            }
    
            delete node;
            --count_;
            return SkipListError::ErrorVariant::NOERR;
        }
    
        //  key not found
        return SkipListError::ErrorVariant::KEY_NOT_FOUND;
    }
}
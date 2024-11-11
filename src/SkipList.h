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
    //  forward references
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    class SkipList;

    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    std::ostream& operator<<(std::ostream&, SkipList<K, V> const&);

    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    class SkipList {
    public:
        /// @nrief  Basic setup of an instance
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
        Search(K const& key);

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

        /// @brief  Retrun a sorted list of the contents
        /// @return A sorted list of the contents. Possible an empty list
        std::vector<std::pair<K, V>>
        Scan();

        friend
        std::ostream& operator<< <> (std::ostream& os, SkipList<K, V> const& skip_list);

    private:
        std::pair<SkipListNode<K, V>*, std::vector<SkipListNode<K, V>*>>
        IdentifyPredecessorNode(K const& key);

    public:
        SkipListNode<K, V>* start_sentinel_{};
        SkipListNode<K, V>* end_sentinel_{};
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
        start_sentinel_ = new SkipListNode<K, V>(max_level_ - 1, {}, {});
        end_sentinel_ = new SkipListNode<K, V>(max_level_ - 1, {}, {});
    
        //  connect start and end nodes
        for(auto& link : start_sentinel_->links_) {
            link = end_sentinel_;
        }
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    SkipList<K, V>::~SkipList() {
        for(auto node = start_sentinel_->links_[0];
            node != end_sentinel_;
            node = start_sentinel_->links_[0]) {
    
            //  move node links to start_sentinel_
            for(int i = 0; i < max_level_; i++) {
                if(start_sentinel_->links_[i] != node) {
                    break;
                }
                start_sentinel_->links_[i] = node->links_[i];
            }
    
            delete node;
        }
    
        delete start_sentinel_;
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
        auto current_node{start_sentinel_};
    
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
            std::clog << std::format("BAZINGA! key {} found with value {}",
                current_node->key_, current_node->value_) << std::endl;
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
        }
    
        return value;
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    std::optional<V>
    SkipList<K, V>::Search(K const& key) {
        std::clog << "Searching for ---> " << key << std::endl;
        auto current{start_sentinel_};
    
        for(auto search_level = max_level_ - 1; search_level >= 0; search_level--) {
            while(current->links_[search_level] != end_sentinel_ && current->links_[search_level]->key_ < key) {
                std::clog << std::format("Exploring key ---> {}\n", current->links_[search_level]->key_);
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
            return SkipListError::ErrorVariant::NOERR;
        }
    
        //  key not found
        return SkipListError::ErrorVariant::KEY_NOT_FOUND;
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    std::vector<std::pair<K, V>>
    SkipList<K, V>::Scan() {
        std::vector<std::pair<std::string, std::string>> result{};
    
        auto node = start_sentinel_->links_[0];
        while(node != end_sentinel_) {
            result.emplace_back(std::make_pair(node->key_, node->value_));
            node = node->links_[0];
        }
    
        return result;
    }
    //  ------------------------------------------------------------------------
    //
    template<typename K, typename V>
    requires SkipNodeArgs<K, V>
    std::ostream&
    operator<<(std::ostream& os, SkipList<K, V> const& skip_list) {
        os << std::format("===== Printing SkipList") << std::endl;
    
        for(auto node = skip_list.start_sentinel_->links_[0];
            node != skip_list.end_sentinel_; 
            node = node->links_[0]) {
            os << *node;
        }
    
        os << std::format("===== END Printing SkipList") << std::endl;
    
        return os;
    }
}
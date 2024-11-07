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
#include    "SkipList.h"

#include    <ranges>

namespace pentifica::tbox {
SkipList::SkipList(int max_level, std::function<int(int)> gen_next_skip_level)
    : max_level_(max_level)
    , gen_next_skip_level_(gen_next_skip_level)
{
    start_ = new SkipListNode(max_level_ - 1, "START_KEY", "START_VALUE");
    end_ = new SkipListNode(max_level_ - 1, "END_KEY", "END_VALUE");

    //  connect start and end nodes
    for(auto& link : start_->links_) {
        link = end_;
    }
}

SkipList::~SkipList() {
    delete start_;
    delete end_;
}

std::pair<SkipListNode*, std::vector<SkipListNode*>>
SkipList::IdentifyPredecessorNode(std::string const& key) {
    //  initialize the update vector
    auto update = std::vector<SkipListNode*>(max_level_, nullptr);

    //  see if the node already exists
    auto current_node{start_};

    for(auto current_level = max_level_ - 1; current_level >= 0; current_level--) {

        //  check if the next node in the level has a key that comes
        //  before our search key
        auto next_node = current_node->links_[current_level];
        while(next_node != end_ && next_node->key_ < key) {
            current_node = next_node;
            next_node = current_node->links_[current_level];
        }

        update[current_level] = current_node;
    }

    current_node = current_node->links_[0];

    return std::make_pair(current_node, update);
}

std::string
SkipList::Insert(std::string const& key, std::string const& value) {

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
        auto new_node = new SkipListNode(level, key, value);
        for(int i = 0; i <= level; i++) {
            new_node->links_[i] = update[i]->links_[i];
            update[i]->links_[i] = new_node;
        }
    }

    return value;
}

std::optional<std::string>
SkipList::Search(std::string const& key) {
    std::clog << "Searching for ---> " << key << std::endl;
    auto current{start_};

    for(auto search_level = max_level_ - 1; search_level >= 0; search_level--) {
        while(current->links_[search_level] != end_ && current->links_[search_level]->key_ < key) {
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

SkipListError::ErrorVariant
SkipList::Delete(const std::string& key) {
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

std::vector<std::pair<std::string, std::string>>
SkipList::Scan() {
    std::vector<std::pair<std::string, std::string>> result{};

    auto node = start_->links_[0];
    while(node != end_) {
        result.emplace_back(std::make_pair(node->key_, node->value_));
        node = node->links_[0];
    }

    return result;
}

std::ostream&
operator<<(std::ostream& os, SkipList const& skip_list) {
    os << std::format("===== Printing SkipList") << std::endl;

    for(auto node = skip_list.start_; node != nullptr; node = node->links_[0]) {
        os << *node;
    }

    os << std::format("===== END Printing SkipList") << std::endl;

    return os;
}
}
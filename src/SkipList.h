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
#include    "SkipListNode.h"
#include    "SkipListError.h"

#include    <optional>
#include    <expected>
#include    <random>
#include    <memory>
#include    <functional>

namespace pentifica::tbox {
    //  forward references
    class SkipList;
    std::ostream& operator<<(std::ostream&, SkipList const&);

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
        std::optional<std::string>
        Search(std::string const& key);

        /// @brief  Insert a key-value pair
        /// @param key 
        /// @param value 
        /// @return 
        std::string
        Insert(std::string const& key, std::string const& value);

        /// @brief Delete a key-value pair from the list
        /// @param key The key to delete
        /// @return 
        SkipListError::ErrorVariant
        Delete(std::string const& key);

        /// @brief  Retrun a sorted list of the contents
        /// @return A sorted list of the contents. Possible an empty list
        std::vector<std::pair<std::string, std::string>>
        Scan();

        friend std::ostream& operator<<(std::iostream& os, SkipList const& skip_list);

    private:
        std::pair<SkipListNode*, std::vector<SkipListNode*>>
        IdentifyPredecessorNode(std::string const& key);

    public:
        SkipListNode* start_sentinel_{};
        SkipListNode* end_sentinel_{};
        const int max_level_{};
        std::function<int(int)> gen_next_skip_level_{};
    };
}
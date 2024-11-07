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
#include    <random>
#include    <functional>
#include    <ctime>

namespace pentifica::tbox{
    class SkipListLevelGenerator {
    public:
        SkipListLevelGenerator(double p_factor) :
            p_(p_factor)
        {
        }
        /// @brief  Generate the next value within the range 0 .. max_level-1
        /// @param max_level    The upper bound for the generated level
        /// @return The generated value
        int operator()(int max_level) {
            int level{1};
            while((level < max_level) && (distribution_(rng_) < p_)) {
                ++level;
            }
            return level - 1;
        }
    private:
        double const p_{};
        std::mt19937 rng_{std::mt19937(std::time(nullptr))};
        std::uniform_real_distribution<double>
            distribution_{std::uniform_real_distribution<double>(0.0, 1.0)};
    };
}

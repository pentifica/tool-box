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
#include <tuple>
#include <iostream>

namespace pentifica::tbox {
    /// @brief  Streams the tuple members
    /// @tparam TupleType   Type information
    /// @tparam ...Is   Indexes into the tuple
    /// @param os   Where to stream
    /// @param tp   The tuple to stream
    /// @param Is   Tuple indexes
    /// @return     The supplied stream
    template<typename TupleType, std::size_t... Is>
    std::ostream& PrintTuple(std::ostream& os, TupleType const& tp, std::index_sequence<Is...>) {
        auto print_field = [&os](const auto& x) { os << x; };
        (print_field(std::get<Is>(tp)), ...);
        return os;
    }
    /// @brief  Streams a tuple
    /// @tparam TupleType   The type information for the tuple
    /// @tparam TupleSize   Number of tuple members
    /// @param os   Where to stream the tuple
    /// @param tp   The tuple to stream
    /// @return     The supplied stream
    template<typename TupleType, std::size_t TupleSize = std::tuple_size<TupleType>::value>
    std::ostream& operator<<(std::ostream& os, TupleType const& tp) {
        return PrintTuple(os, tp, std::make_index_sequence<TupleSize>{});
    }
    /// @brief Implements RAII for an encapsulated set of actions. Can be either a functor
    ///        or lambda. The encapsulating object must support copy semantics.
    /// @tparam Action Type of encapsulation
    template<typename Action>
    class RAII {
    public:
        explicit RAII(Action action) : action_{action} {}
        ~RAII() { action_(); }
        RAII(RAII const&) = delete;
        RAII(RAII&&) = delete;
        RAII& operator=(RAII const&) = delete;
        RAII& operator=(RAII&&) = delete;
    private:
        Action action_;
    };
}
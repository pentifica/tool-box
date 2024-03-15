#include <bits/stdc++.h>

namespace pentifica::tbox {
    namespace internal {
        template<size_t N>
        struct fnv_hash_values;

        template<>
        struct fnv_hash_values<32> {
            static constexpr uint32_t base = 2166136261UL;
            static constexpr uint32_t prime = 16777619UL;
        };

        template<>
        struct fnv_hash_values<64> {
            static constexpr uint64_t base = 14695981039346656037ULL;
            static constexpr uint64_t prime = 1099511628211ULL;
        };

        template<typename ResultT>
        struct fnv_hash {
            static constexpr ResultT base_ = fnv_hash_values<CHAR_BIT * sizeof(ResultT)>::base;
            static constexpr ResultT prime_ = fnv_hash_values<CHAR_BIT * sizeof(ResultT)>::prime;

            static constexpr ResultT string_hash(const char* str, ResultT base = base_) {
                return str ? str_hash_impl(str[0], str + 1, base) : 0;
            }

            template<typename T>
            static constexpr ResultT hash(const T* p, size_t sz, ResultT base = base_) {
                return p ? hash_impl(static_cast<const char*>(p), sizeof(T) * sz, base) : 0;
            }

        protected:
            static constexpr ResultT str_hash_impl(char c, const char* remain, ResultT value) {
                return (c == 0)
                    ? value
                    : str_hash_impl(remain[0],
                        remain + 1,
                        static_cast<ResultT>(value ^ static_cast<ResultT>(c)) * prime_);
            }

            static constexpr ResultT hash_impl(const char* current, size_t remain, ResultT value) {
                return (remain == 0)
                    ? value
                    : hash_impl(current + 1,
                        remain - 1,
                        static_cast<ResultT>(value ^ static_cast<ResultT>(*current)) * prime_);
            }
        };

        template<typename ResultT>
        class string_hash {
        public:
            using hash_type = ResultT;
            using value_type = ResultT;
            using hasher = fnv_hash<ResultT>;

            constexpr string_hash() = default;
            constexpr explicit string_hash(hash_type value) : value_(value) {}
            constexpr string_hash(const char* str, size_t len) : value_(hasher::hash(str, len)) {}
            constexpr explicit string_hash(const char* str) : value_(hasher::string_hash(str)) {}
            inline explicit string_hash(const std::string_view& rhs) : value_(hasher::hash(rhs.data(), rhs.size())) {}
            inline explicit string_hash(const std::string& rhs) : value_(hasher::string_hash(rhs.c_str())) {}
            template<size_t N> constexpr explicit string_hash(char (&s)[N]) : value_(hasher::hash(s, N-1)) {}
            template<size_t N> constexpr explicit string_hash(const char (&s)[N]) : value_(hasher::hash(s, N-1)) {}

            [[nodiscard]] constexpr bool is_valid() const { return value_ != 0; }
            [[nodiscard]] constexpr ResultT value() const { return value_; }

            constexpr explicit operator bool() const { return is_valid(); }
            constexpr ResultT operator()() { return value_; }

        protected:
            hash_type value_;
        };

        using shash32 = string_hash<uint32_t>;
        using shash64 = string_hash<uint64_t>;
        using shash = string_hash<size_t>;

        template<typename ResultT>
        constexpr bool operator<(const string_hash<ResultT>& lhs, const string_hash<ResultT>& rhs) {
            return lhs.value() < rhs.value();
        }

        template<typename ResultT>
        constexpr bool operator>(const string_hash<ResultT>& lhs, const string_hash<ResultT>& rhs) {
            return lhs.value() > rhs.value();
        }

        template<typename ResultT>
        constexpr bool operator==(const string_hash<ResultT>& lhs, const string_hash<ResultT>& rhs) {
            return lhs.value() == rhs.value();
        }

        template<typename ResultT>
        constexpr bool operator!=(const string_hash<ResultT>& lhs, const string_hash<ResultT>& rhs) {
            return lhs.value() != rhs.value();
        }

        template<typename ResultT>
        constexpr bool operator<=(const string_hash<ResultT>& lhs, const string_hash<ResultT>& rhs) {
            return lhs.value() <= rhs.value();
        }

        template<typename ResultT>
        constexpr bool operator>=(const string_hash<ResultT>& lhs, const string_hash<ResultT>& rhs) {
            return lhs.value() >= rhs.value();
        }
        
    }

    namespace literals {
        constexpr internal::shash32::hash_type operator""_sh32(const char* str, size_t len) {
            return internal::shash32(str, len)();
        }

        constexpr internal::shash64::hash_type operator""_sh64(const char* str, size_t len) {
            return internal::shash64(str, len)();
        }

        constexpr internal::shash::hash_type operator""_sh(const char* str, size_t len) {
            return internal::shash(str, len)();
        }
    }

    using internal::string_hash;

    template<typename ResultT = size_t>
    ResultT hash(const char* str, size_t len) {
        return internal::string_hash<ResultT>(str)();
    }

    template<typename ResultT = size_t>
    ResultT hash(const std::string& str) {
        return internal::string_hash<ResultT>(str)();
    }
}
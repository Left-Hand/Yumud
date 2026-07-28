#pragma once

#include <string_view>
#include <cstddef>
#include <algorithm>
#include <array>
#include "core/string/utils/c_style/strnlen.hpp"
#include "core/utils/Option.hpp"

namespace ymd{


namespace str{
[[nodiscard]] static constexpr size_t
calc_thrifty_string_length(const char * buffer_end, size_t buffer_capacity) {
    if (buffer_end[-2] == 0) {
        return static_cast<unsigned char>(buffer_end[-1]);
    } else {
        if (buffer_end[-1] == 0) return buffer_capacity - 1;
        else return buffer_capacity;
    }
}

[[nodiscard]] static constexpr int
init_thrifty_string_from_sv(const char * src, size_t len, char * buffer_begin, size_t buffer_capacity) {
    // 错误条件：长度超限
    if (len > 255 || len > buffer_capacity) return -1;   // -1 表示失败
    
    if (len == buffer_capacity) {
        std::copy(src, src + len, buffer_begin);
    } else if (len + 1 == buffer_capacity) {
        std::copy(src, src + len, buffer_begin);
        buffer_begin[buffer_capacity - 1] = 0;
    } else {
        std::copy(src, src + len, buffer_begin);
        buffer_begin[buffer_capacity - 2] = 0;
        buffer_begin[buffer_capacity - 1] = static_cast<char>(len); // len <= 255
    }
    return static_cast<int>(len);   // 成功返回实际长度（可以为0）
}

}


template<size_t N>
struct alignas(4) [[nodiscard]] ThriftyInlineString final{
    using Self = ThriftyInlineString;

    static_assert(N >= 3);
    static_assert(N < 256);

    alignas(4) std::array<char, N> buffer;

    static constexpr Option<Self> try_from_sv(std::string_view str) noexcept {
        if (str.length() > N || str.length() > 255) return None;
        // 尾部约束检查
        if (str.length() == N) {
            if (str[N-2] == '\0' || str[N-1] == '\0') return None;
        } else if (str.length() == N-1) {
            if (str[N-2] == '\0') return None;
        }
        ThriftyInlineString self;
        if (std::is_constant_evaluated()){
            self.buffer.fill(0);
        }
        const auto len = str::init_thrifty_string_from_sv(str.data(), str.length(), self.buffer.data(), N);
        if (len < 0) return None;   // 初始化失败（实际上不会发生，因为预先检查了条件）
        return Some(self);
    }

    static constexpr Option<ThriftyInlineString> try_from_cstr(const char* cstr) noexcept {
        if (cstr == nullptr) return None;
        const size_t len = str::strnlen_from_left(cstr, N + 1);  // 最多读到第 N+1 个字符（索引 N）
        if (len == N + 1) return None;  // 长度 ≥ N+1，超出容量
        return try_from_sv(std::string_view(cstr, len));
    }

    [[nodiscard]] constexpr size_t length() const noexcept{
        return str::calc_thrifty_string_length(buffer.end(), N);
    }

    [[nodiscard]] constexpr const char * data() const noexcept {
        return buffer.data();
    }

    // [[nodiscard]] constexpr char operator [](const size_t idx) const noexcept{
    //     return data()[idx];
    // }

    // [[nodiscard]] constexpr char at(const size_t idx) const noexcept{
    //     if(idx >= N) __builtin_abort();
    //     return data()[idx];
    // }

    [[nodiscard]] constexpr std::string_view view() const noexcept {
        return std::string_view{data(), length()};
    }
};
}
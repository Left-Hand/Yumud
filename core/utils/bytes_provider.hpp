#pragma once

#include <cstdint>
#include <span>
#include "core/utils/bits_caster.hpp"

namespace ymd{

class BytesProvider;

template<std::endian E>
struct [[nodiscard]] FetchLeadingBytesCtorBitsProxy{
    BytesProvider & fetcher;

    template<typename T>

    [[nodiscard]] constexpr operator T() const;
};

struct [[nodiscard]] BytesProvider{
    explicit constexpr BytesProvider(std::span<const uint8_t> bytes) : 
        remaining_bytes_(bytes) {}

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_leading(){
        if(remaining_bytes_.size() < N) [[unlikely]]
            on_overflow();
        const auto ret = std::span<const uint8_t, N>(remaining_bytes_.data(), N);
        remaining_bytes_ = std::span<const uint8_t>(remaining_bytes_.data() + N, remaining_bytes_.size() - N);
        return ret;
    }

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_trailing(){
        if(remaining_bytes_.size() < N) [[unlikely]]
            on_overflow();
        const auto ret = std::span<const uint8_t, N>(std::prev(remaining_bytes_.end(), N), remaining_bytes_.end());
        remaining_bytes_ = std::span<const uint8_t>(remaining_bytes_.data(), remaining_bytes_.size() - N);
        return ret;
    }

    template <std::endian E>
    [[nodiscard]] constexpr FetchLeadingBytesCtorBitsProxy<E> fetch_leading_ctor_bits(){
        return FetchLeadingBytesCtorBitsProxy<E>(*this);
    }

    template <std::endian E>
    [[nodiscard]] constexpr FetchLeadingBytesCtorBitsProxy<E> fetch_trailing_ctor_bits(){
        return FetchLeadingBytesCtorBitsProxy<E>(*this);
    }

    [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
        return remaining_bytes_;
    }
private:
    std::span<const uint8_t> remaining_bytes_;

    constexpr void on_overflow(){
        __builtin_abort();
    }
};


template<std::endian E>
template<typename T>
constexpr FetchLeadingBytesCtorBitsProxy<E>::operator T() const {
    using D = from_bits_t<T>;
    static_assert(std::is_void_v<D> == false);
    static constexpr size_t Extents = sizeof(D);
    const D bits = bytes_to_int<E, D>(fetcher.fetch_leading<Extents>());
    return T(IntoBitsCtor<D>(bits));
}
}
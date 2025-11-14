#pragma once

#include <cstdint>
#include <span>
#include "core/utils/bits_caster.hpp"

namespace ymd{

class BytesProvider;

template<Endian::Kind E>
struct [[nodiscard]] FetchLeadingBytesCtorBitsProxy{
    BytesProvider & fetcher;

    template<typename T>

    [[nodiscard]] constexpr operator T() const;
};

struct [[nodiscard]] BytesProvider{
    explicit constexpr BytesProvider(std::span<const uint8_t> bytes) : 
        bytes_(bytes) {}

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_leading(){
        if(bytes_.size() < N) __builtin_abort();
        const auto ret = std::span<const uint8_t, N>(bytes_.data(), N);
        bytes_ = std::span<const uint8_t>(bytes_.data() + N, bytes_.size() - N);
        return ret;
    }

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_trailing(){
        if(bytes_.size() < N) __builtin_abort();
        const auto ret = std::span<const uint8_t, N>(std::prev(bytes_.end(), N), bytes_.end());
        bytes_ = std::span<const uint8_t>(bytes_.data(), bytes_.size() - N);
        return ret;
    }

    template <Endian::Kind E>
    [[nodiscard]] constexpr FetchLeadingBytesCtorBitsProxy<E> fetch_leading_ctor_bits(){
        return FetchLeadingBytesCtorBitsProxy<E>(*this);
    }

    template <Endian::Kind E>
    [[nodiscard]] constexpr FetchLeadingBytesCtorBitsProxy<E> fetch_trailing_ctor_bits(){
        return FetchLeadingBytesCtorBitsProxy<E>(*this);
    }

    [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
        return bytes_;
    }
private:
    std::span<const uint8_t> bytes_;
};


template<Endian::Kind E>
template<typename T>
constexpr FetchLeadingBytesCtorBitsProxy<E>::operator T() const {
    using D = from_bits_t<T>;
    static_assert(std::is_void_v<D> == false);
    static constexpr size_t Extents = sizeof(D);
    const D bits = bytes_to_int<E, D>(fetcher.fetch_leading<Extents>());
    return T(IntoBitsCtor<D>(bits));
}
}
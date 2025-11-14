#pragma once

#include <cstdint>
#include <span>
#include "core/utils/bits_caster.hpp"

namespace ymd{

class BytesSpawner;

template<Endian::Kind E>
struct [[nodiscard]] SpawnLeadingBytesCtorBitsProxy{
    BytesSpawner & spawner;

    template<typename T>

    [[nodiscard]] constexpr operator T() const;
};

struct [[nodiscard]] BytesSpawner{
    explicit constexpr BytesSpawner(std::span<const uint8_t> bytes) : 
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
    [[nodiscard]] constexpr SpawnLeadingBytesCtorBitsProxy<E> fetch_leading_ctor_bits(){
        return SpawnLeadingBytesCtorBitsProxy<E>(*this);
    }

    template <Endian::Kind E>
    [[nodiscard]] constexpr SpawnLeadingBytesCtorBitsProxy<E> fetch_trailing_ctor_bits(){
        return SpawnLeadingBytesCtorBitsProxy<E>(*this);
    }

    [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
        return bytes_;
    }
private:
    std::span<const uint8_t> bytes_;
};


template<Endian::Kind E>
template<typename T>
constexpr SpawnLeadingBytesCtorBitsProxy<E>::operator T() const {
    using D = bits_type_t<T>;
    static_assert(std::is_void_v<D> == false);
    static constexpr size_t Extents = sizeof(D);
    const D bits = bytes_to_int<E, D>(spawner.fetch_leading<Extents>());
    return T(IntoBitsCtor<D>(bits));
}
}
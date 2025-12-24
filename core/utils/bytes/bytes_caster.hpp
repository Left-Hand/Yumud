#pragma once

#include <cstdint>
#include <bit>
#include <span>

namespace ymd{

template<std::endian E, typename D, size_t Extent>
__attribute__((always_inline))
[[nodiscard]] static constexpr D bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    static_assert((Extent == sizeof(D)),
        "static extent must equal to sizeof(D)"
    );
    using UD = std::make_unsigned_t<D>;

    UD sum = 0;

    if constexpr (std::endian(E) == std::endian::big){
        #pragma GCC unroll 8
        for(size_t i = 0; i < Extent; i++){
            sum <<= 8;
            sum |= bytes[i];
        }
    }else{
        #pragma GCC unroll 8
        for(int i = Extent-1; i >= 0; i--){
            sum <<= 8;
            sum |= bytes[i];
        }
    }
    return std::bit_cast<D>(sum);
}

template<typename D, size_t Extent>
__attribute__((always_inline))
[[nodiscard]] static constexpr D le_bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    return bytes_to_int<std::endian::little, D, Extent>(bytes);
}

template<typename D, size_t Extent>
__attribute__((always_inline))
[[nodiscard]] static constexpr D be_bytes_to_int(const std::span<const uint8_t, Extent> bytes){
    return bytes_to_int<std::endian::big, D, Extent>(bytes);
}
}
#pragma once

#include "core/constants/enums.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/bytes/bytes_caster.hpp"
#include "core/tmp/bits/bits_conv.hpp"


namespace ymd{


//可以将整数完成bits构造
template<typename D>
struct [[nodiscard]] IntoBitsCtor{
    D bits;

    //no explicit
    template<typename T>
    requires (std::is_same_v<tmp::from_bits_t<T>, D>)
    [[nodiscard]] constexpr operator T() const {
        return tmp::obj_from_bits<T>(bits);
    }
};

template<typename D>
IntoBitsCtor(D) -> IntoBitsCtor<D>;



//可以将字节片段完成从bits构造
template<std::endian E, size_t Extent>
struct [[nodiscard]] BytesCtorBits{

    constexpr explicit BytesCtorBits(
        const std::span<const uint8_t, Extent> bytes
    ):  
        bytes_(bytes) {}

    //no explicit
    template<typename T, typename D = tmp::from_bits_t<T>>
    requires (Extent != std::dynamic_extent)
    [[nodiscard]] constexpr operator T() const {
        static_assert(Extent == sizeof(D), "Extent != sizeof(D)");
        const auto bits = bytes_to_int<E, D>(bytes_);
        return T(IntoBitsCtor<D>(bits));
    }

    
    //no explicit
    template<typename T, typename D = tmp::from_bits_t<T>>
    requires (Extent == std::dynamic_extent)
    [[deprecated("use try_into instead for better runtime safety")]]
    [[nodiscard]] constexpr operator T() const {
        if(Extent != sizeof(D)) [[unlikely]] __builtin_abort();
        const auto bits = bytes_to_int<E, D>(bytes_.template subspan<0, sizeof(D)>());
        return T(IntoBitsCtor<D>(bits));
    }

    template<typename T, typename D = tmp::from_bits_t<T>>
    [[nodiscard]] constexpr Option<T> try_into() const{
        if constexpr (Extent == std::dynamic_extent){
            if(bytes_.size() != sizeof(D)) [[unlikely]] return None;
        }
        const auto bits = bytes_to_int<E, T>(bytes_);
        return Some(IntoBitsCtor<T>(bits));
    }
private:
    std::span<const uint8_t, Extent> bytes_;
};

template<std::endian E, size_t Extents>
BytesCtorBits(std::span<const uint8_t, Extents>) -> BytesCtorBits<E, Extents>;


template<std::endian E, size_t Extents>
[[nodiscard]] static constexpr BytesCtorBits<E, Extents> 
make_bytes_ctor_bits_caster(std::span<const uint8_t, Extents> bytes) {
    return BytesCtorBits<E, Extents>(bytes);
}

template<size_t Extents>
[[nodiscard]] static constexpr BytesCtorBits<std::endian::little, Extents> 
le_bytes_ctor_bits(const std::span<const uint8_t, Extents> bytes){
    return BytesCtorBits<std::endian::little, Extents>(bytes);
}

template<size_t Extents>
[[nodiscard]] static constexpr BytesCtorBits<std::endian::big, Extents> 
be_bytes_ctor_bits(const std::span<const uint8_t, Extents> bytes){
    return BytesCtorBits<std::endian::big, Extents>(bytes);
}




}
#pragma once

#include <cstdint>
#include <array>
#include <span>

//描述了一个Ipv4地址

namespace ymd{
    class OutputStream;
}

namespace ymd{
struct [[nodsicard]] Ipv4{
    using Self = Ipv4;
    inline constexpr explicit Ipv4(
        const uint8_t b1,
        const uint8_t b2,
        const uint8_t b3,
        const uint8_t b4
    ): bytes_({b1, b2, b3, b4}){;}

    inline constexpr explicit Ipv4(
        const std::span<const uint8_t, 4> bytes
    ): Ipv4(bytes[0], bytes[1], bytes[2], bytes[3]){;}

    static constexpr Self from_u32(const uint32_t bits){return Self(bits);} 

    template<size_t I> requires(I < 4)
    [[nodiscard]] constexpr uint8_t get() const {return get_element<I>(*this);}

    template<size_t I> requires(I < 4)
    [[nodiscard]] constexpr uint8_t & get() {return get_element<I>(*this);}

    [[nodiscard]] constexpr uint8_t operator [](const size_t idx) const{return bytes_[idx];}

    [[nodiscard]] constexpr uint8_t & operator [](const size_t idx) {return bytes_[idx];}

    [[nodiscard]] constexpr uint8_t at(const size_t idx) const{
        if(idx >= 4) [[unlikely]] __builtin_abort();
        bytes_[idx];
    }

    [[nodiscard]] constexpr uint8_t & at(const size_t idx) {
        if(idx >= 4) [[unlikely]] __builtin_abort();
        bytes_[idx];
    }

    [[nodiscard]] constexpr std::span<const uint8_t, 4> bytes() const {return std::span(bytes_);}
    [[nodiscard]] constexpr std::span<uint8_t, 4> mut_bytes() {return std::span(bytes_);}

    [[nodiscard]] constexpr uint32_t as_u32() const {return std::bit_cast<uint32_t>(bytes_);}

    [[nodiscard]] constexpr bool operator ==(const Self & other){return as_u32() == other.as_u32();}
private:
    alignas(4) std::array<uint8_t, 4> bytes_;


    template<size_t I>
    requires(I < 4)
    [[nodiscard]] static constexpr uint8_t get_element(auto & self){
        return self.bytes_[I];
    }

    inline constexpr explicit Ipv4(const uint32_t bits){
        bytes_ = std::bit_cast<std::array<uint8_t, 4>>(bits);
    }
};
}
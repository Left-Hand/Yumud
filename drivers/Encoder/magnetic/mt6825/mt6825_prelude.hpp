#pragma once


#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/conn/spi/spidrv.hpp"

#include "drivers/encoder/encoder.hpp"


namespace ymd::drivers{


struct MT6825_Prelude { 
using Error = EncoderError;

template<typename T = void>
using IResult = Result<T, Error>;


enum class [[nodiscard]] RegAddr:uint16_t{
    UserId = 0x001,
    AngleHigh = 0x003,
    AngleLow = 0x004,
    Status = 0x005,
};

#pragma pack(push, 1)
struct [[nodiscard]] alignas(4) Packet final{
    using Self = Packet;
    union{
        struct{
            uint8_t angle_17_10;
            uint8_t pc1:1;
            uint8_t no_mag_warning:1;
            uint8_t angle_9_4:6;
            uint8_t __resv__:2;
            uint8_t pc2:1;
            uint8_t is_overspeed:1;
            uint8_t angle_3_0:4;
        };
        struct {
            uint16_t d1;
            uint8_t d2;
        };
    };
    //方便对齐到4字节
    uint8_t __padding__;

    [[nodiscard]] static constexpr Packet from_u24(uint32_t bits){                  
        return std::bit_cast<Self>(bits);
    }
    
    [[nodiscard]] static constexpr Packet from_bytes(
        const uint8_t b1, 
        const uint8_t b2, 
        const uint8_t b3
    ){
        return from_u24((static_cast<uint32_t>(b3) << 16) 
            | (static_cast<uint32_t>(b2) << 8) 
            | static_cast<uint32_t>(b1)
        );
    }

    [[nodiscard]] std::span<uint8_t, 3> as_bytes_mut() {
        return std::span<uint8_t, 3>(reinterpret_cast<uint8_t *>(this), sizeof(Self));
    }

    [[nodiscard]] std::span<const uint8_t> as_bytes() const noexcept {
        return std::span<const uint8_t, 3>(reinterpret_cast<const uint8_t *>(this), sizeof(Self));
    }

    IResult<Angular<uq32>> parse() const noexcept {
        return Ok(Angular<uq32>::from_turns(uq18::from_bits(angle_u18())));
    }

    [[nodiscard]] constexpr bool is_pc1_valid() const noexcept {
        const uint16_t bits = d1 & 0xfeff;
        const bool is_odd = (std::popcount(bits) & 0x01);
        return is_odd == pc1;
    }

    [[nodiscard]] constexpr bool is_pc2_valid() const noexcept {
        const uint8_t bits = d2 & 0xf8;
        const bool is_odd = (std::popcount(bits) & 0x01);
        return is_odd == pc2;
    }
private:
    [[nodiscard]] constexpr uint32_t angle_u18() const noexcept {
        return (angle_17_10 << 10) | (angle_9_4 << 4) | angle_3_0;
    }
};
#pragma pack(pop)
static_assert(sizeof(Packet) == 4);
};

}
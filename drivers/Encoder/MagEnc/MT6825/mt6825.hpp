#pragma once

//18位磁编码器
//已完成 已测试

//锐评：19年出品的依托答辩。从没见过回程差0.003圈 再低的spi速率校验位也频繁出错的编码器



#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/angle.hpp"

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"


namespace ymd::drivers{
struct MT6825_Prelude { 
using Error = EncoderError;

template<typename T = void>
using IResult = Result<T, Error>;


enum class RegAddr:uint16_t{
    UserId = 0x001,
    AngleHigh = 0x003,
    AngleLow = 0x004,
    Status = 0x005,
};

#pragma pack(push, 1)
struct Packet{
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
    uint8_t __padding__;

    [[nodiscard]] static constexpr Packet from_u24(uint32_t bits){                  
        Self ret;                
        ret.d1 = static_cast<uint16_t>(bits);               
        ret.d2 = static_cast<uint8_t>(bits >> 16);
        return ret;
    }
    
    [[nodiscard]] static constexpr Packet from_bytes(
        const uint8_t b1, 
        const uint8_t b2, 
        const uint8_t b3
    ){
        Self ret;
        ret.d1 = static_cast<uint16_t>(b2 << 8) | b1;
        ret.d2 = b3;
        return ret;
    }

    [[nodiscard]] std::span<uint8_t, 3> as_mut_bytes() {
        return std::span<uint8_t, 3>(reinterpret_cast<uint8_t *>(this), sizeof(Self));
    }

    [[nodiscard]] std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t, 3>(reinterpret_cast<const uint8_t *>(this), sizeof(Self));
    }

    [[nodiscard]] IResult<Angle<q31>> parse() const {
        
        // if(not is_overspeed) [[unlikely]] 
        //     return Err(Error::OverSpeed);
        // if(not is_pc1_valid()) [[unlikely]] 
        //     return Err(Error::InvalidPc);
        // if(not is_pc2_valid()) [[unlikely]] 
        //     return Err(Error::InvalidPc2);

        return Ok(Angle<q31>::from_turns(q18::from_i32(angle_u18())));
    }

    [[nodiscard]] constexpr bool is_pc1_valid() const {
        const uint16_t bits = d1 & 0xfeff;
        const bool is_odd = (std::popcount(bits) & 0x01);
        return is_odd == pc1;
    }

    [[nodiscard]] constexpr bool is_pc2_valid() const {
        const uint8_t bits = d2 & 0xf8;
        const bool is_odd = (std::popcount(bits) & 0x01);
        return is_odd == pc2;
    }
private:
    [[nodiscard]] constexpr uint32_t angle_u18() const {
        return (angle_17_10 << 10) | (angle_9_4 << 4) | angle_3_0;
    }
};
#pragma pack(pop)
static_assert(sizeof(Packet) == 4);
};


struct MT6825:
    public MT6825_Prelude
{
    explicit MT6825(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){}

    explicit MT6825(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){}

    explicit MT6825(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<Angle<q31>> get_lap_angle();
private:
    hal::SpiDrv spi_drv_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto address = T::ADDRESS;
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | std::bit_cast<uint8_t>(reg.as_val()));
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(const T & reg){
        const auto address = T::ADDRESS;
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | std::bit_cast<uint8_t>(reg.as_val()));
        uint16_t rx;
        if(const auto res = spi_drv_.transceive_single<uint16_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.as_ref() = rx;
        return Ok();
    }


    [[nodiscard]] IResult<Packet> read_packet();
};
}
#pragma once

//18位磁编码器
//群友强力推荐 据说1lsb都不抖

//未验证


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

    std::span<uint8_t, 3> as_mut_bytes() {
        return std::span<uint8_t, 3>(reinterpret_cast<uint8_t *>(this), sizeof(Self));
    }

    std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t, 3>(reinterpret_cast<const uint8_t *>(this), sizeof(Self));
    }

    [[nodiscard]] IResult<Angle<q31>> decode() const {
        if(not is_overspeed) [[unlikely]] 
            return Err(Error::OverSpeed);
        if(not is_pc1_valid()) [[unlikely]] 
            return Err(Error::InvalidPc);
        if(not is_pc2_valid()) [[unlikely]] 
            return Err(Error::InvalidPc);
        return Ok(Angle<q31>::from_turns(q18(angle_u18())>>18));
    }

    [[nodiscard]] constexpr bool is_pc1_valid() const {
        const uint16_t bits = d1 & 0xfffe;
        const bool is_even = std::popcount(bits) % 2 == 0;
        return is_even == pc1;
    }

    [[nodiscard]] constexpr bool is_pc2_valid() const {
        const uint8_t bits = d2 & 0xf8;
        const bool is_odd = std::popcount(bits) % 2 == 0;
        return is_odd == pc2;
    }
private:
    [[nodiscard]] uint32_t angle_u18() const {
        return (angle_17_10 << 10) | (angle_9_4 << 4) | angle_3_0;
    }
};
#pragma pack(pop)
static_assert(sizeof(Packet) == 3);
};


struct MT6825:
    public MT6825_Prelude
{
    explicit MT6825(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        spi_drv_(hal::SpiDrv(spi, index)){}

    explicit MT6825(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){}

    explicit MT6825(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<Angle<q31>> get_lap_angle(){
        if(const auto res = read_packet();
            res.is_err()) return Err(res.unwrap_err());
        else{
            return res.unwrap().decode();
        }
    }
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


    [[nodiscard]] IResult<Packet> read_packet(){
        #if 1
        static constexpr std::array<uint8_t, 4> tx = {0x83, 0x00, 0x00, 0x00};
        std::array<uint8_t, 4> rx;
        if(const auto res = spi_drv_.transceive_burst<uint8_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok(Packet::from_bytes(rx[1], rx[2], rx[3]));
        #else
        //exprimental
        static constexpr std::array<uint16_t, 2> tx = {0x8300, 0x0000};
        std::array<uint16_t, 2> rx;
        if(const auto res = spi_drv_.transceive_burst<uint16_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok(Packet::from_u24(std::bit_cast<uint32_t>(rx) >> 8));
        #endif
    }
};
}
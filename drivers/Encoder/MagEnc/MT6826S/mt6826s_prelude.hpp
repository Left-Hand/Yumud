#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/angle.hpp"

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"


namespace ymd::drivers{
struct MT6826S_Prelude { 
using Error = EncoderError;

template<typename T = void>
using IResult = Result<T, Error>;

// @brief MT6826S commands
enum class Command:uint16_t{
    ReadReg = 0b0011,
    WriteReg = 0b0110,
    BurnReg = 0b1100,
    ReadAngle = 0b1010,
};

enum class RegAddr:uint16_t{
    UserId = 0x001,
    AngleHigh = 0x003,
    AngleLow = 0x004,
    Status = 0x005,
};


enum class Hysteresis:uint8_t{
    LSB1, LSB2, LSB4, LSB8,
    LSB0, LSB0_25, LSB00_5
};

enum class ZeroPulseWidth{
    LSB1, LSB2, LSB4, LSB8,LSB12, LSB16, HALF
};

enum class PwmFreq{
    _994Hz,_497Hz
};

struct Status{
    uint8_t overspeed:1;
    uint8_t magweak:1;
    uint8_t uvlo:1;

    [[nodiscard]] bool any_fault() const {
        return std::bit_cast<uint8_t>(*this);
    }

    [[nodiscard]] EncoderError to_encoder_error() const {
        if(overspeed) return EncoderError::OverSpeed;
        if(magweak) return EncoderError::MagnetLow;
        if(uvlo) return EncoderError::UnderVoltage;
        __builtin_trap();
    }
};


struct Packet{
    uint16_t angle_msb_u15;
    Status status;
    uint8_t crc;

    std::span<uint8_t, 4> as_mut_bytes() {
        return std::span<uint8_t, 4>(reinterpret_cast<uint8_t *>(this), 4);
    }

    constexpr bool is_crc_valid() const {
        //TODO
        return true;
    }
};
static_assert(sizeof(Packet) == 4);

struct Header{
    RegAddr addr:12;
    Command cmd:4;

    constexpr uint16_t as_u16() const {
        return std::bit_cast<uint16_t>(*this);
    }
};
static_assert(sizeof(Header) == 2);
};


struct MT6826S:
    // public MagEncoderIntf,
    public MT6826S_Prelude
{

    explicit MT6826S(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        spi_drv_(hal::SpiDrv(spi, index)){}

    explicit MT6826S(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){}

    explicit MT6826S(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<Angle<q31>> get_lap_angle();
    [[nodiscard]] IResult<MagStatus> get_mag_status();
    // [[nodiscard]] IResult<void> update();


private:
    hal::SpiDrv spi_drv_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto address = T::ADDRESS;
        const uint8_t data = reg.as_val();
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | data);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_packet(Packet & packet){
        return Ok();
    }
};
}
#pragma once

#include <optional>

#include "core/io/regs.hpp"

#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct MT6701_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b000110);
    using Error = EncoderError;
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Hysteresis:uint8_t{
        LSB1, LSB2, LSB4, LSB8,
        LSB0, LSB0_25, LSB00_5
    };

    enum class ZeroPulseWidth{
        LSB1, LSB2, LSB4, LSB8,LSB12, LSB16, HALF
    };

    enum class PwmFreq{
        HZ994_4,HZ497_2
    };


    enum class RegAddr:uint8_t{
        RawAngle = 0x03,
        UVWMux = 0x25,
        ABZMux = 0x29,
        Resolution = 0x30,
        ZeroConfig = 0x32,
        Hystersis = 0x34,
        WireConfig = 0x38,
        StartStop = 0x3e,
        Start = 0x3f,
        Stop = 0x40
    };


    struct Semantic{
        union{
            struct{
                uint32_t crc:6;
                union{
                    struct{
                        uint32_t stat:2;
                        uint32_t pushed:1;
                        uint32_t overspd:1;
                    };
                    uint32_t mg:4;
                };
                uint16_t data_14bit:14;
            };
            struct{
                uint8_t data8;
                uint16_t data16;
            };
        };
        
        Semantic(const uint16_t data):data8(0), data16(data){;}

        Semantic(const uint8_t _data8, const uint16_t _data16):data8(_data8), data16(_data16){;}
        
        __inline constexpr IResult<> validate_fast() const {
            if(pushed) return Err(Error::MagnetHigh);
            if(overspd) return Err(Error::OverSpeed);
            return Ok();
        }

        __inline constexpr IResult<> validate() const {
            if(pushed) return Err(Error::MagnetHigh);
            if(overspd) return Err(Error::OverSpeed);
            if(is_crc_valid() == false) return Err(Error::InvalidCrc);
            return Ok();
        }

    private:
        __inline constexpr bool is_crc_valid() const{
            return true;//TODO
        }
    };

};

class MT6701_Phy:public MT6701_Prelude{
public:
    using RegAddr = MT6701_Phy::RegAddr;

    MT6701_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        MT6701_Phy(std::nullopt, hal::SpiDrv(spi, rank)){;}

    MT6701_Phy(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr):
        MT6701_Phy(hal::I2cDrv(i2c, addr), std::nullopt){;}

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){

        if(i2c_drv_){
            if(const auto res = i2c_drv_->write_reg(
                    std::bit_cast<uint8_t>(T::ADDRESS), 
                    reg.as_val(), 
                    MSB);
                res.is_err()) return Err(res.unwrap_err());
            reg.apply();
            return Ok();
        }else{
            return Err(details::EncoderError_Kind::SpiIsNotImplementedYet);
        }
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        if(i2c_drv_){
            if(const auto res = i2c_drv_->read_reg(
                    std::bit_cast<uint8_t>(T::ADDRESS), 
                    reg.as_ref(), 
                    MSB);
                res.is_err()) return Err(res.unwrap_err());
            return Ok();
        }else{
            return Err(details::EncoderError_Kind::SpiIsNotImplementedYet);
        }
    }

private:
    MT6701_Phy(
        std::optional<hal::I2cDrv> && i2c_drv, 
        std::optional<hal::SpiDrv> && spi_drv
    ):
        i2c_drv_(std::move(i2c_drv)),
        spi_drv_(std::move(spi_drv)){;}

    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
};

struct MT6701_Regs:public MT6701_Prelude{
    struct R16_RawAngle : public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::RawAngle;
        uint16_t angle;
    }DEF_R16(raw_angle_reg)

    struct R8_UVWMux : public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::UVWMux;
        uint8_t __resv__:7;
        uint8_t uvwMux:1;
    };

    struct R8_ABZMux : public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ABZMux;

        uint8_t __resv1__:1;
        uint8_t clockwise:1;
        uint8_t __resv2__:4;
        uint8_t abzMux:1;
        uint8_t __resv3__:1;
    };

    struct R16_Resolution : public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Resolution;
        uint16_t abzResolution:10;
        uint16_t __resv__:2;
        uint16_t poles:4;
    };

    struct R16_ZeroConfig : public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::ZeroConfig;
        uint16_t zeroPosition:12;
        uint16_t zeroPulseWidth:3;
        uint16_t hysteresis:1;
    };

    struct R8_Hystersis : public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Hystersis;
        uint8_t __resv__:6;
        uint8_t hysteresis:2;
    };

    struct R8_WireConfig : public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::WireConfig;
        uint8_t __resv__:5;
        uint8_t isPwm:1;
        uint8_t pwmPolarityLow:1;
        uint8_t pwmFreq:1;
    };

    struct R8_StartStop : public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::StartStop;
        uint8_t start:4;
        uint8_t stop:4;
    };

    struct R8_Start:public  Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Start;
        uint8_t data;
    };

    struct R8_Stop:public  Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Stop;
        uint8_t data;
    };

    R8_UVWMux uvw_mux_reg = {};
    R8_ABZMux abz_mux_reg = {};
    R16_Resolution resolution_reg = {};
    R16_ZeroConfig zero_config_reg = {};
    R8_Hystersis hystersis_reg = {};
    R8_WireConfig wire_config_reg = {};
    R8_StartStop start_stop_reg = {};
    R8_Start start_reg = {};
    R8_Stop stop_reg = {};


};


}

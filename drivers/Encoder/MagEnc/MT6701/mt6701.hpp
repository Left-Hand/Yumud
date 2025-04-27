#pragma once

#include <optional>

#include "core/io/regs.hpp"

#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class MT6701_Phy{
public:
    using Error = EncoderError;
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class RegAddress:uint8_t{
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


    MT6701_Phy(hal::Spi & spi, const hal::SpiSlaveIndex idx):
        MT6701_Phy(std::nullopt, std::make_optional<hal::SpiDrv>(spi, idx)){;}

    MT6701_Phy(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr):
        MT6701_Phy(std::make_optional<hal::I2cDrv>(i2c, addr), std::nullopt){;}


    IResult<> write_reg(const RegAddress addr, const uint16_t data);
    
    IResult<> read_reg(const RegAddress addr, uint16_t & data);
    
    IResult<> write_reg(const RegAddress addr, const uint8_t data);
    
    IResult<> read_reg(const RegAddress addr, uint8_t & data);
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


class MT6701 final :public MagEncoderIntf{
public:
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

protected:
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
        
        __inline bool valid(const bool fast_mode) const {
            bool valid = (pushed == false) and (overspd == false);
            if(fast_mode == false){
                return valid and crc_valid();
            }else{
                return valid;
            }
        }

    private:
        __inline bool crc_valid() const{
            return true;//TODO
        }
    };
    
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b000110);

    MT6701_Phy phy_;
    Semantic semantic = {0, 0};
    real_t lap_position = real_t(0);
    bool fast_mode = true;

    struct UVWMuxReg : public Reg8<>{
        uint8_t __resv__:7;
        uint8_t uvwMux:1;
    };

    struct ABZMuxReg : public Reg8<>{
        uint8_t __resv1__:1;
        uint8_t clockwise:1;
        uint8_t __resv2__:4;
        uint8_t abzMux:1;
        uint8_t __resv3__:1;
    };

    struct ResolutionReg : public Reg16<>{
        uint16_t abzResolution:10;
        uint16_t __resv__:2;
        uint16_t poles:4;
    };

    struct ZeroConfigReg : public Reg16<>{
        uint16_t zeroPosition:12;
        uint16_t zeroPulseWidth:3;
        uint16_t hysteresis:1;
    };

    struct HystersisReg : public Reg8<>{
        uint8_t __resv__:6;
        uint8_t hysteresis:2;
    };

    struct WireConfigReg : public Reg8<>{
        uint8_t __resv__:5;
        uint8_t isPwm:1;
        uint8_t pwmPolarityLow:1;
        uint8_t pwmFreq:1;
    };

    struct StartStopReg : public Reg8<>{
        uint8_t start:4;
        uint8_t stop:4;
    };

    uint16_t rawAngleData = {};
    UVWMuxReg uvwMuxReg = {};
    ABZMuxReg abzMuxReg = {};
    ResolutionReg resolutionReg = {};
    ZeroConfigReg zeroConfigReg = {};
    HystersisReg hystersisReg = {};
    WireConfigReg wireConfigReg = {};
    StartStopReg startStopReg = {};
    uint8_t startData = {};
    uint8_t stopData = {};

    using RegAddress = MT6701_Phy::RegAddress;
public:
    MT6701(MT6701_Phy && phy):
        phy_(std::move(phy)){;}

    MT6701(hal::I2c & i2c, hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(MT6701_Phy(i2c, addr)){;}


    ~MT6701(){};


    void init();

    IResult<> update();
    IResult<real_t> get_lap_position();
    
    IResult<bool> is_stable();

    IResult<> enable_uvwmux(const bool enable = true);

    IResult<> enable_abzmux(const bool enable = true);

    IResult<> set_direction(const bool clockwise);

    IResult<> set_poles(const uint8_t _poles);

    IResult<> set_abz_resolution(const uint16_t abzResolution);

    IResult<> set_zero_position(const uint16_t zeroPosition);

    IResult<> set_zero_pulse_width(const ZeroPulseWidth zeroPulseWidth);

    IResult<> set_hysteresis(const Hysteresis hysteresis);

    IResult<> enable_fast_mode(const bool en = true);

    IResult<> enable_pwm(const bool enable = true);

    IResult<> set_pwm_polarity(const bool polarity);

    IResult<> set_pwm_freq(const PwmFreq pwmFreq);

    IResult<> set_start(const real_t start);

    IResult<> set_stop(const real_t stop);
};

}

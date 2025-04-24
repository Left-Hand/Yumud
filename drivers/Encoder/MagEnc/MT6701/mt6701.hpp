#pragma once

#include <optional>

#include "core/io/regs.hpp"

#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{
class MT6701:public MagEncoderIntf{
public:
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

    std::optional<hal::I2cDrv> i2c_drv;
    std::optional<hal::SpiDrv> spi_drv;

    Semantic semantic = {0, 0};
    real_t lap_position = real_t(0);
    bool fast_mode = true;

    struct UVWMuxReg{
        uint8_t __resv__:7;
        uint8_t uvwMux:1;
    };

    struct ABZMuxReg{
        uint8_t __resv1__:1;
        uint8_t clockwise:1;
        uint8_t __resv2__:4;
        uint8_t abzMux:1;
        uint8_t __resv3__:1;
    };

    struct ResolutionReg{
        uint16_t abzResolution:10;
        uint16_t __resv__:2;
        uint16_t poles:4;
    };

    struct ZeroConfigReg{
        uint16_t zeroPosition:12;
        uint16_t zeroPulseWidth:3;
        uint16_t hysteresis:1;
    };

    struct HystersisReg{
        uint8_t __resv__:6;
        uint8_t hysteresis:2;
    };

    struct WireConfigReg{
        uint8_t __resv__:5;
        uint8_t isPwm:1;
        uint8_t pwmPolarityLow:1;
        uint8_t pwmFreq:1;
    };

    struct StartStopReg{
        uint8_t start:4;
        uint8_t stop:4;
    };

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

    struct{
        uint16_t rawAngleData;
        UVWMuxReg uvwMuxReg;
        ABZMuxReg abzMuxReg;
        ResolutionReg resolutionReg;
        ZeroConfigReg zeroConfigReg;
        HystersisReg hystersisReg;
        WireConfigReg wireConfigReg;
        StartStopReg startStopReg;
        uint8_t startData;
        uint8_t stopData;
    };


    
    BusError write_reg(const RegAddress addr, const uint16_t data);
    BusError read_reg(const RegAddress addr, uint16_t & data);
    BusError write_reg(const RegAddress addr, const uint8_t data);
    BusError read_reg(const RegAddress addr, uint8_t & data);
    // MT6701(hal::I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){};
public:
    MT6701(const hal::I2cDrv & _i2c_drv):
        i2c_drv(_i2c_drv), spi_drv(std::nullopt){};
    // MT6701(hal::I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){};
    MT6701(hal::I2c & _i2c):
        MT6701(hal::I2cDrv(_i2c, DEFAULT_I2C_ADDR)){};

    MT6701(const hal::SpiDrv & _spi_drv):
        i2c_drv(std::nullopt), spi_drv(_spi_drv){};
    // MT6701(hal::SpiDrv && _spi_drv):spi_drv(_spi_drv){};
    MT6701(hal::Spi & _spi, const hal::SpiSlaveIndex spi_index):
        MT6701(hal::SpiDrv(_spi, spi_index)){};
    ~MT6701(){};


    void init() override;

    void update();
    real_t get_lap_position() override;
    
    bool stable() override;

    void enable_uvwmux(const bool enable = true){
        uvwMuxReg.uvwMux = enable;
        write_reg(RegAddress::UVWMux, std::bit_cast<uint8_t>(uvwMuxReg));
    }

    void enable_abzmux(const bool enable = true){
        abzMuxReg.abzMux = enable;
        write_reg(RegAddress::ABZMux, std::bit_cast<uint8_t>(abzMuxReg));
    }

    void set_direction(const bool clockwise){
        abzMuxReg.clockwise = clockwise;
        write_reg(RegAddress::ABZMux, std::bit_cast<uint8_t>(abzMuxReg));
    }

    void set_poles(const uint8_t _poles){
        resolutionReg.poles = _poles;
        write_reg(RegAddress::Resolution, std::bit_cast<uint16_t>(resolutionReg));
    }

    void set_abz_resolution(const uint16_t abzResolution){
        resolutionReg.abzResolution = abzResolution;
        write_reg(RegAddress::Resolution, std::bit_cast<uint16_t>(resolutionReg));
    }

    void set_zero_position(const uint16_t zeroPosition){
        zeroConfigReg.zeroPosition = zeroPosition;
        write_reg(RegAddress::ZeroConfig, std::bit_cast<uint16_t>(zeroConfigReg));
    }

    void set_zero_pulse_width(const ZeroPulseWidth zeroPulseWidth){
        zeroConfigReg.zeroPulseWidth = (uint8_t)zeroPulseWidth;
        write_reg(RegAddress::ZeroConfig, std::bit_cast<uint16_t>(zeroConfigReg));
    }

    void set_hysteresis(const Hysteresis hysteresis){
        hystersisReg.hysteresis = (uint8_t)hysteresis & 0b11;
        zeroConfigReg.hysteresis = (uint8_t)hysteresis >> 2;
        write_reg(RegAddress::Hystersis, std::bit_cast<uint8_t>(hystersisReg));
        write_reg(RegAddress::ZeroConfig, std::bit_cast<uint16_t>(zeroConfigReg));
    }

    void enable_fast_mode(const bool en = true){
        fast_mode = en;
    }
    void enable_pwm(const bool enable = true){
        wireConfigReg.isPwm = enable;
        write_reg(RegAddress::WireConfig, std::bit_cast<uint8_t>(wireConfigReg));
    }

    void set_pwm_polarity(const bool polarity){
        wireConfigReg.pwmPolarityLow = !polarity;
        write_reg(RegAddress::WireConfig, std::bit_cast<uint8_t>(wireConfigReg));
    }

    void set_pwm_freq(const PwmFreq pwmFreq){
        wireConfigReg.pwmFreq = (uint8_t)pwmFreq;
        write_reg(RegAddress::WireConfig, std::bit_cast<uint8_t>(wireConfigReg));
    }

    void set_start(const real_t start){
        uint16_t _startData = uni_to_u16(start);
        _startData >>= 4;
        startData = _startData;
        startStopReg.start = _startData >> 8;
        write_reg(RegAddress::Start, startData);
        write_reg(RegAddress::StartStop, std::bit_cast<uint8_t>(startStopReg));
    }

    void set_stop(const real_t stop){
        uint16_t _stopData = uni_to_u16(stop);
        _stopData >>= 4;
        stopData = _stopData;
        startStopReg.stop = _stopData >> 8;
        write_reg(RegAddress::Stop, stopData);
        write_reg(RegAddress::StartStop, std::bit_cast<uint8_t>(startStopReg));
    }
};

}

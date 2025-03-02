#pragma once

#include "drivers/device_defs.h"


#include "drivers/Encoder/MagEncoder.hpp"

#ifdef MT6701_DEBUG
#undef MT6701_DEBUG
#define MT6701_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__)
#else
#define MT6701_DEBUG(...)
#endif



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
    
    scexpr uint8_t default_i2c_addr = 0b000'110'0;

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


    
    void writeReg(const RegAddress addr, const uint16_t data);
    void readReg(const RegAddress addr, uint16_t & data);
    void writeReg(const RegAddress addr, const uint8_t data);
    void readReg(const RegAddress addr, uint8_t & data);
public:
    MT6701(hal::I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){};
    MT6701(hal::I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){};
    MT6701(hal::I2c & _i2c):i2c_drv(hal::I2cDrv(_i2c, default_i2c_addr)){};

    MT6701(hal::SpiDrv & _spi_drv):spi_drv(_spi_drv){};
    MT6701(hal::SpiDrv && _spi_drv):spi_drv(_spi_drv){};
    MT6701(hal::Spi & _spi, const uint8_t spi_index):spi_drv(hal::SpiDrv(_spi, spi_index)){};
    ~MT6701(){};


    void init() override;

    void update();
    real_t getLapPosition() override;
    
    bool stable() override;

    void enableUVWMUX(const bool enable = true){
        uvwMuxReg.uvwMux = enable;
        writeReg(RegAddress::UVWMux, std::bit_cast<uint8_t>(uvwMuxReg));
    }

    void enableABZMUX(const bool enable = true){
        abzMuxReg.abzMux = enable;
        writeReg(RegAddress::ABZMux, std::bit_cast<uint8_t>(abzMuxReg));
    }

    void setDirection(const bool clockwise){
        abzMuxReg.clockwise = clockwise;
        writeReg(RegAddress::ABZMux, std::bit_cast<uint8_t>(abzMuxReg));
    }

    void setPoles(const uint8_t _poles){
        resolutionReg.poles = _poles;
        writeReg(RegAddress::Resolution, std::bit_cast<uint16_t>(resolutionReg));
    }

    void setABZResolution(const uint16_t abzResolution){
        resolutionReg.abzResolution = abzResolution;
        writeReg(RegAddress::Resolution, std::bit_cast<uint16_t>(resolutionReg));
    }

    void setZeroPosition(const uint16_t zeroPosition){
        zeroConfigReg.zeroPosition = zeroPosition;
        writeReg(RegAddress::ZeroConfig, std::bit_cast<uint16_t>(zeroConfigReg));
    }

    void setZeroPulseWidth(const ZeroPulseWidth zeroPulseWidth){
        zeroConfigReg.zeroPulseWidth = (uint8_t)zeroPulseWidth;
        writeReg(RegAddress::ZeroConfig, std::bit_cast<uint16_t>(zeroConfigReg));
    }

    void setHysteresis(const Hysteresis hysteresis){
        hystersisReg.hysteresis = (uint8_t)hysteresis & 0b11;
        zeroConfigReg.hysteresis = (uint8_t)hysteresis >> 2;
        writeReg(RegAddress::Hystersis, std::bit_cast<uint8_t>(hystersisReg));
        writeReg(RegAddress::ZeroConfig, std::bit_cast<uint16_t>(zeroConfigReg));
    }

    void enableFastMode(const bool en = true){
        fast_mode = en;
    }
    void enablePwm(const bool enable = true){
        wireConfigReg.isPwm = enable;
        writeReg(RegAddress::WireConfig, std::bit_cast<uint8_t>(wireConfigReg));
    }

    void setPwmPolarity(const bool polarity){
        wireConfigReg.pwmPolarityLow = !polarity;
        writeReg(RegAddress::WireConfig, std::bit_cast<uint8_t>(wireConfigReg));
    }

    void setPwmFreq(const PwmFreq pwmFreq){
        wireConfigReg.pwmFreq = (uint8_t)pwmFreq;
        writeReg(RegAddress::WireConfig, std::bit_cast<uint8_t>(wireConfigReg));
    }

    void setStart(const real_t start){
        uint16_t _startData = uni_to_u16(start);
        _startData >>= 4;
        startData = _startData;
        startStopReg.start = _startData >> 8;
        writeReg(RegAddress::Start, startData);
        writeReg(RegAddress::StartStop, std::bit_cast<uint8_t>(startStopReg));
    }

    void setStop(const real_t stop){
        uint16_t _stopData = uni_to_u16(stop);
        _stopData >>= 4;
        stopData = _stopData;
        startStopReg.stop = _stopData >> 8;
        writeReg(RegAddress::Stop, stopData);
        writeReg(RegAddress::StartStop, std::bit_cast<uint8_t>(startStopReg));
    }
};

}

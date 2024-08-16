#ifndef __MT6701_HPP__
#define __MT6701_HPP__

#include "hal/bus/i2c/i2cdrv.hpp"
#include "../types/real.hpp"
#include <optional>

class MT6701{
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
    static constexpr uint8_t default_i2c_addr = 0b000'110'0;

    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    struct UVWMuxReg{
        REG8_BEGIN
        uint8_t __resv__:7;
        uint8_t uvwMux:1;
        REG8_END
    };

    struct ABZMuxReg{
        REG8_BEGIN
        uint8_t __resv1__:1;
        uint8_t clockwise:1;
        uint8_t __resv2__:4;
        uint8_t abzMux:1;
        uint8_t __resv3__:1;
        REG8_END
    };

    struct ResolutionReg{
        REG16_BEGIN
        uint16_t abzResolution:10;
        uint16_t __resv__:2;
        uint16_t poles:4;
        REG16_END
    };

    struct ZeroConfigReg{
        REG16_BEGIN
        uint16_t zeroPosition:12;
        uint16_t zeroPulseWidth:3;
        uint16_t hysteresis:1;
        REG16_END
    };

    struct HystersisReg{
        REG8_BEGIN
        uint8_t __resv__:6;
        uint8_t hysteresis:2;
        REG8_END
    };


    struct WireConfigReg{
        REG8_BEGIN
        uint8_t __resv__:5;
        uint8_t isPwm:1;
        uint8_t pwmPolarityLow:1;
        uint8_t pwmFreq:1;
        REG8_END
    };

    struct StartStopReg{
        REG8_BEGIN
        uint8_t start:4;
        uint8_t stop:4;
        REG8_END
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

    void writeReg(const RegAddress regAddress, const uint16_t regData){
        if(i2c_drv) i2c_drv->writeReg((uint8_t)regAddress, regData);
    }

    void readReg(const RegAddress regAddress, uint16_t & regData){
        if(i2c_drv) i2c_drv->readReg((uint8_t)regAddress, regData);
    }

    void writeReg(const RegAddress regAddress, const uint8_t regData){
        if(i2c_drv) i2c_drv->writeReg((uint8_t)regAddress, regData);
    }

    void readReg(const RegAddress regAddress, uint8_t & regData){
        if(i2c_drv) i2c_drv->readReg((uint8_t)regAddress, regData);
    }
public:
    MT6701(I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){};
    MT6701(I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){};
    MT6701(I2c & _i2c):i2c_drv(I2cDrv(_i2c, default_i2c_addr)){};

    MT6701(SpiDrv & _spi_drv):spi_drv(_spi_drv){};
    MT6701(SpiDrv && _spi_drv):spi_drv(_spi_drv){};
    MT6701(Spi & _spi, const uint8_t spi_index):spi_drv(SpiDrv(_spi, spi_index)){};
    ~MT6701(){};

    real_t getRawPosition(){
        readReg(RegAddress::RawAngle, rawAngleData);
        real_t ret;
        u16_to_uni(rawAngleData, ret);
        return ret;
    }

    void enableUVWMUX(const bool enable = true){
        uvwMuxReg.uvwMux = enable;
        writeReg(RegAddress::UVWMux, uvwMuxReg.data);
    }

    void enableABZMUX(const bool enable = true){
        abzMuxReg.abzMux = enable;
        writeReg(RegAddress::ABZMux, abzMuxReg.data);
    }

    void setDirection(const bool clockwise){
        abzMuxReg.clockwise = clockwise;
        writeReg(RegAddress::ABZMux, abzMuxReg.data);
    }

    void setPoles(const uint8_t _poles){
        resolutionReg.poles = _poles;
        writeReg(RegAddress::Resolution, resolutionReg.data);
    }

    void setABZResolution(const uint16_t abzResolution){
        resolutionReg.abzResolution = abzResolution;
        writeReg(RegAddress::Resolution, resolutionReg.data);
    }

    void setZeroPosition(const uint16_t zeroPosition){
        zeroConfigReg.zeroPosition = zeroPosition;
        writeReg(RegAddress::ZeroConfig, zeroConfigReg.data);
    }

    void setZeroPulseWidth(const ZeroPulseWidth zeroPulseWidth){
        zeroConfigReg.zeroPulseWidth = (uint8_t)zeroPulseWidth;
        writeReg(RegAddress::ZeroConfig, zeroConfigReg.data);
    }

    void setHysteresis(const Hysteresis hysteresis){
        hystersisReg.hysteresis = (uint8_t)hysteresis & 0b11;
        zeroConfigReg.hysteresis = (uint8_t)hysteresis >> 2;
        writeReg(RegAddress::Hystersis, hystersisReg.data);
        writeReg(RegAddress::ZeroConfig, zeroConfigReg.data);
    }

    void enablePwm(const bool enable = true){
        wireConfigReg.isPwm = enable;
        writeReg(RegAddress::WireConfig, wireConfigReg.data);
    }

    void setPwmPolarity(const bool polarity){
        wireConfigReg.pwmPolarityLow = !polarity;
        writeReg(RegAddress::WireConfig, wireConfigReg.data);
    }

    void setPwmFreq(const PwmFreq pwmFreq){
        wireConfigReg.pwmFreq = (uint8_t)pwmFreq;
        writeReg(RegAddress::WireConfig, wireConfigReg.data);
    }

    void setStart(const real_t start){
        uint16_t _startData;
        uni_to_u16(start, _startData);
        _startData >>= 4;
        startData = _startData;
        startStopReg.start = _startData >> 8;
        writeReg(RegAddress::Start, startData);
        writeReg(RegAddress::StartStop, startStopReg.data);
    }

    void setStop(const real_t stop){
        uint16_t _stopData;
        uni_to_u16(stop, _stopData);
        _stopData >>= 4;
        stopData = _stopData;
        startStopReg.stop = _stopData >> 8;
        writeReg(RegAddress::Stop, stopData);
        writeReg(RegAddress::StartStop, startStopReg.data);
    }

    void init(){
        enablePwm();
        setPwmPolarity(true);
        setPwmFreq(PwmFreq::HZ497_2);
    }

};

#endif
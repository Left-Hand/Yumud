#pragma once

#include "drivers/device_defs.h"

namespace ymd::drivers{
class INA226 {
public:

    enum class AverageTimes:uint8_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint8_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };

protected:
    hal::I2cDrv i2c_drv;
    
    real_t current_lsb_ma = real_t(0.2);
    scexpr real_t voltage_lsb_mv = real_t(1.25);

    enum class RegAddress:uint8_t{
        Config = 0x00,
        shuntVoltage = 0x01,
        busVoltage = 0x02,
        power = 0x03,
        current = 0x04,
        calibration = 0x05,
        mask = 0x06,
        alertLimit = 0x07,
        manufactureID = 0xFE,
        chipID = 0xFF,
    };

    struct ConfigReg:public Reg16{
        uint16_t shuntVoltageEnable :1;
        uint16_t busVoltageEnable :1;
        uint16_t continuos :1;
        uint16_t shuntVoltageConversionTime:3;
        uint16_t busVoltageConversionTime:3;
        uint16_t averageMode:3;
        uint16_t __resv__:3;
        uint16_t rst:1;
    };

    struct MaskReg:public Reg16{
        uint16_t alertLatchEnable:1;
        uint16_t alertPolarity:1;
        uint16_t mathOverflow:1;
        uint16_t convReadyFlag:1;
        uint16_t alertFlag:1;
        uint16_t __resv__:5;
        uint16_t convReady:1;
        uint16_t powerOverlimit:1;
        uint16_t busUnderVoltage:1;
        uint16_t busOverVoltage:1;
        uint16_t shuntUnderVoltage:1;
        uint16_t shuntOverVoltage:1;
    };

    ConfigReg configReg;
    uint16_t shuntVoltageReg;
    uint16_t busVoltageReg;
    uint16_t powerReg;
    int16_t currentReg;
    int16_t calibrationReg;
    MaskReg maskReg;
    uint16_t alertLimitReg;
    uint16_t manufactureIDReg;
    uint16_t chipIDReg;

    BusError writeReg(const RegAddress addr, const uint16_t data);

    BusError readReg(const RegAddress addr, uint16_t & data);

    BusError requestPool(const RegAddress addr, uint16_t * data_ptr, const size_t len);

    class CurrentChannel;
    class VoltageChannel;

    friend class CurrentChannel;
    friend class VoltageChannel;

    class INA226Channel:public hal::AnalogInIntf{
    public:
        enum class Index:uint8_t{
            SHUNT_VOLT,
            BUS_VOLT,
            CURRENT,
            POWER
        };

    protected:
        INA226 & parent_;
        Index ch_;
    public:
        INA226Channel(INA226 & _parent, const Index _ch):parent_(_parent), ch_(_ch){}

        INA226Channel(const INA226Channel & other) = delete;
        INA226Channel(INA226Channel && other) = delete;
        operator real_t() override{
            switch(ch_){
                case Index::SHUNT_VOLT:
                    return parent_.getShuntVoltage();
                case Index::BUS_VOLT:
                    return parent_.getVoltage();
                case Index::CURRENT:
                    return parent_.getCurrent();
                case Index::POWER:
                    return parent_.getPower();
                default:
                    return real_t(0);
            }
        }
    };


    std::array<INA226Channel, 4> channels;
public:
    using Index = INA226Channel::Index;
    
    scexpr uint8_t default_i2c_addr = 0x80;

    #define CHANNEL_CONTENT\
        INA226Channel{*this, INA226Channel::Index::SHUNT_VOLT},\
        INA226Channel{*this, INA226Channel::Index::BUS_VOLT},\
        INA226Channel{*this, INA226Channel::Index::CURRENT},\
        INA226Channel{*this, INA226Channel::Index::POWER}\

    INA226(const hal::I2cDrv & _i2c_drv):i2c_drv(_i2c_drv), channels{CHANNEL_CONTENT}{;}
    INA226(hal::I2cDrv && _i2c_drv):i2c_drv(_i2c_drv), channels{CHANNEL_CONTENT}{;}
    INA226(hal::I2c & _i2c, const uint8_t _addr = default_i2c_addr):i2c_drv(hal::I2cDrv(_i2c, _addr)), channels{CHANNEL_CONTENT}{};

    #undef CHANNEL_CONTENT


    void update();

    void init(const uint mohms, const uint max_current_a);

    void config(const uint mohms, const uint max_current_a);

    void setAverageTimes(const uint16_t times);

    bool verify();

    auto & ch(const Index index){
        return channels[uint8_t(index)];
    }

    auto & currChannel(){
        return ch(INA226Channel::Index::CURRENT);
    }

    auto & voltChannel(){
        return ch(INA226Channel::Index::BUS_VOLT);
    }


    real_t getVoltage(){
        return busVoltageReg * voltage_lsb_mv / 1000;
    }

    int getShuntVoltageuV(){
        return((shuntVoltageReg << 1) + (shuntVoltageReg >> 1));
    }

    real_t getShuntVoltage(){
        auto uv = getShuntVoltageuV();
        return real_t(uv / 100) / 10000;
    }

    real_t getCurrent(){
        return currentReg * current_lsb_ma / 1000;
    }

    real_t getPower(){
        return powerReg * current_lsb_ma / 40;
    }

    void setAverageTimes(const AverageTimes times){
        configReg.averageMode = uint8_t(times);
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void setBusConversionTime(const ConversionTime time){
        configReg.busVoltageConversionTime = uint8_t(time);
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void setShuntConversionTime(const ConversionTime time){
        configReg.shuntVoltageConversionTime = uint8_t(time);
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void reset(){
        configReg.rst = 1;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
        configReg.rst = 0;
    }

    void enableShuntVoltageMeasure(const bool en = true){
        configReg.shuntVoltageEnable = en;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void enableBusVoltageMeasure(const bool en = true){
        configReg.busVoltageEnable = en;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void enableContinuousMeasure(const bool en = true){
        configReg.continuos = en;
        writeReg(RegAddress::Config, std::bit_cast<uint16_t>(configReg));
    }

    void enableAlertLatch(const bool en = true){
        maskReg.alertLatchEnable = en;
        writeReg(RegAddress::mask, std::bit_cast<uint16_t>(maskReg));
    }
};

}
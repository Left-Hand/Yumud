#pragma once

#include "drivers/device_defs.h"

#ifdef INA226_DEBUG
#undef INA226_DEBUG
#define INA226_DEBUG(...) DEBUG_LOG(...)
#else 
#define INA226_DEBUG(...)
#endif

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
    I2cDrv i2c_drv;
    
    real_t currentLsb = real_t(0.0002);
    real_t voltageLsb = real_t(0.00125);

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

    void writeReg(const RegAddress addr, const uint16_t data){
        i2c_drv.writeReg((uint8_t)addr, data, MSB);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        i2c_drv.readReg((uint8_t)addr, data, MSB);
    }

    void requestPool(const RegAddress addr, void * data_ptr, const size_t len){
        i2c_drv.readMulti((uint8_t)addr, (uint16_t *)data_ptr, len, LSB);
    }

    class CurrentChannel;
    class VoltageChannel;

    friend class CurrentChannel;
    friend class VoltageChannel;

    class INA226Channel:public AnalogInChannel{
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
                    return 0;
            }
        }
    };


    std::array<INA226Channel, 4> channels;
public:
    using Index = INA226Channel::Index;
    
    scexpr uint8_t default_i2c_addr = 0x80;

    #define CHANNEL_CONTEX\
        INA226Channel{*this, INA226Channel::Index::SHUNT_VOLT},\
        INA226Channel{*this, INA226Channel::Index::BUS_VOLT},\
        INA226Channel{*this, INA226Channel::Index::CURRENT},\
        INA226Channel{*this, INA226Channel::Index::POWER}\

    INA226(const I2cDrv & _i2c_drv):i2c_drv(_i2c_drv), channels{CHANNEL_CONTEX}{;}
    INA226(I2cDrv && _i2c_drv):i2c_drv(_i2c_drv), channels{CHANNEL_CONTEX}{;}
    INA226(I2c & _i2c, const uint8_t _addr = default_i2c_addr):i2c_drv(I2cDrv(_i2c, _addr)), channels{CHANNEL_CONTEX}{};

    #undef CHANNEL_CONTEX

    auto & ch(const Index index){
        return channels[uint8_t(index)];
    }

    void update();

    void init(const real_t ohms, const real_t max_current_a);

    void config(const real_t ohms, const real_t max_current_a);

    void setAverageTimes(const uint16_t times);

    bool verify();

    real_t getVoltage(){
        return busVoltageReg * voltageLsb;
    }

    int getShuntVoltageuV(){
        return((shuntVoltageReg << 1) + (shuntVoltageReg >> 1));
    }

    real_t getShuntVoltage(){
        auto uv = getShuntVoltageuV();
        return real_t(uv / 100) / 10000;
    }

    real_t getCurrent(){
        return currentReg * currentLsb;
    }

    real_t getPower(){
        return powerReg * currentLsb * 25;
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
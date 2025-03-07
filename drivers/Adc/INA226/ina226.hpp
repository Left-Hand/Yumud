#pragma once

#include "drivers/device_defs.h"
#include "sys/utils/rustlike/Result.hpp"
#include "sys/utils/rustlike/Optional.hpp"


namespace ymd::custom{
    template<>
    struct result_converter<void, BusError, BusError> {
        static Result<void, BusError> convert(const BusError & res){
            if(res.ok()) return Ok();
            else return Err(res); 
        }
    };
}

namespace ymd::drivers{



class INA226 {


    public:
    using BusResult = Result<void, BusError>;

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

    using RegAddress = uint8_t;

    struct ConfigReg:public Reg16<>{
        scexpr RegAddress address = 0x00;

        uint16_t shuntVoltageEnable :1;
        uint16_t busVoltageEnable :1;
        uint16_t continuos :1;
        uint16_t shuntVoltageConversionTime:3;
        uint16_t busVoltageConversionTime:3;
        uint16_t averageMode:3;
        uint16_t __resv__:3;
        uint16_t rst:1;
    };

    struct ShuntVoltReg:public Reg16<>{
        scexpr RegAddress address = 0x01;
        uint16_t :16;
    };

    struct BusVoltReg:public Reg16<>{
        scexpr RegAddress address = 0x02;
        uint16_t :16;
    };

    struct PowerReg:public Reg16i<>{
        scexpr RegAddress address = 0x03;
        int16_t :16;
    };

    struct CurrentReg:public Reg16i<>{
        scexpr RegAddress address = 0x04;
        int16_t :16;
    };
    
    struct CalibrationReg:public Reg16i<>{
        scexpr RegAddress address = 0x05;
        int16_t :16;
    };
    
    struct MaskReg:public Reg16<>{
        scexpr RegAddress address = 0x06;

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

    struct AlertLimitReg:public Reg16<>{
        scexpr RegAddress address = 0x07;
        uint16_t :16;
    };

    struct ManufactureReg:public Reg16<>{
        scexpr RegAddress address = 0xfe;
        uint16_t :16;
    };

    struct ChipIdReg:public Reg16<>{
        scexpr RegAddress address = 0xff;
        uint16_t :16;
    };

    ConfigReg configReg = {};
    ShuntVoltReg shuntVoltageReg = {};
    BusVoltReg busVoltageReg = {};
    PowerReg powerReg = {};
    CurrentReg currentReg = {};
    CalibrationReg calibrationReg = {};
    MaskReg maskReg = {};
    AlertLimitReg alertLimitReg = {};
    ManufactureReg manufactureIDReg = {};
    ChipIdReg chipIDReg = {};

    [[nodiscard]] BusResult writeReg(const RegAddress addr, const uint16_t data);

    [[nodiscard]] BusResult readReg(const RegAddress addr, uint16_t & data);
    
    [[nodiscard]] BusResult readReg(const RegAddress addr, int16_t & data);

    [[nodiscard]] BusResult requestPool(const RegAddress addr, uint16_t * data_ptr, const size_t len);

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

    auto & getCurrChannel(){return ch(INA226Channel::Index::CURRENT);}
    auto & getBusVoltChannel(){return ch(INA226Channel::Index::BUS_VOLT); }
    auto & getShuntVoltChannel(){return ch(INA226Channel::Index::SHUNT_VOLT); }
    auto & getPowerChannel(){return ch(INA226Channel::Index::POWER); }


    real_t getVoltage();

    int getShuntVoltageuV();

    real_t getShuntVoltage();

    real_t getCurrent();

    real_t getPower();

    void setAverageTimes(const AverageTimes times);

    void setBusConversionTime(const ConversionTime time);

    void setShuntConversionTime(const ConversionTime time);

    void reset();

    void enableShuntVoltageMeasure(const bool en = true);

    void enableBusVoltageMeasure(const bool en = true);

    void enableContinuousMeasure(const bool en = true);

    void enableAlertLatch(const bool en = true);
};

}
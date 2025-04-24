#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Option.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{



class INA226 {
public:
    using Error = BusError;
    using BusResult = Result<void, Error>;

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
    hal::I2cDrv i2c_drv_;
    
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
    }DEF_R16(configReg);

    struct ShuntVoltReg:public Reg16<>{
        scexpr RegAddress address = 0x01;
        uint16_t data;
    };

    struct BusVoltReg:public Reg16<>{
        scexpr RegAddress address = 0x02;
        uint16_t data;
    };

    struct PowerReg:public Reg16i<>{
        scexpr RegAddress address = 0x03;
        int16_t data;
    };

    struct CurrentReg:public Reg16i<>{
        scexpr RegAddress address = 0x04;
        int16_t data;
    };
    
    struct CalibrationReg:public Reg16i<>{
        scexpr RegAddress address = 0x05;
        int16_t data;
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

    ShuntVoltReg shuntVoltageReg = {};
    BusVoltReg busVoltageReg = {};
    PowerReg powerReg = {};
    CurrentReg currentReg = {};
    CalibrationReg calibrationReg = {};
    MaskReg maskReg = {};
    AlertLimitReg alertLimitReg = {};
    ManufactureReg manufactureIDReg = {};
    ChipIdReg chipIDReg = {};

    [[nodiscard]] BusResult write_reg(const RegAddress addr, const uint16_t data);

    [[nodiscard]] BusResult read_reg(const RegAddress addr, uint16_t & data);
    
    [[nodiscard]] BusResult read_reg(const RegAddress addr, int16_t & data);

    [[nodiscard]] BusResult read_burst(const RegAddress addr, uint16_t * data_ptr, const size_t len);

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
                    return parent_.get_shunt_voltage();
                case Index::BUS_VOLT:
                    return parent_.get_voltage();
                case Index::CURRENT:
                    return parent_.get_current();
                case Index::POWER:
                    return parent_.get_power();
                default:
                    return real_t(0);
            }
        }
    };


    std::array<INA226Channel, 4> channels = {
        INA226Channel{*this, INA226Channel::Index::SHUNT_VOLT},
        INA226Channel{*this, INA226Channel::Index::BUS_VOLT},
        INA226Channel{*this, INA226Channel::Index::CURRENT},
        INA226Channel{*this, INA226Channel::Index::POWER}
    };
    
public:
    using Index = INA226Channel::Index;
    
scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x80);

    INA226(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    INA226(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    INA226(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    #undef CHANNEL_CONTENT


    void update();

    void init(const uint mohms, const uint max_current_a);

    void config(const uint mohms, const uint max_current_a);

    void set_average_times(const uint16_t times);

    bool verify();

    auto & ch(const Index index){
        return channels[uint8_t(index)];
    }

    auto & get_curr_channel(){return ch(INA226Channel::Index::CURRENT);}
    auto & get_bus_volt_channel(){return ch(INA226Channel::Index::BUS_VOLT); }
    auto & get_shunt_volt_channel(){return ch(INA226Channel::Index::SHUNT_VOLT); }
    auto & get_power_channel(){return ch(INA226Channel::Index::POWER); }


    real_t get_voltage();

    int get_shunt_voltage_uv();

    real_t get_shunt_voltage();

    real_t get_current();

    real_t get_power();

    void set_average_times(const AverageTimes times);

    void set_bus_conversion_time(const ConversionTime time);

    void set_shunt_conversion_time(const ConversionTime time);

    void reset();

    void enable_shunt_voltage_measure(const bool en = true);

    void enable_bus_voltage_measure(const bool en = true);

    void enable_continuous_measure(const bool en = true);

    void enable_alert_latch(const bool en = true);
};


}

namespace ymd::custom{
    template<>
    struct result_converter<void, drivers::INA226::Error, BusError> {
        static Result<void, drivers::INA226::Error> convert(const BusError & res){
            if(res.ok()) return Ok();
            else return Err(res); 
        }
    };
}

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{



class INA226 {
public:
    // ASCII 的 TI。
    static constexpr uint16_t VALID_MANU_ID = 0x5449;

    // INA226
    static constexpr uint16_t VALID_CHIP_ID = 0x2260;

    enum class Error_Kind{
        ChipIdVerifyFailed,
        ManuIdVerifyFailed,
        ResTooBig
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

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

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t data);

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint16_t & data);
    
    [[nodiscard]] IResult<> read_reg(const RegAddress addr, int16_t & data);

    [[nodiscard]] IResult<> read_reg(auto & reg){
        return IResult<>(read_reg(reg.address, reg.as_ref()));
    }

    [[nodiscard]] IResult<> write_reg(const auto & reg){
        return IResult<>(write_reg(reg.address, reg.as_val()));
    }
    
    // [[nodiscard]] IResult<> read_burst(const RegAddress addr, uint16_t * data_ptr, const size_t len);

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
                    return parent_.get_shunt_voltage().unwrap();
                case Index::BUS_VOLT:
                    return parent_.get_voltage().unwrap();
                case Index::CURRENT:
                    return parent_.get_current().unwrap();
                case Index::POWER:
                    return parent_.get_power().unwrap();
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
    
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x80);

    INA226(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    INA226(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    INA226(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};


    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> init(const uint mohms, const uint max_current_a);

    [[nodiscard]] IResult<> config(const uint mohms, const uint max_current_a);

    [[nodiscard]] IResult<> set_average_times(const uint16_t times);

    [[nodiscard]] IResult<> verify();

    [[nodiscard]] auto & ch(const Index index){
        return channels[uint8_t(index)];
    }

    [[nodiscard]] auto & get_curr_channel(){return ch(INA226Channel::Index::CURRENT);}
    [[nodiscard]] auto & get_bus_volt_channel(){return ch(INA226Channel::Index::BUS_VOLT); }
    [[nodiscard]] auto & get_shunt_volt_channel(){return ch(INA226Channel::Index::SHUNT_VOLT); }
    [[nodiscard]] auto & get_power_channel(){return ch(INA226Channel::Index::POWER); }


    [[nodiscard]] IResult<real_t> get_voltage();

    [[nodiscard]] IResult<int> get_shunt_voltage_uv();

    [[nodiscard]] IResult<real_t> get_shunt_voltage();

    [[nodiscard]] IResult<real_t> get_current();

    [[nodiscard]] IResult<real_t> get_power();

    [[nodiscard]] IResult<> set_average_times(const AverageTimes times);

    [[nodiscard]] IResult<> set_bus_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<> set_shunt_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> enable_shunt_voltage_measure(const bool en = true);

    [[nodiscard]] IResult<> enable_bus_voltage_measure(const bool en = true);

    [[nodiscard]] IResult<> enable_continuous_measure(const bool en = true);

    [[nodiscard]] IResult<> enable_alert_latch(const bool en = true);
};


}

namespace ymd::custom{
    template<>
    struct result_converter<void, drivers::INA226::Error, hal::HalResult> {
        static Result<void, drivers::INA226::Error> convert(const hal::HalResult & res){
            if(res.is_ok()) return Ok();
            else return Err(res.unwrap_err()); 
        }
    };
}

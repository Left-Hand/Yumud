#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct INA226_Collections{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x80);
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

    enum class AverageTimes:uint16_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint16_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };

    using RegAddress = uint8_t;

    scexpr real_t VOLTAGE_LSB_MV = real_t(1.25);
};

struct INA226_Regs:public INA226_Collections{
    struct ConfigReg:public Reg16<>{
        scexpr RegAddress address = 0x00;

        uint16_t shunt_voltage_enable :1;
        uint16_t bus_voltage_enable :1;
        uint16_t continuos :1;
        ConversionTime shunt_voltage_conversion_time:3;
        ConversionTime bus_voltage_conversion_time:3;
        AverageTimes average_times:3;
        uint16_t __resv__:3;
        uint16_t rst:1;
    }DEF_R16(config_reg)

    struct ShuntVoltReg:public Reg16<>{
        scexpr RegAddress address = 0x01;
        uint16_t data;
    }DEF_R16(shunt_volt_reg)

    struct BusVoltReg:public Reg16<>{
        scexpr RegAddress address = 0x02;
        uint16_t data;
    }DEF_R16(bus_volt_reg)

    struct PowerReg:public Reg16i<>{
        scexpr RegAddress address = 0x03;
        int16_t data;
    }DEF_R16(power_reg)

    struct CurrentReg:public Reg16i<>{
        scexpr RegAddress address = 0x04;
        int16_t data;
    }DEF_R16(current_reg)
    
    struct CalibrationReg:public Reg16i<>{
        scexpr RegAddress address = 0x05;
        int16_t data;
    }DEF_R16(calibration_reg)
    
    struct MaskReg:public Reg16<>{
        scexpr RegAddress address = 0x06;

        uint16_t alert_latch_enable:1;
        uint16_t alert_polarity:1;
        uint16_t math_overflow:1;
        uint16_t conv_ready_flag:1;
        uint16_t alert_flag:1;
        uint16_t __resv__:5;
        uint16_t conv_ready:1;
        uint16_t power_overlimit:1;
        uint16_t bus_under_voltage:1;
        uint16_t bus_over_voltage:1;
        uint16_t shunt_under_voltage:1;
        uint16_t shunt_over_voltage:1;
    }DEF_R16(mask_reg)

    struct AlertLimitReg:public Reg16<>{
        scexpr RegAddress address = 0x07;
        uint16_t data;
    }DEF_R16(alert_limit_reg)

    struct ManufactureReg:public Reg16<>{
        scexpr RegAddress address = 0xfe;
        uint16_t data;
    }DEF_R16(manufacture_reg)

    struct ChipIdReg:public Reg16<>{
        scexpr RegAddress address = 0xff;
        uint16_t data;
    }DEF_R16(chip_id_reg)
};

class INA226 final:public INA226_Regs{
public:
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
    
    struct Config{
        const uint average_times = 16;
        const ConversionTime bus_conv_time = ConversionTime::_140us;
        const ConversionTime shunt_conv_time = ConversionTime::_140us;
        const uint mohms;
        const uint max_current_a;
    };


    INA226(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    INA226(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    INA226(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};


    [[nodiscard]] IResult<> init(const Config & cfg);

    [[nodiscard]] IResult<> set_scale(const uint mohms, const uint max_current_a);

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_average_times(const uint16_t times);

    [[nodiscard]] auto & ch(const Index index){
        return channels[uint8_t(index)];
    }

    [[nodiscard]] auto & get_curr_channel(){
        return ch(INA226Channel::Index::CURRENT);}
    [[nodiscard]] auto & get_bus_volt_channel(){
        return ch(INA226Channel::Index::BUS_VOLT);}
    [[nodiscard]] auto & get_shunt_volt_channel(){
        return ch(INA226Channel::Index::SHUNT_VOLT);}
    [[nodiscard]] auto & get_power_channel(){
        return ch(INA226Channel::Index::POWER);}


    [[nodiscard]] IResult<real_t> get_voltage();

    [[nodiscard]] IResult<int> get_shunt_voltage_uv();

    [[nodiscard]] IResult<real_t> get_shunt_voltage();

    [[nodiscard]] IResult<real_t> get_current();

    [[nodiscard]] IResult<real_t> get_power();

    [[nodiscard]] IResult<> set_average_times(const AverageTimes times);

    [[nodiscard]] IResult<> set_bus_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<> set_shunt_conversion_time(const ConversionTime time);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> enable_shunt_voltage_measure(const Enable en = EN);

    [[nodiscard]] IResult<> enable_bus_voltage_measure(const Enable en = EN);

    [[nodiscard]] IResult<> enable_continuous_measure(const Enable en = EN);

    [[nodiscard]] IResult<> enable_alert_latch(const Enable en = EN);
private:
    hal::I2cDrv i2c_drv_;
    
    real_t current_lsb_ma_ = real_t(0.2);


    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t data);

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint16_t & data);
    
    [[nodiscard]] IResult<> read_reg(const RegAddress addr, int16_t & data);

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(reg.address, reg.as_ref());
    }
    
    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
};


}

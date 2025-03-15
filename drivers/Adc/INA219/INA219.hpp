#pragma once

#include "drivers/device_defs.h"
#include "sys/utils/Result.hpp"
#include "sys/utils/Option.hpp"


#define DEF_R16(T, name)\
static_assert(sizeof(T) == 2 and std::has_unique_object_representations_v<T>, "x must be 16bit register");\
T name = {};\

namespace ymd::drivers{

class INA219 {
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
    hal::I2cDrv i2c_drv;
    
    real_t current_lsb_ma = real_t(0.2);
    scexpr real_t voltage_lsb_mv = real_t(1.25);

    using RegAddress = uint8_t;

    struct R16_Config:public Reg16<>{
        scexpr RegAddress address = 0x00;

        uint16_t mode:3;
        uint16_t sdac:4;
        uint16_t bdac:4;
        uint16_t pg:2;
        uint16_t brng:1;
        uint16_t __resv__:1;
        uint16_t rst:1;
    };DEF_R16(R16_Config, config_reg)

    struct R16_ShuntVolt:public Reg16<>{
        scexpr RegAddress address = 0x01;
        uint16_t :16;
    };DEF_R16(R16_ShuntVolt, shunt_volt_reg)

    struct R16_BusVolt:public Reg16<>{
        scexpr RegAddress address = 0x02;
        uint16_t :16;
    };DEF_R16(R16_BusVolt, bus_volt_reg)

    struct R16_Power:public Reg16i<>{
        scexpr RegAddress address = 0x03;
        int16_t :16;
    };DEF_R16(R16_Power, power_reg)
    struct R16_Current:public Reg16i<>{
        scexpr RegAddress address = 0x04;
        int16_t :16;
    };DEF_R16(R16_Current, current_reg)
    
    struct R16_Calibration:public Reg16i<>{
        scexpr RegAddress address = 0x05;
        int16_t :16;
    };DEF_R16(R16_Calibration, calibration_reg)
    

    [[nodiscard]] Result<void, Error> writeReg(const RegAddress addr, const uint16_t data);

    [[nodiscard]] Result<void, Error> readReg(const RegAddress addr, uint16_t & data);
    
    [[nodiscard]] Result<void, Error> readReg(const RegAddress addr, int16_t & data);

    [[nodiscard]] Result<void, Error> requestPool(const RegAddress addr, uint16_t * data_ptr, const size_t len);

    // class CurrentChannel;
    // class VoltageChannel;

    // friend class CurrentChannel;
    // friend class VoltageChannel;

    // class INA219Channel:public hal::AnalogInIntf{
    // public:
    //     enum class Index:uint8_t{
    //         SHUNT_VOLT,
    //         BUS_VOLT,
    //         CURRENT,
    //         POWER
    //     };

    // protected:
    //     INA219 & parent_;
    //     Index ch_;
    // public:
    //     INA219Channel(INA219 & _parent, const Index _ch):parent_(_parent), ch_(_ch){}

    //     INA219Channel(const INA219Channel & other) = delete;
    //     INA219Channel(INA219Channel && other) = delete;
    //     operator real_t() override{
    //         switch(ch_){
    //             case Index::SHUNT_VOLT:
    //                 return parent_.getShuntVoltage();
    //             case Index::BUS_VOLT:
    //                 return parent_.getVoltage();
    //             case Index::CURRENT:
    //                 return parent_.getCurrent();
    //             case Index::POWER:
    //                 return parent_.getPower();
    //             default:
    //                 return real_t(0);
    //         }
    //     }
    // };


    // std::array<INA219Channel, 4> channels;
public:
    // using Index = INA219Channel::Index;
    
    scexpr uint8_t default_i2c_addr = 0x80;

    #define CHANNEL_CONTENT\
        INA219Channel{*this, INA219Channel::Index::SHUNT_VOLT},\
        INA219Channel{*this, INA219Channel::Index::BUS_VOLT},\
        INA219Channel{*this, INA219Channel::Index::CURRENT},\
        INA219Channel{*this, INA219Channel::Index::POWER}\

    INA219(const hal::I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    INA219(hal::I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){;}
    INA219(hal::I2c & _i2c, const uint8_t _addr = default_i2c_addr):i2c_drv(hal::I2cDrv(_i2c, _addr)){;}

    #undef CHANNEL_CONTENT


    Result<void, Error> update();

    Result<void, Error> verify();
};
}

namespace ymd::custom{
    template<>
    struct result_converter<void, drivers::INA219::Error, BusError> {
        static Result<void, drivers::INA219::Error> convert(const BusError & res){
            if(res.ok()) return Ok();
            else return Err(res); 
        }
    };
}
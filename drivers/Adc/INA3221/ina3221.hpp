#pragma once

#include "drivers/device_defs.h"
#include "hal/bus/i2c/i2cdrv.hpp"

#ifdef INA3221_DEBUG
#define INA3221_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define INA3221_DEBUG(...)
#endif

class INA3221{
public:
// Address Pins and Slave Addresses
// A0   ADDRESS0
// GND  10000000
// VS   10000010
// SDA  10000100
// SCL  10000110

    scexpr uint8_t default_i2c_addr = 0b1000000;

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

    using RegAddress = uint8_t;

    struct ConfigReg:public Reg16{
        uint16_t shunt_measure_en :1;
        uint16_t bus_measure_en :1;
        uint16_t continuos :1;
        uint16_t shunt_conv_time:3;
        uint16_t bus_conv_time:3;
        uint16_t average_times:3;
        uint16_t ch1_en:1;
        uint16_t ch2_en:1;
        uint16_t ch3_en:1;
        uint16_t rst:1;
    };

    struct ShuntVoltReg:public Reg16i{
        scexpr uint16_t address1 = 0x01;
        scexpr uint16_t address2 = 0x03;
        scexpr uint16_t address3 = 0x05;
        int16_t : 16;
    };

    struct BusVoltReg:public Reg16i{
        scexpr uint16_t address1 = 0x02;
        scexpr uint16_t address2 = 0x04;
        scexpr uint16_t address3 = 0x06;
        int16_t : 16;
    };

    struct InstantOVCReg:public Reg16i{
        scexpr uint16_t address1 = 0x07;
        scexpr uint16_t address2 = 0x09;
        scexpr uint16_t address3 = 0x0b;

        int16_t :16;
    };

    struct ConstantOVCReg:public Reg16i{
        scexpr uint16_t address1 = 0x07;
        scexpr uint16_t address2 = 0x09;
        scexpr uint16_t address3 = 0x0b;

        int16_t :16;
    };

    struct MaskReg:public Reg16{
        uint16_t conv_ready:1;
        uint16_t timing_alert:1;
        uint16_t power_valid_alert:1;
        uint16_t constant_alert1:1;
        uint16_t constant_alert2:1;
        uint16_t constant_alert3:1;
        uint16_t sum_alert:1;
        uint16_t instant_alert1:1;
        uint16_t instant_alert2:1;
        uint16_t instant_alert3:1;
        uint16_t instant_en:1;
        uint16_t constant_en:1;
        uint16_t sum_en:3;
        uint16_t :1;
    };

    struct PowerHoReg:public Reg16i{
        scexpr uint16_t address = 0x10;
        int16_t :16;
    };

    struct PowerLoReg:public Reg16i{
        scexpr uint16_t address = 0x11;
        int16_t :16;
    };

    struct ManuIdReg:public Reg16{
        scexpr uint16_t correct_id = 0x5449;
        scexpr RegAddress address = 0xfe;
        uint16_t:16;
    };

    struct ChipIdReg:public Reg16{
        scexpr uint16_t correct_id = 0x3220;
        scexpr RegAddress address = 0xff;
        uint16_t:16;
    };

    __inline void readReg(const RegAddress addr, auto & data){
        if constexpr(sizeof(data == 1)) i2c_drv.readReg(uint8_t(addr), *reinterpret_cast<uint8_t *>(&data));
        if constexpr(sizeof(data == 2)) i2c_drv.readReg(uint8_t(addr), *reinterpret_cast<uint16_t *>(&data));
    }

    __inline void writeReg(const RegAddress addr, const auto & data){
        if constexpr(sizeof(data == 1)) i2c_drv.writeReg(uint8_t(addr), *reinterpret_cast<const uint8_t *>(&data));
        if constexpr(sizeof(data == 2)) i2c_drv.writeReg(uint8_t(addr), *reinterpret_cast<const uint16_t *>(&data));
    }


    // struct INA3221Channel:public AnalogInChannel{
    // public:
    //     enum class Index:uint8_t{
    //         SHUNT_VOLT,
    //         BUS_VOLT,
    //         CURRENT,
    //         POWER
    //     };

    // protected:
    //     INA3221 & parent_;
    //     Index ch_;
    // public:
    //     INA3221Channel(INA3221 & _parent, const Index _ch):parent_(_parent), ch_(_ch){}

    //     INA3221Channel(const INA3221Channel & other) = delete;
    //     INA3221Channel(INA3221Channel && other) = delete;
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
    //                 return 0;
    //         }
    //     }
    // };

    struct{
        ConfigReg       config_reg;
        ShuntVoltReg    shuntvolt1_reg;
        BusVoltReg      busvolt1_reg;
        ShuntVoltReg    shuntvolt2_reg;
        BusVoltReg      busvolt2_reg;
        ShuntVoltReg    shuntvolt3_reg;
        BusVoltReg      busvolt3_reg;
        InstantOVCReg   instant_ovc1_reg;
        ConstantOVCReg  constant_ovc1_reg;
        InstantOVCReg   instant_ovc2_reg;
        ConstantOVCReg  constant_ovc2_reg;
        InstantOVCReg   instant_ovc3_reg;
        ConstantOVCReg  constant_ovc3_reg;

        ShuntVoltReg    shuntvolt_reg;
        ShuntVoltReg    shuntvolt_limit_reg;
        MaskReg         mask_reg;
        PowerHoReg      power_ho_reg;
        PowerLoReg      power_lo_reg;

        ManuIdReg       manu_id_reg;
        ChipIdReg       chip_id_reg;
    };

    I2cDrv i2c_drv;
    // std::array<INA3221Channel, 4> channels;
public:
    INA3221(const I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    INA3221(I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){;}
    INA3221(I2c & _i2c, const uint8_t addr = default_i2c_addr):i2c_drv(I2cDrv(_i2c, addr)){;}
    ~INA3221(){;}
    
    void init();
    void update();
    bool verify();
    void setAverageTimes(const uint16_t times);
    
};
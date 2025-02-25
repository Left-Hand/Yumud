#pragma once

#include "drivers/device_defs.h"


namespace ymd::drivers{

class INA3221{
public:
    // Address Pins and Slave Addresses
    // A0   ADDRESS 0
    // GND  1000000 0
    // VS   1000001 0
    // SDA  1000010 0
    // SCL  1000011 0

    scexpr uint8_t default_i2c_addr = 0b10000000;

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
        scexpr RegAddress address = 0x00;

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
        scexpr RegAddress address1 = 0x01;
        scexpr RegAddress address2 = 0x03;
        scexpr RegAddress address3 = 0x05;
        int16_t : 16;

        constexpr real_t to_volt() const {
            return real_t((int16_t(*this) >> 3) * 40 / 100) / 1000;
        }

        constexpr int to_uv() const {
            return ((int16_t(*this) >> 3) * 40);
        }

        static constexpr int16_t to_i16(const real_t volt){
            return int16_t(volt * 100000) & 0xfff8;
        }
    };

    struct BusVoltReg:public Reg16i{
        scexpr RegAddress address1 = 0x02;
        scexpr RegAddress address2 = 0x04;
        scexpr RegAddress address3 = 0x06;
        int16_t : 16;

        constexpr real_t to_volt() const {
            return real_t((int16_t(*this) >> 3) * 8) / 1000;
        }

        constexpr int to_mv() const {
            return int16_t((int16_t(*this) >> 3) * 8);
        }

        static constexpr int16_t to_i16(const real_t volt){
            return int16_t(volt * 1000) & 0xfff8;
        }
    };

    struct InstantOVCReg:public Reg16i{
        scexpr RegAddress address1 = 0x07;
        scexpr RegAddress address2 = 0x09;
        scexpr RegAddress address3 = 0x0b;

        int16_t :16;
    };

    struct ConstantOVCReg:public Reg16i{
        scexpr RegAddress address1 = 0x07;
        scexpr RegAddress address2 = 0x09;
        scexpr RegAddress address3 = 0x0b;

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
        scexpr RegAddress address = 0x10;
        int16_t :16;
    };

    struct PowerLoReg:public Reg16i{
        scexpr RegAddress address = 0x11;
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
        static_assert(sizeof(data) == 2);
        // if constexpr(sizeof(data == 1)) i2c_drv.readReg(uint8_t(addr), *reinterpret_cast<uint8_t *>(&data));
        if constexpr(sizeof(data == 2)) i2c_drv.readReg(uint8_t(addr), reinterpret_cast<uint16_t &>(data), MSB);
    }

    __inline void writeReg(const RegAddress addr, const auto & data){
        static_assert(sizeof(data) == 2);
        // if constexpr(sizeof(data == 1)) i2c_drv.writeReg(uint8_t(addr), *reinterpret_cast<const uint8_t *>(&data));
        if constexpr(sizeof(data == 2)) i2c_drv.writeReg(uint8_t(addr), reinterpret_cast<const uint16_t &>(data), MSB);
    }

    void requestPool(const RegAddress addr, void * data_ptr, const size_t len){
        i2c_drv.readMulti((uint8_t)addr, (uint16_t *)data_ptr, len, LSB);
    }

    struct INA3221Channel:public AnalogInChannel{
    public:
        enum class ChannelType:uint8_t{
            SHUNT_VOLT,
            BUS_VOLT,
        };

        using ChannelIndex = uint8_t;

    protected:
        INA3221 & parent_;
        ChannelType ch_;
        ChannelIndex idx_;
    public:
        INA3221Channel(INA3221 & parent, const ChannelType ch, const ChannelIndex idx):
                parent_(parent), ch_(ch), idx_(idx){}

        INA3221Channel(const INA3221Channel & other) = delete;
        INA3221Channel(INA3221Channel && other) = delete;
        operator real_t() override{
            switch(ch_){
                case ChannelType::SHUNT_VOLT:
                    return parent_.getShuntVolt(idx_);
                case ChannelType::BUS_VOLT:
                    return parent_.getBusVolt(idx_);
                default:
                    return real_t(0);
            }
        }
    };
    
    I2cDrv i2c_drv;

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


    std::array<INA3221Channel, 6> channels = {
        INA3221Channel{*this, INA3221Channel::ChannelType::BUS_VOLT, 0},
        INA3221Channel{*this, INA3221Channel::ChannelType::SHUNT_VOLT, 0},
        INA3221Channel{*this, INA3221Channel::ChannelType::BUS_VOLT, 1},
        INA3221Channel{*this, INA3221Channel::ChannelType::SHUNT_VOLT, 1},
        INA3221Channel{*this, INA3221Channel::ChannelType::BUS_VOLT, 2},
        INA3221Channel{*this, INA3221Channel::ChannelType::SHUNT_VOLT, 2},
    };
public:
    INA3221(const I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    INA3221(I2cDrv && _i2c_drv):i2c_drv(std::move(_i2c_drv)){;}
    INA3221(I2c & _i2c, const uint8_t addr = default_i2c_addr):i2c_drv(I2cDrv(_i2c, addr)){;}
    ~INA3221(){;}
    
    bool ready();

    INA3221 & init();
    INA3221 & update();
    INA3221 & update(const size_t index);
    bool verify();
    INA3221 & reset();
    INA3221 & setAverageTimes(const uint16_t times);
    
    INA3221 & enableChannel(const size_t index, const bool en = true);

    INA3221 & setBusConversionTime(const ConversionTime time);
    INA3221 & setShuntConversionTime(const ConversionTime time);

    int getShuntVoltuV(const size_t index);
    int getBusVoltmV(const size_t index);

    real_t getShuntVolt(const size_t index);
    real_t getBusVolt(const size_t index);

    INA3221 & setInstantOVC(const size_t index, const real_t volt);
    INA3221 & setConstantOVC(const size_t index, const real_t volt);
};

}
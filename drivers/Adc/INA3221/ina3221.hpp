#pragma once

#include "../../hal/bus/i2c/i2cdrv.hpp"

#ifdef INA3221_DEBUG
#define INA3221_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define INA3221_DEBUG(...)
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#define REG16(x) (*reinterpret_cast<uint16_t *>(&x))
#define REG8(x) (*reinterpret_cast<uint8_t *>(&x))

class INA3221{
public:
// Address Pins and Slave Addresses
// A0   ADDRESS0
// GND  10000000
// VS   10000010
// SDA  10000100
// SCL  10000110

    static constexpr default_i2c_addr = 0b1000000;

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

    INA3221(const I2cDrv & i2c_driver) : i2c_drv(i2c_driver){}
    INA3221(I2cDrv && i2c_driver) : i2c_drv(i2c_driver){}
    INA3221(I2c & i2c, const uint8_t addr = default_i2c_addr) : i2c_drv(I2cDrv(i2c, addr)){}

protected:

    using RegAddress = uint8_t;

    struct ConfigReg{
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

    struct ShuntVoltReg{
        static constexpr uint16_t address1 = 0x01;
        static constexpr uint16_t address2 = 0x03;
        static constexpr uint16_t address3 = 0x05;

        int16_t value;
    };

    struct BusVoltReg{
        static constexpr uint16_t address1 = 0x02;
        static constexpr uint16_t address2 = 0x04;
        static constexpr uint16_t address3 = 0x06;

        int16_t value;
    };

    struct InstantOVCReg{
        static constexpr uint16_t address1 = 0x07;
        static constexpr uint16_t address2 = 0x09;
        static constexpr uint16_t address3 = 0x0b;

        int16_t value;
    };

    struct ConstantOVCReg{
        static constexpr uint16_t address1 = 0x07;
        static constexpr uint16_t address2 = 0x09;
        static constexpr uint16_t address3 = 0x0b;

        int16_t value;
    };

    struct MaskReg{
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

    }

    struct ManuIdReg{
        static constexpr uint16_t correct_id = 0x5449;
        static constexpr RegAddress address = 0xfe;
    };

    struct DieIdReg{
        static constexpr uint16_t correct_id = 0x3220;
        static constexpr RegAddress address = 0xff;
    };


    struct PowerHoReg{
        static constexpr uint16_t address = 0x10;
        int16_t value;
    };

    struct PowerUlReg{
        static constexpr uint16_t address = 0x10;
        int16_t value;
    };



    void readReg(const RegAddress addr, auto & data){
        i2c_drv.readReg(uint8_t(addr), data);
    }

    void writeReg(const RegAddress addr, const auto & data){
        i2c_drv.writeReg(uint8_t(addr), data);
    }
protected:
    I2cDrv i2c_drv;

    
};

#pragma GCC diagnostic pop
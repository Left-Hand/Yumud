#pragma once

#include "../../hal/bus/i2c/i2cdrv.hpp"

#ifdef ADS111X_DEBUG
#define ADS111X_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define ADS111X_DEBUG(...)
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#define REG16(x) (*reinterpret_cast<uint16_t *>(&x))
#define REG8(x) (*reinterpret_cast<uint8_t *>(&x))

class ADS111X{
public:
    enum class Package:uint8_t{
        ADS1113,
        ADS1114,
        ADS1115
    };

    enum class DataRate:uint8_t{
        _8 = 0b000,
        _16, _32, _64, _128, _250, _475, _860
    };

    enum class MUX:uint8_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class PGA:uint8_t{
        _6_144V = 0, _4_096V, _2_048V, _1_024V, _0_512V, _0_256V
    };

protected:
    I2cDrv i2c_drv;

    using RegAddress = uint8_t;

    struct ConversionReg{
        static constexpr RegAddress address = 0b00; 
        uint16_t data;
    };

    struct ConfigReg{
        static constexpr RegAddress address = 0b01; 
        uint16_t comp_que:2;
        bool comp_latch:1;
        bool comp_pol:1;
        bool comp_mode_is_window:1;
        DataRate data_rate:3;
        bool oneshot_en:1;
        PGA pga:3;
        MUX mux:3;
        bool busy:1;
    };

    struct LowThreshReg{
        static constexpr RegAddress address = 0b10;
        int16_t data;
    };

    struct HighThreshReg{
        static constexpr RegAddress address = 0b11; 
        int16_t data;
    };

    struct{
        ConversionReg conversion_reg;
        ConfigReg config_reg;
        LowThreshReg low_thresh_reg;
        HighThreshReg high_thresh_reg;
    };

    void readReg(const RegAddress addr, uint16_t & data){
        i2c_drv.readReg(uint8_t(addr), data);
    }

    void writeReg(const RegAddress addr, const uint16_t data){
        i2c_drv.writeReg(uint8_t(addr), data);
    }
public:
    // ADDR PIN CONNECTION SLAVE ADDRESS
    // GND 1001000
    // VDD 1001001
    // SDA 1001010
    // SCL 1001011
    static constexpr uint8_t default_i2c_addr = 0b10010000;
    ADS111X(const I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    ADS111X(I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){;}
    ADS111X(I2c & _i2c, const uint8_t _addr = default_i2c_addr):i2c_drv(I2cDrv(_i2c, _addr)){};

    void startConv(){
        auto & reg = config_reg;
        reg.busy = true;
        writeReg(reg.address, REG16(reg));
        reg.busy = false;
    }

    bool isBusy(){
        auto & reg = config_reg;
        readReg(reg.address, REG16(reg));
        return reg.busy;
    }

    void setThreshold(int16_t low, int16_t high){
        low_thresh_reg.data = low;
        high_thresh_reg.data = high;
        writeReg(LowThreshReg::address, REG16(low_thresh_reg));
        writeReg(HighThreshReg::address, REG16(high_thresh_reg));
    }

    void enableContMode(bool en = true){
        auto & reg = config_reg;
        reg.oneshot_en =!en;
        writeReg(reg.address, REG16(reg));
    }

    void setPga(const PGA pga){
        auto & reg = config_reg;
        reg.pga = pga;
        writeReg(reg.address, REG16(reg));
    }

    void setMux(const MUX mux){
        auto & reg = config_reg;
        reg.mux = mux;
        writeReg(reg.address, REG16(reg));
    }

    void setDataRate(const DataRate data_rate){
        auto & reg = config_reg;
        reg.data_rate = data_rate;
        writeReg(reg.address, REG16(reg));
    }
};

using ADS1113 = ADS111X;
using ADS1114 = ADS111X;
using ADS1115 = ADS111X;

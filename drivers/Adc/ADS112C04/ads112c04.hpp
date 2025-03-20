#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{
class ADS112C04{
public:
    enum class DataRate:uint8_t{
        _20 = 0, _45, _90, _175, _330, _600, _1000
    };


    enum class MUX : uint8_t {
        P0N1 = 0b0000, // AINP = AIN0, AINN = AIN1 (default)
        P0N2 = 0b0001, // AINP = AIN0, AINN = AIN2
        P0N3 = 0b0010, // AINP = AIN0, AINN = AIN3
        P1N0 = 0b0011, // AINP = AIN1, AINN = AIN0
        P1N2 = 0b0100, // AINP = AIN1, AINN = AIN2
        P1N3 = 0b0101, // AINP = AIN1, AINN = AIN3
        P2N3 = 0b0110, // AINP = AIN2, AINN = AIN3
        P3N2 = 0b0111, // AINP = AIN3, AINN = AIN2
        P0NG = 0b1000, // AINP = AIN0, AINN = AVSS
        P1NG = 0b1001, // AINP = AIN1, AINN = AVSS
        P2NG = 0b1010, // AINP = AIN2, AINN = AVSS
        P3NG = 0b1011, // AINP = AIN3, AINN = AVSS
        PRNR = 0b1100, // (V(REFP) – V(REFN)) / 4 monitor (PGA bypassed)
        PVNG = 0b1101, // (AVDD – AVSS) / 4 monitor (PGA bypassed)
        PHNH = 0b1110, // AINP and AINN shorted to (AVDD + AVSS) / 2
        __RESV__ = 0b1111 // Reserved
    };

    enum class VREF:uint8_t{
        INTERNAL = 0b00, // 00 : Internal 2.048-V reference selected (default)
        EXTERNEL = 0b01, // 01 : External reference selected using the REFP and REFN inputs
        SUPPLY = 0b10// 10 : Analog supply (AVDD – AVSS) used as reference
    };

    enum class GAIN:uint8_t{
        _1 = 0, _2, _4, _8, _16, _32, _64, _128
    };

    enum class IDAC {
        OFF     = 0b000, // 000 : Off (default)
        _10uA   = 0b001, // 001 : 10 µA
        _50uA   = 0b010, // 010 : 50 µA
        _100uA  = 0b011, // 011 : 100 µA
        _250uA  = 0b100, // 100 : 250 µA
        _500uA  = 0b101, // 101 : 500 µA
        _1000uA = 0b110, // 110 : 1000 µA
        _1500uA = 0b111  // 111 : 1500 µA
    };

    enum class CRC_Type{
        DISABLED = 0b00, // 00 : CRC disabled (default)
        INV = 0b01, // 01 : CRC 8-bit
        _16BIT = 0b10, // 10 : CRC 16-bit
        __RESV__ = 0b11 // 11 : CRC 24-bit
    };
protected:
    hal::I2cDrv i2c_drv;

    // [A1]     [A0]    [I2C ADDRESS]
    // DGND     DGND    100 0000
    // DGND     DVDD    100 0001
    // DGND     SDA     100 0010
    // DGND     SCL     100 0011
    // DVDD     DGND    100 0100
    // DVDD     DVDD    100 0101
    // DVDD     SDA     100 0110
    // DVDD     SCL     100 0111
    // SDA      DGND    100 1000
    // SDA      DVDD    100 1001
    // SDA      SDA     100 1010
    // SDA      SCL     100 1011
    // SCL      DGND    100 1100
    // SCL      DVDD    100 1101
    // SCL      SDA     100 1110
    // SCL      SCL     100 1111

    scexpr uint8_t default_i2c_addr = 0b10000000;

    enum class Command:uint8_t{
        RST =         0b0000'0110,
        START =         0b0000'1000,
        POWER_DOWN =    0b0000'0010,
        READ_DATA =     0b0001'0000,
        READ_REG =      0b0010'0000,
        WRITE_REG =     0b0100'0000
    };

    enum class IDAC1_MUX {
        DISABLED = 0b000, // 000 : IDAC1 disabled (default)
        AIN0    = 0b001, // 001 : IDAC1 connected to AIN0
        AIN1    = 0b010, // 010 : IDAC1 connected to AIN1
        AIN2    = 0b011, // 011 : IDAC1 connected to AIN2
        AIN3    = 0b100, // 100 : IDAC1 connected to AIN3
        REFP    = 0b101, // 101 : IDAC1 connected to REFP
        REFN    = 0b110, // 110 : IDAC1 connected to REFN
        __RESV__ = 0b111  // 111 : Reserved
    };

    enum class IDAC2_MUX {
        DISABLED = 0b000, // 000 :IDAC2 disabled (default)
        AIN0    = 0b001, // 001 : IDAC2 connected to AIN0
        AIN1    = 0b010, // 010 : IDAC2 connected to AIN1
        AIN2    = 0b011, // 011 : IDAC2 connected to AIN2
        AIN3    = 0b100, // 100 : IDAC2 connected to AIN3
        REFP    = 0b101, // 101 : IDAC2 connected to REFP
        REFN    = 0b110, // 110 : IDAC2 connected to REFN
        __RESV__ = 0b111  // 111 : Reserved
    };

    using RegAddress = uint8_t;

    struct Config0Reg:public Reg8<>{
        scexpr RegAddress address = 0;
        uint8_t pga_bypass:1;
        uint8_t gain:3;
        uint8_t mux:4;
    };

    struct Config1Reg:public Reg8<>{
        scexpr RegAddress address = 1;
        uint8_t temp_sensor_mode:1;
        uint8_t vref:2;
        uint8_t cont_mode:1;
        uint8_t turbo_mode:1;
        uint8_t data_rate:3;
    };

    struct Config2Reg:public Reg8<>{
        scexpr RegAddress address = 2;
        uint8_t idac:3;
        uint8_t current_sense_en:1;
        uint8_t crc_type:2;
        uint8_t data_counter_en:1;
        uint8_t conv_done:1;
    };

    struct Config3Reg:public Reg8<>{
        scexpr RegAddress address = 3;
        uint8_t __resv__:2;
        uint8_t idac1_mux:3;
        uint8_t idac2_mux:3;
    };

    Config0Reg config0_reg = {};
    Config1Reg config1_reg = {};
    Config2Reg config2_reg = {};
    Config3Reg config3_reg = {};



    auto readData(uint16_t & data){
        return i2c_drv.read_reg(uint8_t(Command::READ_DATA), data, LSB);
    }

    auto read_reg(const RegAddress addr, uint8_t & data){
        return i2c_drv.read_reg(uint8_t(uint8_t(Command::READ_REG) + addr), data);
    }

    auto write_reg(const RegAddress addr, const uint8_t data){
        return i2c_drv.write_reg(uint8_t(uint8_t(Command::WRITE_REG) + addr), data);
    }
public:

    ADS112C04(const hal::I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    ADS112C04(hal::I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){;}
    ADS112C04(hal::I2c & _i2c, const uint8_t _addr = default_i2c_addr):i2c_drv(hal::I2cDrv(_i2c, _addr)){};

    void init(){
        config3_reg.__resv__ = 0;
    }
    void setMux(const MUX mux){
        config0_reg.mux = uint8_t(mux);
        write_reg(config0_reg.address, config0_reg);
    }

    void setGain(const GAIN gain){
        config0_reg.gain = uint8_t(gain);
        write_reg(config0_reg.address, config0_reg);
    }

    void enableTurbo(const bool en = true){
        config1_reg.turbo_mode = en;
        write_reg(config1_reg.address, (config1_reg));
    }

    bool isDone(){
        read_reg(config2_reg.address, (config2_reg));
        return config2_reg.conv_done;
    }

    void setIDAC(const IDAC idac){
        config2_reg.idac = uint8_t(idac);
        write_reg(config2_reg.address, (config2_reg));
    }

    void setDataRate(const DataRate data_rate){
        config1_reg.data_rate = uint8_t(data_rate);
        write_reg(config1_reg.address, (config1_reg));
    }
};

}
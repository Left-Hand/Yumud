#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct ADS112C04_Prelude{

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

    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b1000000);

    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Command:uint8_t{
        RST =         0b0000'0110,
        START =         0b0000'1000,
        POWER_DOWN =    0b0000'0010,
        READ_DATA =     0b0001'0000,
        READ_REG =      0b0010'0000,
        WRITE_REG =     0b0100'0000
    };

    enum class IDAC1_MUX:uint8_t{
        DISABLED = 0b000, // 000 : IDAC1 disabled (default)
        AIN0    = 0b001, // 001 : IDAC1 connected to AIN0
        AIN1    = 0b010, // 010 : IDAC1 connected to AIN1
        AIN2    = 0b011, // 011 : IDAC1 connected to AIN2
        AIN3    = 0b100, // 100 : IDAC1 connected to AIN3
        REFP    = 0b101, // 101 : IDAC1 connected to REFP
        REFN    = 0b110, // 110 : IDAC1 connected to REFN
        __RESV__ = 0b111  // 111 : Reserved
    };

    enum class IDAC2_MUX:uint8_t{
        DISABLED = 0b000, // 000 :IDAC2 disabled (default)
        AIN0    = 0b001, // 001 : IDAC2 connected to AIN0
        AIN1    = 0b010, // 010 : IDAC2 connected to AIN1
        AIN2    = 0b011, // 011 : IDAC2 connected to AIN2
        AIN3    = 0b100, // 100 : IDAC2 connected to AIN3
        REFP    = 0b101, // 101 : IDAC2 connected to REFP
        REFN    = 0b110, // 110 : IDAC2 connected to REFN
        __RESV__ = 0b111  // 111 : Reserved
    };

    enum class DataRate:uint8_t{
        _20 = 0, _45, _90, _175, _330, _600, _1000
    };


    enum class Mux : uint8_t {
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

    enum class Vref:uint8_t{
        INTERNAL = 0b00, // 00 : Internal 2.048-V reference selected (default)
        EXTERNEL = 0b01, // 01 : External reference selected using the REFP and REFN inputs
        SUPPLY = 0b10// 10 : Analog supply (AVDD – AVSS) used as reference
    };

    enum class Gain:uint8_t{
        _1 = 0, _2, _4, _8, _16, _32, _64, _128
    };

    enum class IDAC:uint8_t{
        OFF     = 0b000, // 000 : Off (default)
        _10uA   = 0b001, // 001 : 10 µA
        _50uA   = 0b010, // 010 : 50 µA
        _100uA  = 0b011, // 011 : 100 µA
        _250uA  = 0b100, // 100 : 250 µA
        _500uA  = 0b101, // 101 : 500 µA
        _1000uA = 0b110, // 110 : 1000 µA
        _1500uA = 0b111  // 111 : 1500 µA
    };

    enum class CrcType:uint8_t{
        DISABLED = 0b00, // 00 : CRC disabled (default)
        INV = 0b01, // 01 : CRC 8-bit
        _16BIT = 0b10, // 10 : CRC 16-bit
        __RESV__ = 0b11 // 11 : CRC 24-bit
    };
    using RegAddr = uint8_t;
};

struct ADS112C04_Regs:public ADS112C04_Prelude{
    struct R8_Config0:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0;
        uint8_t pga_bypass:1;
        Gain gain:3;
        Mux mux:4;
    }DEF_R8(config0_reg)

    struct R8_Config1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 1;
        uint8_t temp_sensor_mode:1;
        Vref vref:2;
        uint8_t cont_mode:1;
        uint8_t turbo_mode:1;
        DataRate data_rate:3;
    }DEF_R8(config1_reg)

    struct R8_Config2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 2;
        IDAC idac:3;
        uint8_t current_sense_en:1;
        CrcType crc_type:2;
        uint8_t data_counter_en:1;
        uint8_t conv_done:1;
    }DEF_R8(config2_reg)

    struct R8_Config3:public Reg8<>{
        static constexpr RegAddr ADDRESS = 3;
        uint8_t __resv__:2;
        IDAC1_MUX idac1_mux:3;
        IDAC2_MUX idac2_mux:3;
    }DEF_R8(config3_reg)
};


class ADS112C04 final:
    public ADS112C04_Regs{
public:

    explicit ADS112C04(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit ADS112C04(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit ADS112C04(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};


    struct Config{

    };

    IResult<> init();

    IResult<> validate();

    IResult<> set_mux(const Mux mux);

    IResult<> set_gain(const Gain gain);

    IResult<> enable_turbo(const Enable en);

    IResult<bool> is_done();

    IResult<> set_idac(const IDAC idac);

    IResult<> set_data_rate(const DataRate data_rate);
private:
    hal::I2cDrv i2c_drv_;

    IResult<> read_data(uint16_t & data){
        if(const auto res = i2c_drv_.read_reg(
                std::bit_cast<uint8_t>(Command::READ_DATA), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddr addr, uint8_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(
                std::bit_cast<uint8_t>(Command::READ_REG) + addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(const RegAddr addr, const uint8_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(
                std::bit_cast<uint8_t>(Command::WRITE_REG) + addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_ref());
    }
};

}
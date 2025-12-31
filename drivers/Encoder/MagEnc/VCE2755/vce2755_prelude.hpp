#pragma once



#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/bus/spi/spidrv.hpp"


#include "drivers/Encoder/MagEncoder.hpp"


namespace ymd::drivers{

struct VCE2755_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Package:uint8_t{
        SOIC8 = 0x5A,
        TQFN3 = 0x5B
    };

    enum class PwmFreq:uint8_t{
        _971_1Hz,_485_6Hz
    };


    enum class Mux:uint8_t{
        _1,
        _2,
        _3,
        _4,

        TQFN_ABZ_PWM_SSI_SPI = _1,
        TQFN_UVW_PWM_SSI_SPI = _2,
        TQFN_ABZ_UVW_SPI = _3,
        TQFN_ABZ_SPI = _4,

        SOIC_ABZ_SPI = _1,
        SOIC_UVW_SPI = _2,
        SOIC_PWM_SPI = _3,
    };

    enum class Hysteresis:uint8_t{
        _0deg = 0b000,
        _0_011deg = 0b001,
        _0_022deg = 0b010,
        _0_044deg = 0b011,
        _0_066deg = 0b100,
        _0_088deg = 0b101,
        _0_132deg = 0b110,
        _0_176deg = 0b111,
    };

    enum class ZeroPulseWidth:uint8_t{
        _1lsb   = 0b000,
        _2lsb   = 0b001,
        _4lsb   = 0b010,
        _8lsb   = 0b011,
        _12lsb  = 0b100,
        _16lsb  = 0b101,
        _180deg = 0b110,
    };

    enum class BandWidth:uint8_t{ 
        _8BW0 = 0b011001,
        _4BW0 = 0b100011,
        _2BW0 = 0b101101,
        _BW0  = 0b110111,
    };


    struct [[nodiscard]] Packet{
        union{
            struct {
                uint8_t angle_17_10;
                uint8_t angle_9_2;

                // CRC0～CRC3 为 4bitCRC，系 ANGLE+SMF+BTE 共 20bit 数据的 CRC 校验值，
                // 对应的 CRC 生成多项式为 X4+X+1，初始值=0000b，数据输入输出不取反。
                uint8_t crc_3_0:4;

                uint8_t bte:1;
                uint8_t mag_weak:1;
                uint8_t angle_1_0:2;
            };

            std::array<uint8_t, 3> bytes;
        };
        uint8_t __padding__; // to 32bit
            
        [[nodiscard]] static Packet from_bytes(
            const uint8_t b1, const uint8_t b2, const uint8_t b3
        ){
            Packet ret;
            ret.bytes[0] = b1;
            ret.bytes[1] = b2;
            ret.bytes[2] = b3;
            return ret;
        }

        [[nodiscard]] bool is_crc_valid() const {
            return calc_crc() == crc_3_0;
        }

        [[nodiscard]] IResult<Angular<uq32>> parse() const {
            if(!is_crc_valid()) [[unlikely]]
                return Err(Error::InvalidCrc);

            if(mag_weak) [[unlikely]]
                return Err(Error::MagnetLow);

            const auto b18 = static_cast<uint32_t>(b20() >> 2);
            const auto turns = static_cast<uq32>(uq18::from_bits(b18));
            return Ok(Angular<uq32>::from_turns(turns));
        }
    private:
        [[nodiscard]] constexpr uint32_t b20() const{
            return (bytes[0] << 12) | (bytes[1] << 8) | (bytes[2] >> 4);
        }

        [[nodiscard]] constexpr uint8_t calc_crc() const {
            // CRC0～CRC3 为 4bitCRC，系 ANGLE+SMF+BTE 共 20bit 数据的 CRC 校验值，
            // 对应的 CRC 生成多项式为 X^4+X+1，初始值=0000b，数据输入输出不取反。

            const auto data = b20();
            
            // CRC-4 with polynomial X^4 + X + 1 (0x13 in normal representation)
            // But we only use the 4 MSB bits of the polynomial: 0x9 (1001)
            uint8_t crc = 0;
            uint32_t crc_data = data;
            
            // Process all 20 bits
            for(int i = 19; i >= 0; i--) {
                uint8_t bit = (crc_data >> i) & 1;
                uint8_t msb = (crc >> 3) & 1;
                
                crc <<= 1;
                crc |= bit;
                
                if(msb) {
                    crc ^= 0x3; // 0b0011 (X + 1 part of the polynomial X^4 + X + 1)
                }
            }
            
            // Finalize CRC by processing 4 more bits
            for(size_t i = 0; i < 4; i++) {
                uint8_t msb = (crc >> 3) & 1;
                crc <<= 1;
                if(msb) {
                    crc ^= 0x3; // 0b0011
                }
            }
            
            return crc & 0xF;
        }
    };

    enum class AbzPowerOnWaveform:uint8_t{
        // 00 脉冲序列 1（上电期间的标准脉冲输出）


        // 01 脉冲序列 2（上电期间 Z1 脉冲从低拉高持续 5ms 后，AB 输
        // 出上电初始位置的绝对角度脉冲信号）


        // 11 脉冲序列 3（上电期间 Z1 脉冲从低拉高持续 10ms 后，AB
        // 输出上电初始位置的绝对角度脉冲信号
    };

};

struct VCE2755_Regset:public VCE2755_Prelude{
    //0x00
    struct [[nodiscard]] R8_ChipId:public Reg8<>{
        static constexpr Package KEY1 = Package::SOIC8;
        static constexpr Package KEY2 = Package::TQFN3;

        Package package;

        [[nodiscard]] constexpr bool is_valid() const {
            return package == KEY1 || package == KEY2;
        }
    };


    static_assert(sizeof(Packet) == 4);

    //0x40
    struct [[nodiscard]] R8_IO:public Reg8<> {
        uint8_t spi_3wire_en:1;

        //0: 2mA 
        //1: 4mA
        uint8_t io_strength:1;
        Mux mux:2;
        AbzPowerOnWaveform abz_power_on_waveform:2;
        uint8_t :2;
    };

    //0x41
    struct [[nodiscard]] R8_AbzInvert:public Reg8<> {
        uint8_t invert_en:1;
        uint8_t :7;
    };

    //0x42
    struct [[nodiscard]] R8_PwmMode:public Reg8<> {
        uint8_t :6;
        PwmFreq pwm_mode:1;
        uint8_t :1;
    };

    //0x43
    struct [[nodiscard]] R8_Direction:public Reg8<> {
        uint8_t :5;

        // 0:磁铁在芯片上方顺时针旋转（B 超前 A 1/4 周期），角度递增(如图 4)
        // 磁铁在芯片上方逆时针旋转（B 滞后 A 1/4 周期），角度递减(如图 5)
        // 1:磁铁在芯片上方逆时针旋转（B 超前 A 1/4 周期），角度递增
        // 磁铁在芯片上方顺时针旋转（B 滞后 A 1/4 周期），角度递减
        uint8_t is_ccw:1;
        uint8_t :2;
    };

    //0x43,0x44,
    
    //0x46,0x47

    //0x48
    struct [[nodiscard]] R8_Hysteresis:public Reg8<> {
        uint8_t :5;
        Hysteresis hysteresis:3;
    };

    //0x4a
    //0x4c
    //0x4d
};


};
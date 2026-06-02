#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/spi/spidrv.hpp"

namespace ymd::drivers{

struct [[nodiscard]] GD30AD3344_Prelude{

    // DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    using Error = hal::HalError;

    template<typename T = void>
    using IResult = Result<T, Error>;


    // 输入多路复用器 MUX[2:0]
    enum class [[nodiscard]] Mux : uint16_t {
        P0N1       = 0b000,  // AIN0+ AIN1- (默认)
        P0N3       = 0b001,  // AIN0+ AIN3-
        P1N3       = 0b010,  // AIN1+ AIN3-
        P2N3       = 0b011,  // AIN2+ AIN3-
        P0GND      = 0b100,  // AIN0+ GND-
        P1GND      = 0b101,  // AIN1+ GND-
        P2GND      = 0b110,  // AIN2+ GND-
        P3GND      = 0b111,  // AIN3+ GND-
    };

    // 可编程增益放大器 PGA[2:0]
    enum class [[nodiscard]] Pga : uint16_t {
        _6_144V    = 0b000,  // ±6.144V
        _4_096V    = 0b001,  // ±4.096V
        _2_048V    = 0b010,  // ±2.048V (默认)
        _1_024V    = 0b011,  // ±1.024V
        _0_512V    = 0b100,  // ±0.512V
        _0_256V    = 0b101,  // ±0.256V
        _0_064V    = 0b110,  // ±0.064V
    };

    // 工作模式 MODE[1]
    enum class [[nodiscard]] Mode : uint16_t {
        Continuous = 0b0,    // 连续转换模式
        SingleShot = 0b1,    // 掉电/单次模式 (默认)
    };

    // 数据速率 DR[2:0]
    enum class [[nodiscard]] DataRate : uint16_t {
        _6_25Hz   = 0b000,  // 6.25SPS
        _12_5Hz   = 0b001,  // 12.5SPS
        _25Hz     = 0b010,  // 25SPS
        _50Hz     = 0b011,  // 50SPS
        _100Hz    = 0b100,  // 100SPS (默认)
        _250Hz    = 0b101,  // 250SPS
        _500Hz    = 0b110,  // 500SPS
        _1000Hz   = 0b111,  // 1000SPS
    };

    // 上拉使能 PULL_UP_EN[3]
    enum class [[nodiscard]] PullUpEn : uint16_t {
        Disable    = 0b0,    // 禁用上拉
        Enable     = 0b1,    // 启用上拉 (默认)
    };

    // 无操作控制 NOP[1:0]
    enum class [[nodiscard]] Nop : uint16_t {
        NOP_00     = 0b00,   // 无效，不更新
        Valid      = 0b01,   // 有效，更新配置 (默认)
        NOP_10     = 0b10,   // 无效，不更新
        NOP_11     = 0b11,   // 无效，不更新
    };

    // 单次转换启动 OS[15]
    enum class [[nodiscard]] Os : uint16_t {
        NoEffect   = 0b0,    // 无作用
        Start      = 0b1,    // 启动单次转换
    };


    struct [[nodiscard]] ConvCode final{
        int16_t bits;
    };


    /* Register 0x01 (CONFIG) definition
    * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    * |  Bit 15  |  Bit 14  |  Bit 13  |  Bit 12  |  Bit 11  |  Bit 10  |   Bit 9  |   Bit 8  |   Bit 7  |   Bit 6  |   Bit 5  |   Bit 4  |   Bit 3  |   Bit 2  |   Bit 1  |   Bit 0  |
    * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    * |    SS    |            MUX[2:0]            |            PGA[2:0]            |   MODE   |             DR[2:0]            | RESERVED |PULL_UP_EN|       NOP[1:0]      | RESERVED |
    * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    */

    struct [[nodiscard]] ConfCode final{
        uint16_t __resv0__ : 1;        // Bit 0: RESERVED
        Nop nop : 2;                   // Bit 2:1: NOP[1:0]
        PullUpEn pull_up_en : 1;       // Bit 3: PULL_UP_EN
        uint16_t __resv1__ : 1;        // Bit 4: RESERVED
        DataRate dr : 3;               // Bit 7:5: DR[2:0]
        Mode mode : 1;                 // Bit 8: MODE
        Pga pga : 3;                   // Bit 11:9: PGA[2:0]
        Mux mux : 3;                   // Bit 14:12: MUX[2:0]
        Os os : 1;                     // Bit 15: OS (Start Single Conversion)

        static constexpr ConfCode from_default(){
            return std::bit_cast<ConfCode>(uint16_t(0x058b));
        }

        [[nodiscard]] constexpr uint16_t to_u16() const {
            return std::bit_cast<uint16_t>(*this);
        }
    };

    static_assert(sizeof(ConfCode) == 2);

};



}


#pragma once



#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/conn/spi/spidrv.hpp"


#include "drivers/encoder/encoder.hpp"


namespace ymd::drivers{

struct TMR3111_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;

    enum class [[nodiscard]] OpCode:uint8_t{
        ProgramEE = 0b001,
        WriteReg = 0b101,
        ReadReg = 0b110,
        ChangeMode = 0b111,
        ReadAngle = 0b011
    };

    static constexpr uint32_t make_txcode(
        const OpCode op_code, 
        const uint8_t arg1, 
        const uint16_t arg2
    ){
        uint32_t sum = 0;
        sum |= static_cast<uint32_t>(op_code) << (32 - 3);
        sum |= static_cast<uint32_t>(arg1) << (32 - 11);
        sum |= static_cast<uint32_t>(arg2);
        return sum;
    }

    struct [[nodiscard]] AngleCode final{
        uint32_t bits;

        [[nodiscard]] constexpr bool is_error() const noexcept {return bits & 0x01;}
        [[nodiscard]] constexpr uint32_t angle23() const noexcept {
            constexpr uint32_t MASK = ((1 << 23) - 1);
            return (bits >> 5) & MASK;
        }


        [[nodiscard]] constexpr uint8_t checksum4() const noexcept {
            constexpr uint32_t MASK = ((1 << 4) - 1);
            return (bits >> 1) & MASK;
        }

        [[nodiscard]] constexpr uint8_t calc_actual_checksum() const noexcept {
            // 4-bit CRC 检验码，校验范围为 1-bit “0” 后接 23-bit 角度值（总共 24-bit 数据）,CRC 校验多
            // 项式为 x^4 + x^3 + x^2 + 1,初始值为 4'b0011

            const uint32_t data = angle23(); // 23-bit value; bit23 is treated as 0
            uint8_t crc = 0b0011; // initial value
            constexpr uint8_t poly = 0b1101; // x^3 + x^2 + 1 (for polynomial x^4 + x^3 + x^2 + 1)

            for (int i = 23; i >= 0; --i) {
                const uint8_t bit = (data >> i) & 1U;
                crc ^= (bit << 3);
                crc = (crc & 0x08) ? ((crc << 1) ^ poly) : (crc << 1);
                crc &= 0x0F;
            }
            return crc;
        }

        [[nodiscard]] constexpr bool is_crc_valid(){
            //todo
            return true;
        }
    };

};

struct TMR3111_Regset{

};

}


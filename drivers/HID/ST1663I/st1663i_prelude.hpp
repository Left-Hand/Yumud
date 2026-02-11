#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "algebra/vectors/vec2.hpp"
#include <span>

//参考:
    // SPDX-License-Identifier: Apache-2.0
    // https://github.com/ArmSoM/rk3506-rkr4.2-sdk/blob/7066d7b709d07fe21dc1808017ba11cc9987cfcf/rtos/bsp/nuvoton/libraries/nu_packages/TPC/st1663i.h
    // https://github.com/ArmSoM/rk3506-rkr4.2-sdk/blob/7066d7b709d07fe21dc1808017ba11cc9987cfcf/rtos/bsp/nuvoton/libraries/nu_packages/TPC/st1663i.c

namespace ymd::drivers::st1663i{

struct [[nodiscard]] ST1663I_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x55);
    static constexpr auto MAX_I2C_BAUDRATE = 200000;
    static constexpr size_t NUM_MAX_TOUCH_POINTS = 5;
    static constexpr size_t REGISTER_LEN = 1;
};



struct [[nodiscard]] ST1663I_Regs : public ST1663I_Prelude {


    struct alignas(4) [[nodiscard]] PackedPoint{

        struct R8_XY0H{
            uint8_t u8Y0_H: 3;
            uint8_t : 1;
            uint8_t u8X0_H: 3;
            uint8_t is_valid: 1;
        };

        R8_XY0H u8XY0H;

        //012H*n+1 (n=0, 1, ...,4)
        uint8_t u8X0_L;

        //012H*n+2 (n=0, 1, ...,4)
        uint8_t u8Y0_L;

        //012H*n+3 (n=0, 1, ...,4)
        uint8_t u8_z;


        constexpr uint16_t x() const {
            return (u8XY0H.u8X0_H << 5) | u8X0_L;
        }

        constexpr uint16_t y() const {
            return (u8XY0H.u8Y0_H << 5) | u8Y0_L;
        }

        constexpr uint8_t z() const {
            return u8_z;
        }

        bool is_valid() const {
            return u8XY0H.is_valid;
        }

        std::span<const uint8_t, 4> as_bytes() const {
            return std::span<const uint8_t, 4>(reinterpret_cast<const uint8_t*>(std::assume_aligned<4>(this)), 4);
        }

        std::span<uint8_t, 4> as_bytes_mut() {
            return std::span<uint8_t, 4>(reinterpret_cast<uint8_t*>(std::assume_aligned<4>(this)), 4);
        }
    };

    static_assert(sizeof(PackedPoint) == 4);


    struct TouchInfo{
        uint8_t fingers:4;
        uint8_t :4;
    }
};

}
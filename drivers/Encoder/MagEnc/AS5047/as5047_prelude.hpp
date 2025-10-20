#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "core/io/regs.hpp"

namespace ymd::drivers{

struct AS5047_Prelude{

    using RegAddr = uint16_t;
    using Error = EncoderError;

};

struct AS5047_Regs:public AS5047_Prelude{

    // static constexpr RegAddr MAG_ENC_REG_ADDR = 0x3FF;
    struct ErrflReg:public Reg8<>{
        static constexpr RegAddr address = 0x001;
        uint8_t frame_error:1;
        uint8_t invalid_cmd_error:1;
        uint8_t parity_error:1;
        uint8_t :5;
    };

    struct ProgReg:public Reg8<>{
        static constexpr RegAddr address = 0x002;

        uint8_t prog_otp_en:1;
        uint8_t otp_reflash:1;
        uint8_t start_otp_prog:1;
        uint8_t prog_verify:1;
        uint8_t :4;
    };

};

}
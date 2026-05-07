#pragma once


#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

// https://github.com/Infineon/DPS368-Library-Arduino/blob/dps368/src/Dps310.cpp

namespace ymd::drivers::dps368{


enum class Mode:uint8_t{
    IDLE = 0x00,
    CMD_PRS = 0x01,
    CMD_TEMP = 0x02,
    CMD_BOTH = 0x03, // only for DPS422
    CONT_PRS = 0x05,
    CONT_TMP = 0x06,
    CONT_BOTH = 0x07
};


enum class ConfigRegisters:uint8_t{
    TEMP_MR = 0, // temperature measure rate
    TEMP_OSR,    // temperature measurement resolution
    PRS_MR,      // pressure measure rate
    PRS_OSR,     // pressure measurement resolution
    MSR_CTRL,    // measurement control
    FIFO_EN,

    TEMP_RDY,
    PRS_RDY,
    INT_FLAG_FIFO,
    INT_FLAG_TEMP,
    INT_FLAG_PRS,
};

__attribute__((optimize("Ofast")))
static constexpr void get_twos_complement(int32_t *raw, uint8_t length){
	if (*raw & ((uint32_t)1 << (length - 1))){
		*raw -= (uint32_t)1 << length;
	}
}

struct [[nodiscard]] alignas(4) CalibrateCoeffs final{
    int32_t c0_half;
    int32_t c1;
    int32_t c00;
    int32_t c10;
    int32_t c01;
    int32_t c11;
    int32_t c20;
    int32_t c21;
    int32_t c30;

    void init_from_bytes(std::span<const uint8_t, 18> bytes){
        //compose coefficients from bytes content
        c0_half = ((uint32_t)bytes[0] << 4) | (((uint32_t)bytes[1] >> 4) & 0x0F);
        get_twos_complement(&c0_half, 12);
        //c0 is only used as c0*0.5, so c0_half is calculated immediately
        c0_half = c0_half / 2U;


        //now do the same thing for all other coefficients
        c1 = (((uint32_t)bytes[1] & 0x0F) << 8) | (uint32_t)bytes[2];
        get_twos_complement(&c1, 12);
        c00 = ((uint32_t)bytes[3] << 12) | ((uint32_t)bytes[4] << 4) | (((uint32_t)bytes[5] >> 4) & 0x0F);
        get_twos_complement(&c00, 20);
        c10 = (((uint32_t)bytes[5] & 0x0F) << 16) | ((uint32_t)bytes[6] << 8) | (uint32_t)bytes[7];
        get_twos_complement(&c10, 20);

        c01 = ((uint32_t)bytes[8] << 8) | (uint32_t)bytes[9];
        get_twos_complement(&c01, 16);

        c11 = ((uint32_t)bytes[10] << 8) | (uint32_t)bytes[11];
        get_twos_complement(&c11, 16);
        c20 = ((uint32_t)bytes[12] << 8) | (uint32_t)bytes[13];
        get_twos_complement(&c20, 16);
        c21 = ((uint32_t)bytes[14] << 8) | (uint32_t)bytes[15];
        get_twos_complement(&c21, 16);
        c30 = ((uint32_t)bytes[16] << 8) | (uint32_t)bytes[17];
        get_twos_complement(&c30, 16);
    }
}

}
#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/ms5611-pressure-sensor.html

// MIT license
// https://github.com/libdriver/ms5611/blob/main/src/driver_ms5611.c

namespace ymd::drivers{

struct MS5611_Prelude{

// CSB 的反补码 即 CSB 引脚接高电平时， 地址为 1110 110+(读写位)
// CSB 的反补码 即 CSB 引脚接低电平时， 地址为 1110 111+(读写位)
static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1110'110);
static constexpr uint8_t D1_ADDR = 0x48;
static constexpr uint8_t D2_ADDR = 0x58;
static constexpr uint8_t RESET_COMMAND = 0x1e;

struct [[nodiscard]] Coeffs final{
    std::array<uint16_t, 6> c_table;

    struct [[nodiscard]] Product final{
        int64_t off;
        int64_t sens;
        int32_t temp;
        int32_t pressure;

        [[nodiscard]] constexpr float pressure_mbar() const noexcept {
            return static_cast<float>(pressure) / 100.0f;
        }

        [[nodiscard]] constexpr float temperature_c() const noexcept {
            return static_cast<float>(temp) / 100.0f;
        }
    };

    
    struct [[nodiscard]] Intermediate final{
        uint32_t d1;
        int32_t dt;
        int32_t temp;

        int64_t off2;
        int64_t sens2;
    };

    constexpr Intermediate calc_intermediate(const uint32_t d1, const uint32_t d2) const noexcept {
        const int32_t dt = d2 - (c_table[4] << 8);

        const int32_t temp = 2000 + ((static_cast<int64_t>(dt) * static_cast<int64_t>(c_table[5])) >> 23);

        int32_t t2;
        int64_t off2;
        int64_t sens2;

        if (temp < 2000){                                                                                
            t2 = (3 * ((int64_t)dt * (int64_t)dt)) >> 33;                                               
            off2 = 61 * ((int64_t)temp - 2000) * ((int64_t)temp - 2000) / 16;                             
            sens2 = 29 * ((int64_t)temp - 2000) * ((int64_t)temp - 2000) / 16;                      
            if (temp < -1500){
                off2 += 17 * ((int64_t)temp + 1500) * ((int64_t)temp + 1500);                          
                sens2 += 9 * ((int64_t)temp + 1500) * ((int64_t)temp + 1500);                   
            }
        }else{
            t2 = (5 * ((int64_t)dt * (int64_t)dt)) >> 38;                                      
            off2 = 0;                                                                          
            sens2 = 0;                                                                          
        }

        return Intermediate{
            .d1 = d1,
            .dt = dt,
            .temp = temp - t2,
            .off2 = off2,
            .sens2 = sens2
        };
    }
    

    constexpr Product calc_product(const Intermediate intermediate) const noexcept {
        const int64_t off = (static_cast<uint32_t>(c_table[1]) << 16) + 
            ((static_cast<int64_t>(c_table[3]) * static_cast<int64_t>(intermediate.dt)) >> 7) - intermediate.off2;

        const int64_t sens = (static_cast<uint32_t>(c_table[0]) << 15) + 
            ((static_cast<uint32_t>(c_table[2]) * intermediate.dt ) >> 8) - intermediate.sens2;

        const int32_t pressure = ((intermediate.d1 * sens >> 21) - off) >> 15;

        return Product{
            .off = off,
            .sens = sens,
            .temp = intermediate.temp,
            .pressure = pressure
        };
    }
};

struct Osr{
    uint8_t bits;
};

static constexpr Osr MS5611_OSR256 = Osr{0b0000};
static constexpr Osr MS5611_OSR512 = Osr{0b0001};
static constexpr Osr MS5611_OSR1024 = Osr{0b0010};
static constexpr Osr MS5611_OSR2048 = Osr{0b0011};
static constexpr Osr MS5611_OSR4096 = Osr{0b0100};


using CommandId = uint8_t;

struct [[nodiscard]] CommandFactory{

    static constexpr CommandId MS5611_COMMAND_RESET                      = CommandId{0x1E};        /**< reset command */
    static constexpr CommandId MS5611_COMMAND_PRESSURE                   = CommandId{0x40};        /**< d1 convert command */
    static constexpr CommandId MS5611_COMMAND_TEMPERATURE                = CommandId{0x50};        /**< d2 convert command */
    static constexpr CommandId MS5611_COMMAND_ADC_READ                   = CommandId{0x00};        /**< adc read command */
    static constexpr CommandId MS5611_COMMAND_PROM_READ_ADDRESS_0        = CommandId{0xA0};        /**< prom read address 0 command */

    static constexpr CommandId adc_read(){return MS5611_COMMAND_ADC_READ;}

    static constexpr CommandId read_prom(const uint8_t addr){
        return MS5611_COMMAND_PROM_READ_ADDRESS_0 | ((addr & 0b111) << 1);
    }

    static constexpr CommandId convert_d1(const Osr osr){
        return MS5611_COMMAND_PRESSURE | (osr.bits & 0x0f);
    }

    static constexpr CommandId convert_d2(const Osr osr){
        return MS5611_COMMAND_TEMPERATURE | (osr.bits & 0x0f);
    }

    static constexpr CommandId reset(){
        return MS5611_COMMAND_RESET;
    }
};

struct [[nodiscard]] CrcBuilder final{
    using Self = CrcBuilder;

    static constexpr Self from_default(){
        Self self;
        self.checksum = 0;
        return self;
    }


    // https://github.com/libdriver/ms5611/blob/main/src/driver_ms5611.c
    // MIT license
    constexpr Self push_byte(const uint8_t byte) const noexcept {
        Self self = *this;
        for (int32_t n_bit = 8; n_bit > 0; n_bit--){
            const uint16_t rhs = ((byte & 0x8000U) != 0)
                ? 0x3000 : 0x0000;

            self.checksum = (self.checksum << 1) ^ rhs;
        }
        return *this;
    }

    constexpr uint16_t finalize() const noexcept {
        Self self = *this;
        self.checksum = (0x000F & (checksum >> 12));                                /* get rem */
        self.checksum ^= 0x00;  
        return self.checksum;
    }

private:
    uint16_t checksum;
};


enum class [[nodiscard]] Error_Kind{
    ChipIdMismatch,
    NoPressure
};

DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

template<typename T = void>
using IResult = Result<T, Error>;


};




}
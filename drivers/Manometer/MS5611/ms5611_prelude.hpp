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
static constexpr auto I2C_DEFAULT_ADDR = hal::I2cSlaveAddr<7>::from_u7(0xee >> 1);
static constexpr uint8_t D1_ADDR = 0x48;
static constexpr uint8_t D2_ADDR = 0x58;
static constexpr uint8_t RESET_COMMAND = 0x1e;

struct [[nodiscard]] Coeffs final{
    std::array<uint16_t, 6> c_table;

    struct [[nodiscard]] Intermediate final{
        uint32_t d1;
        int32_t dt;
        int32_t temp;

        int64_t off2;
        int64_t sens2;
    };

    constexpr Intermediate calc_intermediate(const uint32_t d1, const uint32_t d2) const{
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
    
    struct [[nodiscard]] Product final{
        int64_t off;
        int64_t sens;
        int32_t temp;
        int32_t pressure;

        [[nodiscard]] constexpr float pressure_mbar() const {
            return static_cast<float>(pressure) / 100.0f;
        }

        [[nodiscard]] constexpr float temperature_c() const {
            return static_cast<float>(temp) / 100.0f;
        }
    };

    constexpr Product calc_product(const Intermediate intermediate) const {
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


using Command = uint8_t;

struct [[nodiscard]] CommandFactory{

    static constexpr Command MS5611_COMMAND_RESET                      = Command{0x1E};        /**< reset command */
    static constexpr Command MS5611_COMMAND_PRESSURE                   = Command{0x40};        /**< d1 convert command */
    static constexpr Command MS5611_COMMAND_TEMPERATURE                = Command{0x50};        /**< d2 convert command */
    static constexpr Command MS5611_COMMAND_ADC_READ                   = Command{0x00};        /**< adc read command */
    static constexpr Command MS5611_COMMAND_PROM_READ_ADDRESS_0        = Command{0xA0};        /**< prom read address 0 command */

    static constexpr Command adc_read(){return MS5611_COMMAND_ADC_READ;}

    static constexpr Command read_prom(const uint8_t addr){
        return MS5611_COMMAND_PROM_READ_ADDRESS_0 | ((addr & 0b111) << 1);
    }

    static constexpr Command convert_d1(const Osr osr){
        return MS5611_COMMAND_PRESSURE | (osr.bits & 0x0f);
    }

    static constexpr Command convert_d2(const Osr osr){
        return MS5611_COMMAND_TEMPERATURE | (osr.bits & 0x0f);
    }

    static constexpr Command reset(){
        return MS5611_COMMAND_RESET;
    }
};

struct [[nodiscard]] CrcBuilder final{
    using Self = CrcBuilder;

    uint16_t n_rem;

    static constexpr Self from_default(){
        return Self{.n_rem = 0};
    }


    // https://github.com/libdriver/ms5611/blob/main/src/driver_ms5611.c
    // MIT license
    constexpr Self push_byte(const uint8_t byte) const {
        Self self = *this;
        for (int32_t n_bit = 8; n_bit > 0; n_bit--){
            const uint16_t rhs = ((byte & 0x8000U) != 0)
                ? 0x3000 : 0x0000;

            self.n_rem = (self.n_rem << 1) ^ rhs;
        }
        return *this;
    }

    constexpr uint16_t finalize() const {
        Self self = *this;
        self.n_rem = (0x000F & (n_rem >> 12));                                /* get rem */
        self.n_rem ^= 0x00;  
        return self.n_rem;
    }
};

};

}
#pragma once

#include "drivers/device_defs.h"
#include "sys/math/real.hpp"

namespace ymd::drivers{

class AD9833{
public:
    enum class WaveMode:uint8_t{
        SINUS    = ((0 << 5) | (0 << 1) | (0 << 3)),
        TRIANGLE = ((0 << 5) | (1 << 1) | (0 << 3)),
        MSB      = ((1 << 5) | (0 << 1) | (1 << 3)),
        MSB2     = ((1 << 5) | (0 << 1) | (0 << 3))
    };
protected:
    SpiDrv spi_drv;
    Gpio & fsync_gpio;
    void writeData(uint16_t data); 
public:
    AD9833(
        const SpiDrv & _spi_drv,
        Gpio & _fsync_gpio
    ):
        spi_drv(_spi_drv),
        fsync_gpio(_fsync_gpio){;
    }

    AD9833(
        SpiDrv && _spi_drv,
        Gpio & _fsync_gpio
    ):
        spi_drv(std::move(_spi_drv)),
        fsync_gpio(_fsync_gpio){;
    }

    void init(uint16_t freq_reg, real_t freq, uint16_t phase_reg, uint16_t phase, WaveMode wave_mode);
    void reset();
    void setFreq(uint16_t freq_reg, real_t freq, bool reset);
    void setPhase(uint16_t phase_reg, uint16_t phase);
    void setWave(WaveMode wave_mode, uint16_t freq_reg, uint16_t phase_reg);
};
}
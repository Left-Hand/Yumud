#pragma once

#include "core/io/regs.hpp"
#include "core/math/real.hpp"

#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio_intf.hpp"

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
    hal::SpiDrv spi_drv_;
    hal::GpioIntf & fsync_pin_;
    void write_data(uint16_t data); 
public:
    AD9833(
        const hal::SpiDrv & spi_drv,
        hal::GpioIntf & fsync_gpio
    ):
        spi_drv_(spi_drv),
        fsync_pin_(fsync_gpio){;
    }

    AD9833(
        hal::SpiDrv && spi_drv,
        hal::GpioIntf & fsync_gpio
    ):
        spi_drv_(std::move(spi_drv)),
        fsync_pin_(fsync_gpio){;
    }

    void init(uint16_t freq_reg, real_t freq, uint16_t phase_reg, uint16_t phase, WaveMode wave_mode);
    void reset();
    void setFreq(uint16_t freq_reg, real_t freq, bool reset);
    void setPhase(uint16_t phase_reg, uint16_t phase);
    void setWave(WaveMode wave_mode, uint16_t freq_reg, uint16_t phase_reg);
};
}
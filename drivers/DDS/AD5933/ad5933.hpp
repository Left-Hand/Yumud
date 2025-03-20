#pragma once

/**
 * @file AD5933.hpp
 * @brief Library code for AD5933
 *
 * Library code for AD5933. Referenced the datasheet and code found at
 * https://github.com/WuMRC/drive
 *
 * @author Michael Meli
 */



#include "drivers/device_defs.h"
#include "core/math/real.hpp"

namespace ymd::drivers{

class AD5933 {
protected:
    hal::I2cDrv _i2c_drv;
public:
    scexpr uint8_t default_i2c_addr = 0x0D;
public:
    AD5933(const hal::I2cDrv & i2c_drv):_i2c_drv(i2c_drv){;}
    AD5933(hal::I2cDrv && i2c_drv):_i2c_drv(std::move(i2c_drv)){;}
    AD5933(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):_i2c_drv(hal::I2cDrv(i2c, i2c_addr)){;}
    bool reset(void);

    // Temperature measuring
    bool enableTemperature(uint8_t);
    real_t getTemperature(void);

    // Clock
    bool setClockSource(uint8_t);
    bool setInternalClock(bool);
    bool setSettlingCycles(int);

    // Frequency sweep configuration
    bool setStartFrequency(uint32_t);
    bool setIncrementFrequency(uint32_t);
    bool setNumberIncrements(uint32_t);

    // Gain configuration
    bool setPGAGain(uint8_t);

    // Excitation range configuration
    bool setRange(uint8_t);

    // Read registers
    uint8_t read_register(uint8_t);
    uint8_t write_register(uint8_t);
    uint8_t readStatusRegister(void);
    int readControlRegister(void);

    // Impedance data
    bool getComplexData(int16_t & real, int16_t & imag);

    // Set control mode register (CTRL_REG1)
    bool setControlMode(uint8_t);

    // Power mode
    bool setPowerMode(uint8_t);

    // Perform frequency sweeps
    bool frequencySweep(
        int16_t * real,
        int16_t * imag, 
        const size_t n
    );

    bool calibrate(
        const real_t *gain,
        const int *phase,
        int ref, int n
    );

    bool calibrate(
        const real_t *gain,
        const int *phase,
        int16_t *real,
        int16_t *imag,
        int ref, int n
    );
private:
    // Private data
    const uint32_t clockSpeed = 16776000;

    // Sending/Receiving uint8_t method, for easy re-use
    bool getByte(uint8_t, uint8_t*);
    bool sendByte(uint8_t, uint8_t);
};

};
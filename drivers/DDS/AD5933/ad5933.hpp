#pragma once
/**
 * Includes
 */
#include "../drivers/device_defs.h"
#include "real.hpp"


/**
 * AD5933 Library class
 *  Contains mainly functions for interfacing with the AD5933.
 */
class AD5933 {
protected:
    I2cDrv i2c_drv;
public:
    static constexpr uint8_t default_addr = 0x0D;
public:
    AD5933(I2cDrv & _i2c_drv):i2c_drv(_i2c_drv){;}
    AD5933(I2cDrv && _i2c_drv):i2c_drv(_i2c_drv){;}
    AD5933(I2c & bus):i2c_drv(bus, default_addr){;}
    // Reset the board
    bool reset(void);

    // Temperature measuring
    bool enableTemperature(uint8_t);
    real_t getTemperature(void);

    // Clock
    bool setClockSource(uint8_t);
    bool setInternalClock(bool);
    bool setSettlingCycles(int);

    // Frequency sweep configuration
    bool setStartFrequency(unsigned long);
    bool setIncrementFrequency(unsigned long);
    bool setNumberIncrements(unsigned int);

    // Gain configuration
    bool setPGAGain(uint8_t);

    // Excitation range configuration
    bool setRange(uint8_t);

    // Read registers
    uint8_t readRegister(uint8_t);
    uint8_t readStatusRegister(void);
    int readControlRegister(void);

    // Impedance data
    bool getComplexData(int*, int*);

    // Set control mode register (CTRL_REG1)
    bool setControlMode(uint8_t);

    // Power mode
    bool setPowerMode(uint8_t);

    // Perform frequency sweeps
    bool frequencySweep(int real[], int imag[], int);
    bool calibrate(real_t gain[], int phase[], int ref, int n);
    bool calibrate(real_t gain[], int phase[], int real[],
                            int imag[], int ref, int n);
private:
    // Private data
    const unsigned long clockSpeed = 16776000;

    // Sending/Receiving uint8_t method, for easy re-use
    bool getByte(uint8_t, uint8_t*);
    bool sendByte(uint8_t, uint8_t);
};
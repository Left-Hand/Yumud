/**
 * @file AD5933.cpp
 * @brief Library code for AD5933
 *
 * Library code for AD5933. Referenced the datasheet and code found at
 * https://github.com/WuMRC/drive
 *
 * @author Michael Meli
 */

#include "AD5933.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd;

// void write_register(const uint8_t ){
//     TODO();
// }

/**
 * AD5933 Register Map
 *  Datasheet p23
 */
// Device address and address pointer
#define AD5933_ADDR     (0x0D)
#define ADDR_PTR        (0xB0)
// Control Register
#define CTRL_REG1       (0x80)
#define CTRL_REG2       (0x81)
// Start Frequency Register
#define START_FREQ_1    (0x82)
#define START_FREQ_2    (0x83)
#define START_FREQ_3    (0x84)
// Frequency increment register
#define INC_FREQ_1      (0x85)
#define INC_FREQ_2      (0x86)
#define INC_FREQ_3      (0x87)
// Number of increments register
#define NUM_INC_1       (0x88)
#define NUM_INC_2       (0x89)
// Number of settling time cycles register
#define NUM_SCYCLES_1   (0x8A)
#define NUM_SCYCLES_2   (0x8B)
// Status register
#define STATUS_REG      (0x8F)
// Temperature data register
#define TEMP_DATA_1     (0x92)
#define TEMP_DATA_2     (0x93)
// Real data register
#define REAL_DATA_1     (0x94)
#define REAL_DATA_2     (0x95)
// Imaginary data register
#define IMAG_DATA_1     (0x96)
#define IMAG_DATA_2     (0x97)

/**
 * Constants
 *  Constants for use with the AD5933 library class.
 */
// Temperature measuring
#define TEMP_MEASURE    (CTRL_TEMP_MEASURE)
#define TEMP_NO_MEASURE (CTRL_NO_OPERATION)
// Clock sources
#define CLOCK_INTERNAL  (CTRL_CLOCK_INTERNAL)
#define CLOCK_EXTERNAL  (CTRL_CLOCK_EXTERNAL)
// PGA gain options
#define PGA_GAIN_X1     (CTRL_PGA_GAIN_X1)
#define PGA_GAIN_X5     (CTRL_PGA_GAIN_X5)
// Power modes
#define POWER_STANDBY   (CTRL_STANDBY_MODE)
#define POWER_DOWN      (CTRL_POWER_DOWN_MODE)
#define POWER_ON        (CTRL_NO_OPERATION)
// I2C result success/fail
#define I2C_RESULT_SUCCESS       (0)
#define I2C_RESULT_DATA_TOO_LONG (1)
#define I2C_RESULT_ADDR_NAK      (2)
#define I2C_RESULT_DATA_NAK      (3)
#define I2C_RESULT_OTHER_FAIL    (4)
// Control output voltage range options
#define CTRL_OUTPUT_RANGE_1		(0b00000000)
#define CTRL_OUTPUT_RANGE_2		(0b00000110)
#define CTRL_OUTPUT_RANGE_3		(0b00000100)
#define CTRL_OUTPUT_RANGE_4		(0b00000010)
// Control register options
#define CTRL_NO_OPERATION       (0b00000000)
#define CTRL_INIT_START_FREQ    (0b00010000)
#define CTRL_START_FREQ_SWEEP   (0b00100000)
#define CTRL_INCREMENT_FREQ     (0b00110000)
#define CTRL_REPEAT_FREQ        (0b01000000)
#define CTRL_TEMP_MEASURE       (0b10010000)
#define CTRL_POWER_DOWN_MODE    (0b10100000)
#define CTRL_STANDBY_MODE       (0b10110000)
#define CTRL_RESET              (0b00010000)
#define CTRL_CLOCK_EXTERNAL     (0b00001000)
#define CTRL_CLOCK_INTERNAL     (0b00000000)
#define CTRL_PGA_GAIN_X1        (0b00000001)
#define CTRL_PGA_GAIN_X5        (0b00000000)
// Status register options
#define STATUS_TEMP_VALID       (0x01)
#define STATUS_DATA_VALID       (0x02)
#define STATUS_SWEEP_DONE       (0x04)
#define STATUS_ERROR            (0xFF)
// Frequency sweep parameters
#define SWEEP_DELAY             (1)




/**
 * Request to read a uint8_t from the AD5933.
 *
 * @param address Address of register requesting data from
 * @param value Pointer to a uint8_t where the return value should be stored, or
 *        where the error code will be stored if fail.
 * @return Success or failure
 */
bool AD5933::getByte(uint8_t address, uint8_t *value) {
    i2c_drv_.write_reg(address, *value).unwrap();
    return true;
}

/**
 * Write a uint8_t to a register on the AD5933.
 *
 * @param address The register address to write to
 * @param value The uint8_t to write to the address
 * @return Success or failure of transmission
 */
bool AD5933::sendByte(uint8_t address, uint8_t value) {
    i2c_drv_.write_reg(address, value).unwrap();
    return true;
}

/**
 * Set the control mode register, CTRL_REG1. This is the register where the
 * current command needs to be written to so this is used a lot.
 *
 * @param mode The control mode to set
 * @return Success or failure
 */
bool AD5933::setControlMode(uint8_t mode) {
    // Get the current value of the control register
    uint8_t val;
    if (!getByte(CTRL_REG1, &val))
        return false;

    // Wipe out the top 4 bits...mode bits are bits 5 through 8.
    val &= 0x0F;

    // Set the top 4 bits appropriately
    val |= mode;

    // Write back to the register
    return sendByte(CTRL_REG1, val);
}

/**
 * Reset the AD5933. This interrupts a sweep if one is running, but the start
 * frequency, number of increments, and frequency increment register contents
 * are not overwritten, but an initialize start frequency command is required
 * to restart a frequency sweep.
 *
 * @return Success or failure
 */
bool AD5933::reset() {
    // Get the current value of the control register
    uint8_t val;
    if (!getByte(CTRL_REG2, &val))
        return false;

    // Set bit D4 for restart
    val |= CTRL_RESET;

    // Send uint8_t back
    return sendByte(CTRL_REG2, val);
}

/**
 * Set enable temperature measurement. This interferes with frequency sweep
 * operation, of course.
 *
 * @param enable Option to enable to disable temperature measurement.
 * @return Success or failure
 */
bool AD5933::enableTemperature(uint8_t enable) {
    // If enable, set temp measure bits. If disable, reset to no operation.
    if (enable == TEMP_MEASURE) {
        return setControlMode(CTRL_TEMP_MEASURE);
    } else {
        return setControlMode(CTRL_NO_OPERATION);
    }
}

/**
 * Get the temperature reading from the AD5933. Waits until a temperature is
 * ready. Also ensures temperature measurement mode is active.
 *
 * @return The temperature in celcius, or -1 if fail.
 */
real_t AD5933::getTemperature() {
    // Set temperature mode
    if (enableTemperature(TEMP_MEASURE)) {
        // Wait for a valid temperature to be ready
        while((readStatusRegister() & STATUS_TEMP_VALID) != STATUS_TEMP_VALID) ;

        // Read raw temperature from temperature registers
        uint8_t rawTemp[2];
        if (getByte(TEMP_DATA_1, &rawTemp[0]) &&
            getByte(TEMP_DATA_2, &rawTemp[1]))
        {
            // Combine raw temperature uint8_ts into an interger. The ADC
            // returns a 14-bit 2's C value where the 14th bit is a sign
            // bit. As such, we only need to keep the bottom 13 bits.
            int rawTempVal = (rawTemp[0] << 8 | rawTemp[1]) & 0x1FFF;

            // Convert into celcius using the formula given in the
            // datasheet. There is a different formula depending on the sign
            // bit, which is the 5th bit of the uint8_t in TEMP_DATA_1.
            if ((rawTemp[0] & (1<<5)) == 0) {
                return real_t(rawTempVal) / 32;
            } else {
                return real_t(rawTempVal - 16384) / 32;
            }
        }
    }
    return -1;
}


/**
 * Set the color source. Choices are between internal and external.
 *
 * @param source Internal or External clock
 * @return Success or failure
 */
bool AD5933::setClockSource(uint8_t source) {
    // Determine what source was selected and set it appropriately
    switch (source) {
        case CLOCK_EXTERNAL:
            return sendByte(CTRL_REG2, CTRL_CLOCK_EXTERNAL);
        case CLOCK_INTERNAL:
            return sendByte(CTRL_REG2, CTRL_CLOCK_INTERNAL);
        default:
            return false;
    }
}

/**
 * Set the clock source to internal or not.
 *
 * @param internal Whether or not to set the clock source as internal.
 * @return Success or failure
 */
bool AD5933::setInternalClock(bool internal) {
    // This function is mainly a wrapper for setClockSource()
    if (internal)
        return setClockSource(CLOCK_INTERNAL);
    else
        return setClockSource(CLOCK_EXTERNAL);
}

/**
 * Set the settling time cycles use for frequency sweep.
 *
 * @param time The settling time cycles to set.
 * @return Success or failure
 */
bool AD5933::setSettlingCycles(int time)
{
    int cycles;
    uint8_t settleTime[2], rsTime[2], val;

    settleTime[0] = time & 0xFF;        // LSB - 8B
    settleTime[1] = (time >> 8) & 0xFF; // MSB - 8A

    cycles = (settleTime[0] | (settleTime[1] & 0x1));
    val = (uint8_t)((settleTime[1] & 0x7) >> 1);

    if ((cycles > 0x1FF) || !(val == 0 || val == 1 || val == 3))
    {
        return false;
    }

    if (sendByte(NUM_SCYCLES_1, settleTime[1]) && (sendByte(NUM_SCYCLES_2, settleTime[0])))
    {
        // Reading values which wrote above
        if (getByte(NUM_SCYCLES_1, &rsTime[1]) && getByte(NUM_SCYCLES_2, &rsTime[0]))
        {
            //checking settling time which send and then read both are same or not
            if ((settleTime[0] == rsTime[0]) && (settleTime[1] == rsTime[1]))
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * Set the start frequency for a frequency sweep.
 *
 * @param start The initial frequency.
 * @return Success or failure
 */
bool AD5933::setStartFrequency(uint32_t start) {
    // Page 24 of the Datasheet gives the following formula to represent the
    // start frequency.
    // TODO: Precompute for better performance if we want to keep this constant.

    uint32_t freqHex = (start / clockSpeed / 4)*((1 << 27));
    if (freqHex > 0xFFFFFF) {
        return false;   // overflow
    }

    // freqHex should be a 24-bit value. We need to break it up into 3 uint8_ts.
    uint8_t highByte = (freqHex >> 16) & 0xFF;
    uint8_t midByte = (freqHex >> 8) & 0xFF;
    uint8_t lowByte = freqHex & 0xFF;

    // Attempt sending all three uint8_ts
    return sendByte(START_FREQ_1, highByte) &&
           sendByte(START_FREQ_2, midByte) &&
           sendByte(START_FREQ_3, lowByte);
}

/**
 * Set the increment frequency for a frequency sweep.
 *
 * @param start The frequency to increment by. Max of 0xFFFFFF.
 * @return Success or failure
 */
bool AD5933::setIncrementFrequency(uint32_t increment) {
    // Page 25 of the Datasheet gives the following formula to represent the
    // increment frequency.
    // TODO: Precompute for better performance if we want to keep this constant.
    uint32_t freqHex = (increment / clockSpeed / 4)*((1 << 27));
    if (freqHex > 0xFFFFFF) {
        return false;   // overflow
    }

    // freqHex should be a 24-bit value. We need to break it up into 3 uint8_ts.
    uint8_t highByte = (freqHex >> 16) & 0xFF;
    uint8_t midByte = (freqHex >> 8) & 0xFF;
    uint8_t lowByte = freqHex & 0xFF;

    // Attempt sending all three uint8_ts
    return sendByte(INC_FREQ_1, highByte) &&
           sendByte(INC_FREQ_2, midByte) &&
           sendByte(INC_FREQ_3, lowByte);
}

/**
 * Set the number of frequency increments for a frequency sweep.
 *
 * @param start The number of increments to use. Max 511.
 * @return Success or failure
 */
bool AD5933::setNumberIncrements(uint32_t num) {
    // Check that the number sent in is valid.
    if (num > 511) {
        return false;
    }

    // Divide the 9-bit integer into 2 uint8_ts.
    uint8_t highByte = (num >> 8) & 0xFF;
    uint8_t lowByte = num & 0xFF;

    // Write to register.
    return sendByte(NUM_INC_1, highByte) &&
           sendByte(NUM_INC_2, lowByte);
}

/**
 * Set the PGA gain factor.
 *
 * @param gain The gain factor to select. Use constants or 1/5.
 * @return Success or failure
 */
bool AD5933::setPGAGain(uint8_t gain) {
    // Get the current value of the control register
    uint8_t val;
    if (!getByte(CTRL_REG1, &val))
        return false;

    // Clear out the bottom bit, D8, which is the PGA gain set bit
    val &= 0xFE;

    // Determine what gain factor was selected
    if (gain == PGA_GAIN_X1 || gain == 1) {
        // Set PGA gain to x1 in CTRL_REG1
        val |= PGA_GAIN_X1;
        return sendByte(CTRL_REG1, val);
    } else if (gain == PGA_GAIN_X5 || gain == 5) {
        // Set PGA gain to x5 in CTRL_REG1
        val |= PGA_GAIN_X5;
        return sendByte(CTRL_REG1, val);
    } else {
        return false;
    }
}

/**
 * Read the value of a register.
 *
 * @param reg The address of the register to read.
 * @return The value of the register. Returns 0xFF if can't read it.
 */
uint8_t AD5933::write_register(uint8_t reg) {
    // Read status register and return it's value. If fail, return 0xFF.
    uint8_t val;
    if (getByte(reg, &val)) {
        return val;
    } else {
        return STATUS_ERROR;
    }
}

/**
 * Set the output voltage range.
 * Default value to select in this function is CTRL_OUTPUT_RANGE_1
 *
 * @param range The output voltage range to select.
 * @return Success or failure
 */
bool AD5933::setRange(uint8_t range)
{
    uint8_t val;

    // Get the current value of the control register
    if(!getByte(CTRL_REG1, &val))
    {
        return false;
    }

    // Clear out the bottom bit, D9 and D10, which is the output voltage range set bit
    val &=  0xF9;

    // Determine what output voltage range was selected
    switch (range)
    {
        case CTRL_OUTPUT_RANGE_2:
            // Set output voltage range to 1.0 V p-p typical in CTRL_REG1
            val |= CTRL_OUTPUT_RANGE_2;
            break;

        case CTRL_OUTPUT_RANGE_3:
            // Set output voltage range to 400 mV p-p typical in CTRL_REG1
            val |= CTRL_OUTPUT_RANGE_3;
            break;
        
        case CTRL_OUTPUT_RANGE_4:
            // Set output voltage range to 200 mV p-p typical in CTRL_REG1
            val |= CTRL_OUTPUT_RANGE_4;
            break;

        default:
            // Set output voltage range to 200 mV p-p typical in CTRL_REG1
            val |= CTRL_OUTPUT_RANGE_1;
            break;
    }

    //Write to register
    return sendByte(CTRL_REG1, val);
}

/**
 * Read the value of the status register.
 *
 * @return The value of the status register. Returns 0xFF if can't read it.
 */
uint8_t AD5933::readStatusRegister() {
    return write_register(STATUS_REG);
}

/**
 * Read the value of the control register.
 *
 * @return The value of the control register. Returns 0xFFFF if can't read it.
 */
int AD5933::readControlRegister() {
    return ((write_register(CTRL_REG1) << 8) | write_register(CTRL_REG2)) & 0xFFFF;
}

/**
 * Get a raw complex number for a specific frequency measurement.
 *
 * @param real Pointer to an int that will contain the real component.
 * @param imag Pointer to an int that will contain the imaginary component.
 * @return Success or failure
 */
bool AD5933::getComplexData(int16_t & real, int16_t & imag) {
    // Wait for a measurement to be available
    while ((readStatusRegister() & STATUS_DATA_VALID) != STATUS_DATA_VALID);

    // Read the four data registers.
    // TODO: Do this faster with a block read
    uint8_t realComp[2];
    uint8_t imagComp[2];
    if (getByte(REAL_DATA_1, &realComp[0]) &&
        getByte(REAL_DATA_2, &realComp[1]) &&
        getByte(IMAG_DATA_1, &imagComp[0]) &&
        getByte(IMAG_DATA_2, &imagComp[1]))
    {
        // Combine the two separate uint8_ts into a single 16-bit value and store
        // them at the locations specified.
        real = (int16_t)(((realComp[0] << 8) | realComp[1]) & 0xFFFF);
        imag = (int16_t)(((imagComp[0] << 8) | imagComp[1]) & 0xFFFF);

        return true;
    } else {
        real = -1;
        imag = -1;
        return false;
    }
}

/**
 * Set the power level of the AD5933.
 *
 * @param level The power level to choose. Can be on, standby, or down.
 * @return Success or failure
 */
bool AD5933::setPowerMode(uint8_t level) {
    // Make the appropriate switch. TODO: Does no operation even do anything?
    switch (level) {
        case POWER_ON:
            return setControlMode(CTRL_NO_OPERATION);
        case POWER_STANDBY:
            return setControlMode(CTRL_STANDBY_MODE);
        case POWER_DOWN:
            return setControlMode(CTRL_POWER_DOWN_MODE);
        default:
            return false;
    }
}

/**
 * Perform a complete frequency sweep.
 *
 * @param real An array of appropriate size to hold the real data.
 * @param imag An array of appropriate size to hold the imaginary data.
 * @param n Length of the array (or the number of discrete measurements)
 * @return Success or failure
 */
bool AD5933::frequencySweep(int16_t *real, int16_t *imag, const size_t n) {
    // Begin by issuing a sequence of commands
    // If the commands aren't taking hold, add a brief delay
    if (!(setPowerMode(POWER_STANDBY) &&         // place in standby
         setControlMode(CTRL_INIT_START_FREQ) && // init start freq
         setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
         {
             return false;
         }

    // Perform the sweep. Make sure we don't exceed n.
    size_t i = 0;
    while ((readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE) {
        // Make sure we aren't exceeding the bounds of our buffer
        if (i >= n) {
            return false;
        }

        // Get the data for this frequency point and store it in the array
        if (!getComplexData(real[i], imag[i])) {
            return false;
        }

        // Increment the frequency and our index.
        i++;
        setControlMode(CTRL_INCREMENT_FREQ);
    }

    // Put into standby
    return setPowerMode(POWER_STANDBY);
}

/**
 * Computes the gain factor and phase for each point in a frequency sweep.
 *
 * @param gain An array of appropriate size to hold the gain factors
 * @param phase An array of appropriate size to hold phase data.
 * @param ref The known reference resistance.
 * @param n Length of the array (or the number of discrete measurements)
 * @return Success or failure
 */
bool AD5933::calibrate(
    const real_t *gain, 
    const int *phase,
    int ref, int n
){
    // auto real = (int16_t *)alloca(n*sizeof(int16_t));
    // auto imag = (int16_t *)alloca(n*sizeof(int16_t));
    // int16_t real[n];
    // int16_t imag[n];

    // if (!frequencySweep(real, imag, n)) {
    //     return false;
    // }

    // for (int i = 0; i < n; i++) {
    //     // Calculate gain factor
    //     TODO();
    //     // gain[i] = (real_t(1) / ref) / sqrt(real_t(real[i]) * real_t(real[i]) + real_t(imag[i]) * real_t(imag[i]));

    //     // Calculate phase in degrees
    //     // phase[i] = static_cast<int>(atan2(real_t(imag[i]), real_t(real[i])) * 180 / M_PI);
    // }

    return true;
}

/**
 * Computes the gain factor and phase for each point in a frequency sweep.
 * Also provides the caller with the real and imaginary data.
 *
 * @param gain An array of appropriate size to hold the gain factors
 * @param phase An array of appropriate size to hold the phase data
 * @param real An array of appropriate size to hold the real data
 * @param imag An array of appropriate size to hold the imaginary data.
 * @param ref The known reference resistance.
 * @param n Length of the array (or the number of discrete measurements)
 * @return Success or failure
 */
bool AD5933::calibrate(
    const real_t *gain,
    const int *phase,
    int16_t *real,
    int16_t *imag,
    int ref, int n
) {
    // Perform the frequency sweep
    if (!frequencySweep(real, imag, n)) {
        return false;
    }

    // For each point in the sweep, calculate the gain factor and phase
    for (int i = 0; i < n; i++) {
        TODO("phase");
        // gain[i] = (real_t(1) / ref) / sqrt(real[i] * real[i]  + imag[i] * imag[i]);
    }

    return true;
}
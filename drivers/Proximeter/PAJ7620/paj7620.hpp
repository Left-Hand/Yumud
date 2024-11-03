#ifndef GESTURE_H_
#define GESTURE_H_

#include "../../hal/bus/i2c/i2cdrv.hpp"

#ifdef PAJ7620_DEBUG
#define PAJ7620_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define PAJ7620_DEBUG(...)
#endif


class PAJ7620{

    void GESTURE_Actions(void); /* Sets action flags for gestures */

    void initializeRegisters(void); /* Initializes sensors registers */

    uint8_t gestureInit(void); /* Initialize gesture */

    static uint8_t registerWrite(uint8_t addr, uint8_t cmd); /* Write to addressed register */
    /* Registers address
    * CMD functions data */

    static uint8_t registerRead(uint8_t addr, uint8_t qty, uint8_t data[]); /* Read data from addressed register */
    /* Registers Address
    * Qty - Number of read data
    * data - memory storage */

    enum BankSet_t{
        Bank0 = 0, /* Some of the registers are set in bank 0 */
        Bank1 = 1, /* Registers are set in bank 1 */
    };

};


#endif /* GESTSENS_H_ */
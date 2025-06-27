#pragma once

/** @file
 *
 * This file defines the driver for PAW3805EK-CJV1: Track-On-Glass Mouse Sensor.
 *
 */

/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct PAW3805_Prelude{
    typedef struct
    {
        uint8_t       rw;         // read or write
        uint8_t       regoffset;   // register offset
        uint8_t       value;       // write value
    }PAWSensorRegSeq;

    //////////////////////////////////
    /// Optical Sensor Config for PAW optical sensor
    //////////////////////////////////
    typedef struct
        {
        /// the GPIO pin the CS line is connected to
        uint8_t cs_gpio;

        /// the GPIO pin the motion line is connected to
        uint8_t motion_gpio;

        /// SPI speed to use for the 7050.
        uint32_t spiSpeed;

    } PAWsensor_Config;

    static constexpr uint8_t WRITE_CMD_BIT  = 0x80;

    enum PAWSENSOR_ACT_PROCEDURE_ID
    {
        /// perform spi read and compare
        PAWSENSOR_READCOMPARE            =  1  ,

        /// just read the sensor register via spi interferace, ignore the read result
        PAWSENSOR_READONLY               =  2  ,

        /// write the sensor register
        PAWSENSOR_WRITE                  =  3  ,

        /// compare with previoue read value,
        /// the regoffset will become mask
        PAWSENSOR_COMPARE                  =  4  ,
    };

    PAWsensor_Config  PAW_sensor_config =
    {
        // the CS line is connected to P2
        // uint8_t cs_gpio;
        2,

        // the motion line is connected to P16
        // uint8_t motion_gpio;
        16,

        // SPI speed to use for the PAW.
        //uint32_t spiSpeed;
        1000000,
    };

    //
    // Product ID for PAW3805EK-CJV1
    //
    // Product_ID1 (0x00) = 0x31
    // Product_ID2 (0x01) = 0x61
    //
    //
    static constexpr std::array<PAWSensorRegSeq,2> PAW_IDCheckSequence={
        PAWSensorRegSeq{PAWSENSOR_READCOMPARE,    0x00, 0x31},
        PAWSensorRegSeq{PAWSENSOR_READCOMPARE,    0x01, 0x61}
    };
    #define PAW_IDCHECK_NO    (sizeof(PAW_IDCheckSequence)/sizeof(PAWSensorRegSeq))

    //
    // Software Reset for PAW3805EK-CJV1
    //
    // Configuration (0x06) = 0x80 : full chip reset (to reset all sensor's internal registers and states)
    //
    //
    static constexpr std::array<PAWSensorRegSeq, 1> PAW_resetSequence=
    {
        {PAWSENSOR_WRITE,    0x06, 0x80},
    };
    #define PAW_RESET_NO    (sizeof(PAW_resetSequence)/sizeof(PAWSensorRegSeq))

    //
    // Power down for PAW3805EK-CJV1
    //
    // Configuration (0x06) = 0x08 : power down mode for lowest power consumption
    //
    //
    static constexpr std::array<PAWSensorRegSeq, 1> PAW_powerdownSequence =
    {
        {PAWSENSOR_WRITE,    0x06, 0x08},
    };
    #define PAW_POWERDOWN_NO    (sizeof(PAW_powerdownSequence)/sizeof(PAWSensorRegSeq))

    //
    // Sleep3 mode enable for PAW3805EK-CJV1
    //
    // Configuration (0x06) = 0x20 : Enable Sleep3 mode for power saving
    //
    //
    static constexpr std::array<PAWSensorRegSeq, 1> PAW_sleep3Sequence={
        {PAWSENSOR_WRITE,    0x06, 0x30},
    };
    #define PAW_SLEEP3_NO    (sizeof(PAW_sleep3Sequence)/sizeof(PAWSensorRegSeq))

    //NOTE: QUICK_BURST is DISABLED by default. Due to not able to get XY data.
    #ifdef QUICK_BURST
    //
    // enable Quick Burst for PAW3805EK-CJV1
    //
    // Write_Protect (0x09) = 0x5A: disable write protect first
    // MFIO_Config (0x26) = 0x10: configure to Quick Burst
    // Write_Protect (0x09) = 0x00: enable write protect
    //
    PAWSensorRegSeq PAW_enableQBSequence[]=
    {
        {PAWSENSOR_WRITE,    0x09, 0x5A},
        {PAWSENSOR_WRITE,    0x26, 0x10},
        {PAWSENSOR_WRITE,    0x09, 0x0},
    };

    #define PAW_ENABLE_QB_NO    (sizeof(PAW_enableQBSequence)/sizeof(PAWSensorRegSeq))

    extern void bleapputils_delayUs(uint32_t delay);
    #endif


};

class PAW3805_Phy final:public PAW3805_Prelude{
public:
    PAW3805_Phy(hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}

    PAW3805_Phy(hal::Spi & spi, const hal::SpiSlaveIndex index):
        spi_drv_(hal::SpiDrv(spi, index)){;}

    uint8_t read_reg(uint8_t addr);
    void write_reg(uint8_t addr, uint8_t val);
    void burst_read(uint8_t  *buf, uint8_t bytesToRead);
private:
    hal::SpiDrv spi_drv_;
};

class PAW3805:public PAW3805_Prelude{
public:
    PAW3805(hal::Spi & spi, const hal::SpiSlaveIndex index):phy_(spi, index){;}

    void init(void (*userfn)(void*, uint8_t), void* userdata);
    void enable_Interrupt(bool enabled);
    void getMotion(int16_t *x, int16_t *y);
    void flushMotion(void);
    bool isActive(void);
    void powerdown(void);
    void enable_deep_sleep_mode(void);
private:
    using Phy = PAW3805_Phy;
    Phy phy_;

    // Intr_State   PAWIntr;
    // wiced_timer_t PAW_reset_timer;
    bool  PAWdeviceFound=false;
    bool  PAWActive=false;

    uint8_t read_reg(uint8_t addr);
    void write_reg(uint8_t addr, uint8_t val);
    void burst_read(uint8_t  *buf, uint8_t bytesToRead);
    bool verifyProductId(void);
    void reset(void);
    bool walkRegSequence(const PAWSensorRegSeq *regSeqList, uint8_t regSeqNo);
    void reset_timeout(uint32_t arg);

};

}
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

#include "details/paw3805_prelude.hpp"

namespace ymd::drivers{


class PAW3805:public PAW3805_Prelude{
public:
    PAW3805(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):phy_(spi, index){;}

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
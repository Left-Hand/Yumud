/**
  ******************************************************************************
  * @file    lan8742.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          lan8742.c PHY driver.
  ******************************************************************************
  * @attention
  *
  * 2017 STMicroelectronics.
  * All rights reserved
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 

#pragma once

#include "core/io/regs.hpp"
#include "core/debug/debug.hpp"

namespace ymd::drivers{

class LAN8742 {
protected:
    int32_t ReadReg(uint32_t, uint32_t, uint32_t *){
        TODO();
        return 0;
    }
    int32_t WriteReg(uint32_t, uint32_t, uint32_t){
        TODO();
        return 0;
    }
public:
    uint32_t            DevAddr;
    // uint32_t            Is_Initialized;
    void               *pbuf;

    int32_t init();
    int32_t deinit();
    int32_t disablePowerDownMode();
    int32_t enablePowerDownMode();
    int32_t startAutoNego();
    int32_t getLinkState();
    int32_t setLinkState(uint32_t LinkState);
    int32_t enableLoopbackMode();
    int32_t disableLoopbackMode();
    int32_t enableIT(uint32_t Interrupt);
    int32_t disableIT(uint32_t Interrupt);
    int32_t clearIT(uint32_t Interrupt);
    int32_t getITStatus(uint32_t Interrupt);
};


}


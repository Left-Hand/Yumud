/**
  ******************************************************************************
  * @file    lan8742.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          lan8742.c PHY driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 

#pragma once

#include "drivers/device_defs.h"

namespace ymd::drivers{


  
class LAN8742 {
protected:
    int32_t ReadReg(uint32_t, uint32_t, uint32_t *){
        //TODO
        return 0;
    }
    int32_t WriteReg(uint32_t, uint32_t, uint32_t){
        //TODO
        return 0;
    }
public:
    uint32_t            DevAddr;
    uint32_t            Is_Initialized;
    // lan8742_IOCtx_t     IO;
    void               *pData;

    // int32_t RegisterBusIO(lan8742_IOCtx_t *ioctx);
    int32_t Init();
    int32_t DeInit();
    int32_t DisablePowerDownMode();
    int32_t EnablePowerDownMode();
    int32_t StartAutoNego();
    int32_t GetLinkState();
    int32_t SetLinkState(uint32_t LinkState);
    int32_t EnableLoopbackMode();
    int32_t DisableLoopbackMode();
    int32_t EnableIT(uint32_t Interrupt);
    int32_t DisableIT(uint32_t Interrupt);
    int32_t ClearIT(uint32_t Interrupt);
    int32_t GetITStatus(uint32_t Interrupt);
};


}


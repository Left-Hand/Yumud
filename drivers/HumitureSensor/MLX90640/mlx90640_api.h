/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#pragma once


#include <stdint.h>

#include "hal/bus/i2c/i2cdrv.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"


namespace ymd::drivers{




enum class MLX90640_ErrorKind:uint8_t{
// #define MLX90640_BROKEN_PIXELS_NUM_ERROR 3
// #define MLX90640_OUTLIER_PIXELS_NUM_ERROR 4
// #define MLX90640_BAD_PIXELS_NUM_ERROR 5
// #define MLX90640_ADJACENT_BAD_PIXELS_ERROR 6
// #define MLX90640_EEPROM_DATA_ERROR 7
// #define MLX90640_FRAME_DATA_ERROR 8
// #define MLX90640_MEAS_TRIGGER_ERROR 9
    BrokenPixelsNum = 3,
    OutlierPixelsNum = 4,
    BadPixelsNum = 5,
    AdjacentBadPixels = 6,
    EepromDataErr = 7,
    FrameDataErr = 8,
    MeasTriggerErr = 9
};

DEF_DERIVE_DEBUG(MLX90640_ErrorKind)
DEF_ERROR_SUMWITH_HALERROR(MLX90640_Error, MLX90640_ErrorKind)




#define SCALEALPHA 0.000001
    
typedef struct
    {
        int16_t kVdd;
        int16_t vdd25;
        float KvPTAT;
        float KtPTAT;
        uint16_t vPTAT25;
        float alphaPTAT;
        int16_t gainEE;
        float tgc;
        float cpKv;
        float cpKta;
        uint8_t resolutionEE;
        uint8_t calibrationModeEE;
        float KsTa;
        float ksTo[5];
        int16_t ct[5];
        uint16_t alpha[768];    
        uint8_t alphaScale;
        int16_t offset[768];    
        int8_t kta[768];
        uint8_t ktaScale;    
        int8_t kv[768];
        uint8_t kvScale;
        float cpAlpha[2];
        int16_t cpOffset[2];
        float ilChessC[3]; 
        uint16_t brokenPixels[5];
        uint16_t outlierPixels[5];  
} paramsMLX90640;


Result<void, MLX90640_Error> MLX90640_DumpEE(uint8_t slaveAddr, uint16_t *eeData);
Result<void, MLX90640_Error> MLX90640_SynchFrame(uint8_t slaveAddr);
Result<uint16_t, MLX90640_Error> MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData);
Result<void, MLX90640_Error> MLX90640_ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);

float MLX90640_GetTa(uint16_t *frameData, const paramsMLX90640 *params);
void MLX90640_GetImage(uint16_t *frameData, const paramsMLX90640 *params, float *result);
void MLX90640_CalculateTo(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result);
Result<void, MLX90640_Error> MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution);
Result<uint16_t, MLX90640_Error> MLX90640_GetCurResolution(uint8_t slaveAddr);
Result<void, MLX90640_Error> MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate);   
Result<uint16_t, MLX90640_Error> MLX90640_GetRefreshRate(uint8_t slaveAddr);  
int MLX90640_GetSubPageNumber(uint16_t *frameData);
Result<uint16_t, MLX90640_Error> MLX90640_GetCurMode(uint8_t slaveAddr); 
Result<void, MLX90640_Error> MLX90640_SetInterleavedMode(uint8_t slaveAddr);
Result<void, MLX90640_Error> MLX90640_SetChessMode(uint8_t slaveAddr);
void MLX90640_BadPixelsCorrection(uint16_t *pixels, float *to, int mode, paramsMLX90640 *params);


extern void MLX90640_I2CInit(void);
extern Result<void, MLX90640_Error> MLX90640_I2CGeneralReset(void);
extern Result<void, MLX90640_Error> MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);
extern Result<void, MLX90640_Error> MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data);
extern void MLX90640_I2CFreqSet(int freq);
}
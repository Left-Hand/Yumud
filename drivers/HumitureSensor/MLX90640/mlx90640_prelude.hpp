#pragma once

#include <stdint.h>

#include "hal/bus/i2c/i2cdrv.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

namespace ymd::drivers{

static constexpr size_t MLX90640_LINE_NUM  =  24;
static constexpr size_t MLX90640_COLUMN_NUM  =  32;
static constexpr size_t MLX90640_PIXEL_NUM   = MLX90640_LINE_NUM * MLX90640_COLUMN_NUM;
static constexpr size_t MLX90640_AUX_NUM  =  64;
static constexpr size_t MLX90640_EEPROM_DUMP_NUM   = MLX90640_PIXEL_NUM + MLX90640_AUX_NUM;

struct MLX90640_Coeffs{
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
    uint16_t alpha[MLX90640_PIXEL_NUM];    
    int16_t offset[MLX90640_PIXEL_NUM];    
    int8_t kta[MLX90640_PIXEL_NUM];
    int8_t kv[MLX90640_PIXEL_NUM];
    uint8_t alphaScale;
    uint8_t ktaScale;    
    uint8_t kvScale;
    float cpAlpha[2];
    int16_t cpOffset[2];
    float ilChessC[3]; 
    uint16_t brokenPixels[5];
    uint16_t outlierPixels[5];  
} ;


struct MLX90640_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x33);

    enum class ErrorKind:uint8_t{
        DataNotReady,
        BrokenPixelsNum = 3,
        OutlierPixelsNum = 4,
        BadPixelsNum = 5,
        AdjacentBadPixels = 6,
        EepromDataErr = 7,
        FrameDataErr = 8,
        MeasTriggerErr = 9
    };

    DEF_FRIEND_DERIVE_DEBUG(ErrorKind)

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)



    enum class DataRate:uint8_t{
        _0_5Hz = 0,
        _1Hz = 1,
        _2Hz = 2,
        _4Hz = 3,
        _8Hz = 4,
        _16Hz = 5,
        _32Hz = 6,
        _64Hz = 7
    };
};

}
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
#include "mlx90640.hpp"
#include <algorithm>
#include <math.h>

using namespace ymd;


namespace ymd::drivers{

using Error = MLX90640::Error;

template<typename T = void>
using IResult = Result<T, Error>;

#define BIT_MASK(x) (1UL << (x))
// #define REG_MASK(sbit,nbits) ~((~(~0UL << (nbits))) << (sbit))
static constexpr uint16_t REG_MASK(uint16_t sbit, uint16_t nbits) {
    // return ~((~(~0UL << (nbits))) << (sbit));
    const auto m0 = ~(~0UL << nbits);
    return static_cast<uint16_t>(~(m0 << sbit));
}

static constexpr uint16_t EEPROM_START_ADDRESS = 0x2400;
static constexpr uint16_t PIXEL_DATA_START_ADDRESS = 0x0400;
static constexpr uint16_t AUX_DATA_START_ADDRESS = 0x0700;
static constexpr uint16_t STATUS_REG = 0x8000;
static constexpr uint16_t MS_BYTE_MASK = 0xFF00;
static constexpr uint16_t LS_BYTE_MASK = 0x00FF;
static constexpr uint16_t MSBITS_6_MASK = 0xFC00;
static constexpr uint16_t LSBITS_10_MASK = 0x03FF;

static constexpr uint16_t CTRL_REG = 0x800D;
static constexpr uint16_t INIT_STATUS_VALUE = 0x0030;



static constexpr size_t LINE_SIZE  =  32;
static constexpr size_t COLUMN_SIZE  =  24;



static constexpr size_t  STAT_FRAME_MASK = BIT_MASK(0); 
static constexpr size_t  STAT_DATA_READY_MASK = BIT_MASK(3); 
#define GET_FRAME(reg_value) (reg_value & STAT_FRAME_MASK)
#define GET_DATA_READY(reg_value) (reg_value & STAT_DATA_READY_MASK)


static constexpr size_t CTRL_REFRESH_SHIFT  = 7;
static constexpr size_t CTRL_RESOLUTION_SHIFT = 10;
static constexpr size_t CTRL_MEAS_MODE_SHIFT  = 12;

static constexpr uint16_t CTRL_MEAS_MODE_MASK = BIT_MASK(12);
static constexpr uint16_t CTRL_TRIG_READY_MASK = BIT_MASK(15);
static constexpr uint16_t CTRL_REFRESH_MASK = REG_MASK(CTRL_REFRESH_SHIFT,3);
static constexpr uint16_t CTRL_RESOLUTION_MASK  =REG_MASK(CTRL_RESOLUTION_SHIFT,2);


// struct R16_Ctrl{
//     uint16_t 
// };

static constexpr uint8_t NIBBLE1(uint16_t reg16) {return static_cast<uint8_t>(((reg16 & 0x000f)));}
static constexpr uint8_t NIBBLE2(uint16_t reg16) {return static_cast<uint8_t>(((reg16 & 0x00f0) >> 4));}
static constexpr uint8_t NIBBLE3(uint16_t reg16) {return static_cast<uint8_t>(((reg16 & 0x0f00) >> 8));}
static constexpr uint8_t NIBBLE4(uint16_t reg16) {return static_cast<uint8_t>(((reg16 & 0xf000) >> 12));}

static constexpr uint8_t LS_BYTE(const uint16_t x){return static_cast<uint8_t>(x & 0xFF);}
static constexpr uint8_t MS_BYTE(const uint16_t x){return static_cast<uint8_t>(x >> 8);}

static constexpr float VCC_VOLTAGE = 3.3f;

static constexpr float SCALEALPHA = 0.000001f;
static constexpr float ABSOLUTE_ZERO = 273.15f;

// #define POW2(x) pow(2, (double)x) 

// template<std::floating T>
static inline float POW2(const float in){
    return pow(2,in);
}

// template<std::unsigned_integral T>
// // static constexpr inline int POW2(const T in){
// inline int POW2(const T in){
//     // if(in > 30) DEBUG_PRINTLN(in);
//     // return 1 << in;
//     return pow(2, in);
// }



static float get_vdd(const uint16_t *frameData, const MLX90640_Coeffs *params){
    float vdd;
    float resolutionCorrection;

    uint16_t resolutionRAM;  
    
    resolutionRAM = (frameData[832] & ~CTRL_RESOLUTION_MASK) >> CTRL_RESOLUTION_SHIFT;   
    resolutionCorrection = static_cast<float>(POW2(params->resolutionEE)) / POW2(resolutionRAM);
    vdd = (resolutionCorrection * (int16_t)frameData[810] - params->vdd25) / params->kVdd + VCC_VOLTAGE;
    
    return vdd;
}



IResult<> MLX90640::dump_ee(uint16_t *eeData){
    return read_burst(EEPROM_START_ADDRESS, std::span(eeData, MLX90640_EEPROM_DUMP_NUM));
}

static IResult<> CheckAdjacentPixels(uint16_t pix1, uint16_t pix2){
    const uint16_t lp1 = pix1 >> 5;
    const uint16_t lp2 = pix2 >> 5;
    const uint16_t cp1 = pix1 - (lp1 << 5);
    const uint16_t cp2 = pix2 - (lp2 << 5);
    
    int pixPosDif = lp1 - lp2;
    if(pixPosDif > -2 && pixPosDif < 2){
        pixPosDif = cp1 - cp2;
        if(pixPosDif > -2 && pixPosDif < 2){
            return Err(Error::AdjacentBadPixels);
        }

    } 
    
    return Ok();
}



static IResult<> ValidateAuxData(const uint16_t *auxData){
    
    if(auxData[0] == 0x7FFF) return Err(Error::FrameDataErr);    
    
    for(size_t i=8; i<19; i++)
    {
        if(auxData[i] == 0x7FFF) return Err(Error::FrameDataErr);
    }
    
    for(size_t i=20; i<23; i++)
    {
        if(auxData[i] == 0x7FFF) return Err(Error::FrameDataErr);
    }
    
    for(size_t i=24; i<33; i++)
    {
        if(auxData[i] == 0x7FFF) return Err(Error::FrameDataErr);
    }
    
    for(size_t i=40; i<51; i++)
    {
        if(auxData[i] == 0x7FFF) return Err(Error::FrameDataErr);
    }
    
    for(size_t i=52; i<55; i++)
    {
        if(auxData[i] == 0x7FFF) return Err(Error::FrameDataErr);
    }
    
    for(size_t i=56; i<64; i++)
    {
        if(auxData[i] == 0x7FFF) return Err(Error::FrameDataErr);
    }
    
    return Ok();
    
}


static void ExtractVDDParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640){
    int8_t kVdd;
    int16_t vdd25;
    
    kVdd = MS_BYTE(eeData[51]);

    vdd25 = LS_BYTE(eeData[51]);
    vdd25 = ((vdd25 - 256) << 5) - 8192;
    
    mlx90640->kVdd = 32 * kVdd;
    mlx90640->vdd25 = vdd25; 
}


static void ExtractPTATParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640){
    float KvPTAT;
    float KtPTAT;
    int16_t vPTAT25;
    float alphaPTAT;
    
    KvPTAT = (eeData[50] & MSBITS_6_MASK) >> 10;
    if(KvPTAT > 31)
    {
        KvPTAT = KvPTAT - 64;
    }
    KvPTAT = KvPTAT/4096;
    
    KtPTAT = eeData[50] & LSBITS_10_MASK;
    if(KtPTAT > 511)
    {
        KtPTAT = KtPTAT - 1024;
    }
    KtPTAT = KtPTAT/8;
    
    vPTAT25 = eeData[49];
    
    alphaPTAT = (eeData[16] & 0xf000) / static_cast<float>(POW2(14u)) + 8.0f;
    
    mlx90640->KvPTAT = KvPTAT;
    mlx90640->KtPTAT = KtPTAT;    
    mlx90640->vPTAT25 = vPTAT25;
    mlx90640->alphaPTAT = alphaPTAT;   
}


static void ExtractGainParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    mlx90640->gainEE = (int16_t)eeData[48];;    
}


static void ExtractTgcParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    mlx90640->tgc = (int8_t)LS_BYTE(eeData[60]) / 32.0f;
}


static void ExtractResolutionParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    uint8_t resolutionEE;
    resolutionEE = (eeData[56] & 0x3000) >> 12;    
    
    mlx90640->resolutionEE = resolutionEE;
}


static void ExtractKsTaParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{   
    mlx90640->KsTa = (int8_t)MS_BYTE(eeData[60]) / 8192.0f;
}


static void ExtractKsToParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    int32_t KsToScale;
    int8_t step;
    
    step = ((eeData[63] & 0x3000) >> 12) * 10;
    
    mlx90640->ct[0] = -40;
    mlx90640->ct[1] = 0;
    mlx90640->ct[2] = NIBBLE2(eeData[63]);
    mlx90640->ct[3] = NIBBLE3(eeData[63]);
    
    mlx90640->ct[2] = mlx90640->ct[2]*step;
    mlx90640->ct[3] = mlx90640->ct[2] + mlx90640->ct[3]*step;
    mlx90640->ct[4] = 400;
    
    KsToScale = NIBBLE1(eeData[63]) + 8;
    KsToScale = 1UL << KsToScale;
    
    mlx90640->ksTo[0] = (int8_t)LS_BYTE(eeData[61]) / (float)KsToScale;
    mlx90640->ksTo[1] = (int8_t)MS_BYTE(eeData[61]) / (float)KsToScale;
    mlx90640->ksTo[2] = (int8_t)LS_BYTE(eeData[62]) / (float)KsToScale;
    mlx90640->ksTo[3] = (int8_t)MS_BYTE(eeData[62]) / (float)KsToScale;
    mlx90640->ksTo[4] = -0.0002;
}





static void ExtractAlphaParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    int accRow[24];
    int accColumn[32];
    int p = 0;
    int alphaRef;
    uint8_t alphaScale;
    uint8_t accRowScale;
    uint8_t accColumnScale;
    uint8_t accRemScale;
    float alphaTemp[MLX90640_PIXEL_NUM];
    float temp;
    

    accRemScale = NIBBLE1(eeData[32]);
    accColumnScale = NIBBLE2(eeData[32]);
    accRowScale = NIBBLE3(eeData[32]);
    alphaScale = NIBBLE4(eeData[32]) + 30;
    alphaRef = eeData[33];
    
    for(size_t i = 0; i < 6; i++)
    {
        p = i * 4;
        accRow[p + 0] = NIBBLE1(eeData[34 + i]);
        accRow[p + 1] = NIBBLE2(eeData[34 + i]);
        accRow[p + 2] = NIBBLE3(eeData[34 + i]);
        accRow[p + 3] = NIBBLE4(eeData[34 + i]);
    }
    
    for(size_t i = 0; i < MLX90640_LINE_NUM; i++)
    {
        if (accRow[i] > 7)
        {
            accRow[i] = accRow[i] - 16;
        }
    }
    
    for(size_t i = 0; i < 8; i++)
    {
        p = i * 4;
        accColumn[p + 0] = NIBBLE1(eeData[40 + i]);
        accColumn[p + 1] = NIBBLE2(eeData[40 + i]);
        accColumn[p + 2] = NIBBLE3(eeData[40 + i]);
        accColumn[p + 3] = NIBBLE4(eeData[40 + i]);
    }
    
    for(size_t i = 0; i < MLX90640_COLUMN_NUM; i++)
    {
        if (accColumn[i] > 7)
        {
            accColumn[i] = accColumn[i] - 16;
        }
    }

    for(size_t i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(size_t j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            alphaTemp[p] = (eeData[64 + p] & 0x03F0) >> 4;
            if (alphaTemp[p] > 31)
            {
                alphaTemp[p] = alphaTemp[p] - 64;
            }
            alphaTemp[p] = alphaTemp[p]*(1 << accRemScale);
            alphaTemp[p] = (alphaRef + (accRow[i] << accRowScale) + (accColumn[j] << accColumnScale) + alphaTemp[p]);
            alphaTemp[p] = alphaTemp[p] / POW2(alphaScale);
            alphaTemp[p] = alphaTemp[p] - mlx90640->tgc * (mlx90640->cpAlpha[0] + mlx90640->cpAlpha[1])/2;
            alphaTemp[p] = SCALEALPHA/alphaTemp[p];
        }
    }
    
    temp = alphaTemp[0];
    for(size_t i = 1; i < MLX90640_PIXEL_NUM; i++)
    {
        if (alphaTemp[i] > temp)
        {
            temp = alphaTemp[i];
        }
    }
    
    alphaScale = 0;
    while(temp < 32767.4)
    {
        temp = temp*2;
        alphaScale = alphaScale + 1;
    } 
    
    for(size_t i = 0; i < MLX90640_PIXEL_NUM; i++)
    {
        temp = alphaTemp[i] * POW2(alphaScale);        
        mlx90640->alpha[i] = (temp + 0.5);        
        
    } 
    
    mlx90640->alphaScale = alphaScale;      

}


static void ExtractOffsetParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    int occRow[24];
    int occColumn[32];
    int p = 0;
    int16_t offsetRef;
    uint8_t occRowScale;
    uint8_t occColumnScale;
    uint8_t occRemScale;
    

    occRemScale = NIBBLE1(eeData[16]);
    occColumnScale = NIBBLE2(eeData[16]);
    occRowScale = NIBBLE3(eeData[16]);
    offsetRef = (int16_t)eeData[17];
        
    for(size_t i = 0; i < 6; i++)
    {
        p = i * 4;
        occRow[p + 0] = NIBBLE1(eeData[18 + i]);
        occRow[p + 1] = NIBBLE2(eeData[18 + i]);
        occRow[p + 2] = NIBBLE3(eeData[18 + i]);
        occRow[p + 3] = NIBBLE4(eeData[18 + i]);
    }
    
    for(size_t i = 0; i < MLX90640_LINE_NUM; i++)
    {
        if (occRow[i] > 7)
        {
            occRow[i] = occRow[i] - 16;
        }
    }
    
    for(size_t i = 0; i < 8; i++)
    {
        p = i * 4;
        occColumn[p + 0] = NIBBLE1(eeData[24 + i]);
        occColumn[p + 1] = NIBBLE2(eeData[24 + i]);
        occColumn[p + 2] = NIBBLE3(eeData[24 + i]);
        occColumn[p + 3] = NIBBLE4(eeData[24 + i]);
    }
    
    for(size_t i = 0; i < MLX90640_COLUMN_NUM; i ++)
    {
        if (occColumn[i] > 7)
        {
            occColumn[i] = occColumn[i] - 16;
        }
    }

    for(size_t i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(size_t j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            mlx90640->offset[p] = (eeData[64 + p] & MSBITS_6_MASK) >> 10;
            if (mlx90640->offset[p] > 31)
            {
                mlx90640->offset[p] = mlx90640->offset[p] - 64;
            }
            mlx90640->offset[p] = mlx90640->offset[p]*(1 << occRemScale);
            mlx90640->offset[p] = (offsetRef + (occRow[i] << occRowScale) + (occColumn[j] << occColumnScale) + mlx90640->offset[p]);
        }
    }
}


static void ExtractKtaPixelParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    int p = 0;
    int8_t KtaRC[4];
    uint8_t ktaScale1;
    uint8_t ktaScale2;
    uint8_t split;
    float ktaTemp[MLX90640_PIXEL_NUM];
    float temp;
    
    KtaRC[0] = (int8_t)MS_BYTE(eeData[54]);;
    KtaRC[2] = (int8_t)LS_BYTE(eeData[54]);;
    KtaRC[1] = (int8_t)MS_BYTE(eeData[55]);;
    KtaRC[3] = (int8_t)LS_BYTE(eeData[55]);;

    ktaScale1 = NIBBLE2(eeData[56]) + 8;
    ktaScale2 = NIBBLE1(eeData[56]);

    for(size_t i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(size_t j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            split = 2*(p/32 - (p/64)*2) + p%2;
            ktaTemp[p] = (eeData[64 + p] & 0x000E) >> 1;
            if (ktaTemp[p] > 3)
            {
                ktaTemp[p] = ktaTemp[p] - 8;
            }
            ktaTemp[p] = ktaTemp[p] * (1 << ktaScale2);
            ktaTemp[p] = KtaRC[split] + ktaTemp[p];
            ktaTemp[p] = ktaTemp[p] / POW2(ktaScale1);
            
        }
    }
    
    temp = std::abs(ktaTemp[0]);
    for(size_t i = 1; i < MLX90640_PIXEL_NUM; i++)
    {
        if (std::abs(ktaTemp[i]) > temp)
        {
            temp = std::abs(ktaTemp[i]);
        }
    }
    
    ktaScale1 = 0;
    while(temp < 63.4)
    {
        temp = temp*2;
        ktaScale1 = ktaScale1 + 1;
    }    

    for(size_t i = 0; i < MLX90640_PIXEL_NUM; i++)
    {
        temp = ktaTemp[i] * POW2(ktaScale1);
        if (temp < 0)
        {
            mlx90640->kta[i] = (temp - 0.5);
        }
        else
        {
            mlx90640->kta[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90640->ktaScale = ktaScale1;           
}



static void ExtractKvPixelParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    int p = 0;
    int8_t KvT[4];
    int8_t KvRoCo;
    int8_t KvRoCe;
    int8_t KvReCo;
    int8_t KvReCe;
    uint8_t kvScale;
    uint8_t split;
    float kvTemp[MLX90640_PIXEL_NUM];
    float temp;

    KvRoCo = NIBBLE4(eeData[52]);
    if (KvRoCo > 7)
    {
        KvRoCo = KvRoCo - 16;
    }
    KvT[0] = KvRoCo;
    
    KvReCo = NIBBLE3(eeData[52]);
    if (KvReCo > 7)
    {
        KvReCo = KvReCo - 16;
    }
    KvT[2] = KvReCo;

    KvRoCe = NIBBLE2(eeData[52]);
    if (KvRoCe > 7)
    {
        KvRoCe = KvRoCe - 16;
    }
    KvT[1] = KvRoCe;

    KvReCe = NIBBLE1(eeData[52]);
    if (KvReCe > 7)
    {
        KvReCe = KvReCe - 16;
    }
    KvT[3] = KvReCe;

    kvScale = NIBBLE3(eeData[56]);


    for(size_t i = 0; i < MLX90640_LINE_NUM; i++)
    {
        for(size_t j = 0; j < MLX90640_COLUMN_NUM; j ++)
        {
            p = 32 * i +j;
            split = 2*(p/32 - (p/64)*2) + p%2;
            kvTemp[p] = KvT[split];
            kvTemp[p] = kvTemp[p] / POW2(kvScale);
        }
    }
    
    temp = std::abs(kvTemp[0]);
    for(size_t i = 1; i < MLX90640_PIXEL_NUM; i++)
    {
        if (std::abs(kvTemp[i]) > temp)
        {
            temp = std::abs(kvTemp[i]);
        }
    }
    
    kvScale = 0;
    while(temp < 63.4)
    {
        temp = temp*2;
        kvScale = kvScale + 1;
    }    

    for(size_t i = 0; i < MLX90640_PIXEL_NUM; i++)
    {
        temp = kvTemp[i] * POW2(kvScale);
        if (temp < 0)
        {
            mlx90640->kv[i] = (temp - 0.5);
        }
        else
        {
            mlx90640->kv[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90640->kvScale = kvScale;        
}


static void ExtractCPParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    float alphaSP[2];
    int16_t offsetSP[2];
    float cpKv;
    float cpKta;
    uint8_t alphaScale;
    uint8_t ktaScale1;
    uint8_t kvScale;

    alphaScale = NIBBLE4(eeData[32]) + 27;
    
    offsetSP[0] = (eeData[58] & LSBITS_10_MASK);
    if (offsetSP[0] > 511)
    {
        offsetSP[0] = offsetSP[0] - 1024;
    }
    
    offsetSP[1] = (eeData[58] & MSBITS_6_MASK) >> 10;
    if (offsetSP[1] > 31)
    {
        offsetSP[1] = offsetSP[1] - 64;
    }
    offsetSP[1] = offsetSP[1] + offsetSP[0]; 
    
    alphaSP[0] = (eeData[57] & LSBITS_10_MASK);
    if (alphaSP[0] > 511)
    {
        alphaSP[0] = alphaSP[0] - 1024;
    }
    alphaSP[0] = alphaSP[0] /  POW2(alphaScale);
    
    alphaSP[1] = (eeData[57] & MSBITS_6_MASK) >> 10;
    if (alphaSP[1] > 31)
    {
        alphaSP[1] = alphaSP[1] - 64;
    }
    alphaSP[1] = (1 + alphaSP[1]/128) * alphaSP[0];
    
    cpKta = (int8_t)LS_BYTE(eeData[59]);
    
    ktaScale1 = NIBBLE2(eeData[56]) + 8;    
    mlx90640->cpKta = cpKta / POW2(ktaScale1);
    
    cpKv = (int8_t)MS_BYTE(eeData[59]);
    
    kvScale = NIBBLE3(eeData[56]);
    mlx90640->cpKv = cpKv / POW2(kvScale);

    mlx90640->cpAlpha[0] = alphaSP[0];
    mlx90640->cpAlpha[1] = alphaSP[1];
    mlx90640->cpOffset[0] = offsetSP[0];
    mlx90640->cpOffset[1] = offsetSP[1];  
}


static void ExtractCILCParameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    float ilChessC[3];
    uint8_t calibrationModeEE;
    
    calibrationModeEE = (eeData[10] & 0x0800) >> 4;
    calibrationModeEE = calibrationModeEE ^ 0x80;

    ilChessC[0] = (eeData[53] & 0x003F);
    if (ilChessC[0] > 31)
    {
        ilChessC[0] = ilChessC[0] - 64;
    }
    ilChessC[0] = ilChessC[0] / 16.0f;
    
    ilChessC[1] = (eeData[53] & 0x07C0) >> 6;
    if (ilChessC[1] > 15)
    {
        ilChessC[1] = ilChessC[1] - 32;
    }
    ilChessC[1] = ilChessC[1] / 2.0f;
    
    ilChessC[2] = (eeData[53] & 0xF800) >> 11;
    if (ilChessC[2] > 15)
    {
        ilChessC[2] = ilChessC[2] - 32;
    }
    ilChessC[2] = ilChessC[2] / 8.0f;
    
    mlx90640->calibrationModeEE = calibrationModeEE;
    mlx90640->ilChessC[0] = ilChessC[0];
    mlx90640->ilChessC[1] = ilChessC[1];
    mlx90640->ilChessC[2] = ilChessC[2];
}



static IResult<> ValidateFrameData(const uint16_t *frameData)
{
    uint8_t line = 0;
    
    for(size_t i=0; i<MLX90640_PIXEL_NUM; i+=LINE_SIZE)
    {
        if((frameData[i] == 0x7FFF) && (line%2 == frameData[833])) 
            return Err(Error::FrameDataErr);
        line = line + 1;
    }    
        
    return Ok();   
}



static IResult<> ExtractDeviatingPixels(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    uint16_t pixCnt = 0;
    uint16_t brokenPixCnt = 0;
    uint16_t outlierPixCnt = 0;
    size_t i;
    
    for(pixCnt = 0; pixCnt<5; pixCnt++)
    {
        mlx90640->brokenPixels[pixCnt] = 0xFFFF;
        mlx90640->outlierPixels[pixCnt] = 0xFFFF;
    }
        
    pixCnt = 0;    
    while (pixCnt < MLX90640_PIXEL_NUM && brokenPixCnt < 5 && outlierPixCnt < 5)
    {
        if(eeData[pixCnt+64] == 0)
        {
            mlx90640->brokenPixels[brokenPixCnt] = pixCnt;
            brokenPixCnt = brokenPixCnt + 1;
        }    
        else if((eeData[pixCnt+64] & 0x0001) != 0)
        {
            mlx90640->outlierPixels[outlierPixCnt] = pixCnt;
            outlierPixCnt = outlierPixCnt + 1;
        }    
        
        pixCnt = pixCnt + 1;
        
    } 
    
    if(brokenPixCnt > 4)  
        return Err<Error>(Error::BrokenPixelsNum);

    if(outlierPixCnt > 4)  
        return Err<Error>(Error::OutlierPixelsNum);

    if((brokenPixCnt + outlierPixCnt) > 4)  
        return Err<Error>(Error::BadPixelsNum);

    for(pixCnt=0; pixCnt<brokenPixCnt; pixCnt++)
    {
        for(i=pixCnt+1; i<brokenPixCnt; i++)
        {
            if(const auto res = CheckAdjacentPixels(mlx90640->brokenPixels[pixCnt],mlx90640->brokenPixels[i]);
                res.is_err()) return Err(res.unwrap_err());

        }    
    }
    
    for(pixCnt=0; pixCnt<outlierPixCnt; pixCnt++)
    {
        for(i=pixCnt+1; i<outlierPixCnt; i++)
        {
            if(const auto res = CheckAdjacentPixels(mlx90640->outlierPixels[pixCnt],mlx90640->outlierPixels[i]);
                res.is_err()) return Err(res.unwrap_err());

        }    
    } 
    
    for(pixCnt=0; pixCnt<brokenPixCnt; pixCnt++)
    {
        for(i=0; i<outlierPixCnt; i++)
        {
            if(const auto res = CheckAdjacentPixels(mlx90640->brokenPixels[pixCnt],mlx90640->outlierPixels[i]);
                res.is_err()) return Err(res.unwrap_err());

        }    
    }    


    return Ok();
}



IResult<> MLX90640::extract_parameters(const uint16_t *eeData, MLX90640_Coeffs *mlx90640)
{
    
    ExtractVDDParameters(eeData, mlx90640);
    ExtractPTATParameters(eeData, mlx90640);
    ExtractGainParameters(eeData, mlx90640);
    ExtractTgcParameters(eeData, mlx90640);
    ExtractResolutionParameters(eeData, mlx90640);
    ExtractKsTaParameters(eeData, mlx90640);
    ExtractKsToParameters(eeData, mlx90640);
    ExtractCPParameters(eeData, mlx90640);
    ExtractAlphaParameters(eeData, mlx90640);
    ExtractOffsetParameters(eeData, mlx90640);
    ExtractKtaPixelParameters(eeData, mlx90640);
    ExtractKvPixelParameters(eeData, mlx90640);
    ExtractCILCParameters(eeData, mlx90640);
    return ExtractDeviatingPixels(eeData, mlx90640);  
}


Result<uint16_t, Error> MLX90640::get_frame_data(uint16_t *frameData)
{
    uint16_t statusRegister;
    uint16_t data[64];
    
    if(const auto res = read_burst(STATUS_REG, std::span(&statusRegister, 1));
        res.is_err()) return Err(res.unwrap_err());
    const auto dataReady = GET_DATA_READY(statusRegister); 
    if(not dataReady)
        return Err(Error::DataNotReady);
    
    if(const auto res = write_reg(STATUS_REG, INIT_STATUS_VALUE);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = read_burst(PIXEL_DATA_START_ADDRESS, std::span(frameData, MLX90640_PIXEL_NUM)); 
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = read_burst(AUX_DATA_START_ADDRESS, std::span(data,MLX90640_AUX_NUM));   
        res.is_err()) return Err(res.unwrap_err());
        
    if(const auto res = read_burst(CTRL_REG, std::span(&frameData[832], 1));
        res.is_err()) return Err(res.unwrap_err());
    frameData[833] = GET_FRAME(statusRegister);
    
    if(const auto res = ValidateAuxData(data);
        res.is_err()) return Err(res.unwrap_err());

    for(size_t i=0; i<MLX90640_AUX_NUM; i++){
        frameData[i+MLX90640_PIXEL_NUM] = data[i];
    } 
    
    if(const auto res = ValidateFrameData(frameData);
        res.is_err()) return Err(res.unwrap_err()); 
    
    return Ok(frameData[833]);    
}


IResult<> MLX90640::set_resolution(uint8_t resolution)
{
    uint16_t controlRegister1;
    uint16_t value;
    // int res;
    
    //value = (resolution & 0x03) << 10;
    value = ((uint16_t)resolution << CTRL_RESOLUTION_SHIFT);
    value &= ~CTRL_RESOLUTION_MASK;
    
    if(const auto res = read_burst(CTRL_REG, std::span(&controlRegister1, 1));
        res.is_err()) return Err(res.unwrap_err());

    value = (controlRegister1 & CTRL_RESOLUTION_MASK) | value;
    if(const auto res = write_reg(CTRL_REG, value);         
        res.is_err()) return Err(res.unwrap_err());
    return Ok();

}


Result<void, Error> MLX90640::init(uint16_t EE[832], MLX90640_Coeffs & MLXPars){
    if(const auto res = set_refresh_rate(MLX90640::DataRate::_64Hz);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_burst(0x2400, std::span(EE, 832));                     //读取像素校正参数
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = extract_parameters(EE, &MLXPars);    //解析校正参数（计算温度时需要）
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}
Result<uint16_t, Error> MLX90640::get_cur_resolution(){
    uint16_t controlRegister1;
    int resolutionRAM;

    if(const auto res = read_burst(CTRL_REG, std::span(&controlRegister1, 1));
        res.is_err()) return Err(res.unwrap_err());
    resolutionRAM = (controlRegister1 & ~CTRL_RESOLUTION_MASK) >> CTRL_RESOLUTION_SHIFT;
    
    return Ok(resolutionRAM); 
}


IResult<> MLX90640::set_refresh_rate(DataRate datarate)
{
    uint16_t controlRegister1;
    uint16_t value;
    
    value = ((uint16_t)datarate << CTRL_REFRESH_SHIFT);
    value &= ~CTRL_REFRESH_MASK;
    
    if(const auto res = read_burst(CTRL_REG, std::span(&controlRegister1, 1));
        res.is_err()) return Err(res.unwrap_err());

    value = (controlRegister1 & CTRL_REFRESH_MASK) | value;

    if(const auto res = write_reg(CTRL_REG, value);
        res.is_err()) return Err(res.unwrap_err());
    
    return Ok();
}


Result<uint16_t, Error> MLX90640::get_refresh_rate(){
    uint16_t controlRegister1;
    int refreshRate;
    
    if(const auto res = read_burst(CTRL_REG, std::span(&controlRegister1, 1));
        res.is_err()) return Err(res.unwrap_err());
    refreshRate = (controlRegister1 & ~CTRL_REFRESH_MASK) >> CTRL_REFRESH_SHIFT;
    
    return Ok(refreshRate);
}


IResult<> MLX90640::set_interleaved_mode(){
    uint16_t controlRegister1;
    uint16_t value;

    
    if(const auto res = read_burst(CTRL_REG, std::span(&controlRegister1, 1));
        res.is_err()) return Err(res.unwrap_err());  

    value = (controlRegister1 & ~CTRL_MEAS_MODE_MASK);

    if(const auto res = write_reg(CTRL_REG, value);        
        res.is_err()) return Err(res.unwrap_err());
    
    return Ok();
}


IResult<> MLX90640::set_chess_mode(){
    uint16_t controlRegister1;
    uint16_t value;
        
    if(const auto res = read_burst(CTRL_REG, std::span(&controlRegister1, 1));
        res.is_err()) return Err(res.unwrap_err());

    value = (controlRegister1 | CTRL_MEAS_MODE_MASK);

    if(const auto res = write_reg(CTRL_REG, value);        
        res.is_err()) return Err(res.unwrap_err());
    
    return Ok();
}


Result<uint16_t, Error> MLX90640::get_cur_mode(){
    uint16_t controlRegister1;
    int modeRAM;

    
    if(const auto res = read_burst(CTRL_REG, std::span(&controlRegister1, 1));
        res.is_err()) return Err(res.unwrap_err());

    modeRAM = (controlRegister1 & CTRL_MEAS_MODE_MASK) >> CTRL_MEAS_MODE_SHIFT;
    
    return Ok(modeRAM); 
}


void MLX90640::calculate_to(
    const uint16_t *frameData, 
    const MLX90640_Coeffs *params, 
    float emissivity, float tr, 
    float *result
){
    float vdd;
    float ta;
    float ta4;
    float tr4;
    float taTr;
    float gain;
    float irDataCP[2];
    float irData;
    float alphaCompensated;
    uint8_t mode;
    int8_t ilPattern;
    int8_t chessPattern;
    int8_t pattern;
    int8_t conversionPattern;
    float Sx;
    float To;
    float alphaCorrR[4];
    int8_t range;
    uint16_t subPage;
    float ktaScale;
    float kvScale;
    float alphaScale;
    float kta;
    float kv;
    
    subPage = frameData[833];
    vdd = get_vdd(frameData, params);
    ta = get_ta(frameData, params);
    
    ta4 = (ta + ABSOLUTE_ZERO);
    ta4 = ta4 * ta4;
    ta4 = ta4 * ta4;
    tr4 = (tr + ABSOLUTE_ZERO);
    tr4 = tr4 * tr4;
    tr4 = tr4 * tr4;
    taTr = tr4 - (tr4-ta4)/emissivity;
    
    ktaScale = POW2(params->ktaScale);
    kvScale = POW2(params->kvScale);
    alphaScale = POW2(params->alphaScale);
    
    alphaCorrR[0] = 1 / (1 + params->ksTo[0] * 40);
    alphaCorrR[1] = 1 ;
    alphaCorrR[2] = (1 + params->ksTo[1] * params->ct[2]);
    alphaCorrR[3] = alphaCorrR[2] * (1 + params->ksTo[2] * (params->ct[3] - params->ct[2]));
    
//------------------------- Gain calculation -----------------------------------    
    
    gain = (float)params->gainEE / (int16_t)frameData[778]; 

//------------------------- To calculation -------------------------------------    
    mode = (frameData[832] & CTRL_MEAS_MODE_MASK) >> 5;
    
    irDataCP[0] = (int16_t)frameData[776] * gain;
    irDataCP[1] = (int16_t)frameData[808] * gain;
    
    irDataCP[0] = irDataCP[0] - params->cpOffset[0] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - VCC_VOLTAGE));
    if( mode ==  params->calibrationModeEE){
        irDataCP[1] = irDataCP[1] - params->cpOffset[1] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - VCC_VOLTAGE));
    }else{
      irDataCP[1] = irDataCP[1] - (params->cpOffset[1] + params->ilChessC[0]) * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - VCC_VOLTAGE));
    }

    for(size_t i = 0; i < MLX90640_PIXEL_NUM; i++)
    {
        ilPattern = i / 32 - (i / 64) * 2; 
        chessPattern = ilPattern ^ (i - (i/2)*2); 
        conversionPattern = ((i + 2) / 4 - (i + 3) / 4 + (i + 1) / 4 - i / 4) * (1 - 2 * ilPattern);
        
        if(mode == 0)
        {
            pattern = ilPattern; 
        }
        else 
        {
            pattern = chessPattern; 
        }               
        
        if(pattern == frameData[833]){  
            irData = (int16_t)frameData[i] * gain;
            
            kta = params->kta[i]/ktaScale;
            kv = params->kv[i]/kvScale;
            irData = irData - params->offset[i]*(1 + kta*(ta - 25))*(1 + kv*(vdd - VCC_VOLTAGE));
            
            if(mode !=  params->calibrationModeEE)
            {
              irData = irData + params->ilChessC[2] * (2 * ilPattern - 1) - params->ilChessC[1] * conversionPattern; 
            }                       
    
            irData = irData - params->tgc * irDataCP[subPage];
            irData = irData / emissivity;
            
            alphaCompensated = SCALEALPHA*alphaScale/params->alpha[i];
            alphaCompensated = alphaCompensated*(1 + params->KsTa * (ta - 25));
                        
            Sx = alphaCompensated * alphaCompensated * alphaCompensated * (irData + alphaCompensated * taTr);
            Sx = sqrt(sqrt(Sx)) * params->ksTo[1];            
            
            To = sqrt(sqrt(irData/(alphaCompensated * (1 - params->ksTo[1] * ABSOLUTE_ZERO) + Sx) + taTr)) - ABSOLUTE_ZERO;                     
            
            if(To < params->ct[1])
            {
                range = 0;
            }
            else if(To < params->ct[2])   
            {
                range = 1;            
            }   
            else if(To < params->ct[3])
            {
                range = 2;            
            }
            else
            {
                range = 3;            
            }      
            
            To = sqrt(sqrt(irData / (alphaCompensated * alphaCorrR[range] * (1 + params->ksTo[range] * (To - params->ct[range]))) + taTr)) - ABSOLUTE_ZERO;
                        
            result[i] = To;
        }
    }
}


void MLX90640::get_image(const uint16_t *frameData, const MLX90640_Coeffs *params, float *result){
    float vdd;
    float ta;
    float gain;
    float irDataCP[2];
    float irData;
    float alphaCompensated;

    int8_t ilPattern;
    int8_t chessPattern;
    int8_t pattern;
    int8_t conversionPattern;
    float image;
    uint16_t subPage;
    float ktaScale;
    float kvScale;
    float kta;
    float kv;
    
    subPage = frameData[833];
    vdd = get_vdd(frameData, params);
    ta = get_ta(frameData, params);
    
    ktaScale = POW2(params->ktaScale);
    kvScale = POW2(params->kvScale);
    
    //------------------------- Gain calculation -----------------------------------    
    
    gain = (float)params->gainEE / (int16_t)frameData[778]; 

    //------------------------- Image calculation -------------------------------------    
    
    const uint8_t mode = (frameData[832] & CTRL_MEAS_MODE_MASK) >> 5;
    
    irDataCP[0] = (int16_t)frameData[776] * gain;
    irDataCP[1] = (int16_t)frameData[808] * gain;
    
    irDataCP[0] = irDataCP[0] - params->cpOffset[0] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - VCC_VOLTAGE));
    if( mode ==  params->calibrationModeEE){
        irDataCP[1] = irDataCP[1] - params->cpOffset[1] * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - VCC_VOLTAGE));
    }else{
      irDataCP[1] = irDataCP[1] - (params->cpOffset[1] + params->ilChessC[0]) * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - VCC_VOLTAGE));
    }

    for( size_t i = 0; i < MLX90640_PIXEL_NUM; i++){
        ilPattern = i / 32 - (i / 64) * 2; 
        chessPattern = ilPattern ^ (i - (i/2)*2); 
        conversionPattern = ((i + 2) / 4 - (i + 3) / 4 + (i + 1) / 4 - i / 4) * (1 - 2 * ilPattern);
        
        if(mode == 0){
            pattern = ilPattern; 
        }else {
            pattern = chessPattern; 
        }
        
        if(pattern == frameData[833]){
            irData = (int16_t)frameData[i] * gain;
            
            kta = params->kta[i]/ktaScale;
            kv = params->kv[i]/kvScale;
            irData = irData - params->offset[i]*(1 + kta*(ta - 25))*(1 + kv*(vdd - VCC_VOLTAGE));

            if(mode !=  params->calibrationModeEE){
              irData = irData + params->ilChessC[2] * (2 * ilPattern - 1) - params->ilChessC[1] * conversionPattern; 
            }
            
            irData = irData - params->tgc * irDataCP[subPage];
                        
            alphaCompensated = params->alpha[i];
            
            image = irData*alphaCompensated;
            
            result[i] = image;
        }
    }
}


float MLX90640::get_ta(const uint16_t *frameData, const MLX90640_Coeffs *params)
{
    int16_t ptat;
    float ptatArt;
    float vdd;
    float ta;
    
    vdd = get_vdd(frameData, params);
    
    ptat = (int16_t)frameData[800];
    
    ptatArt = (ptat / (ptat * params->alphaPTAT + (int16_t)frameData[MLX90640_PIXEL_NUM])) * POW2(18u);
    
    ta = (ptatArt / (1 + params->KvPTAT * (vdd - VCC_VOLTAGE)) - params->vPTAT25);
    ta = ta / params->KtPTAT + 25;
    
    return ta;
}


#if 0
void MLX90640::bad_pixels_correction(uint16_t *pixels, float *to, int mode, MLX90640_Coeffs *params)
{   
    float ap[4];
    uint8_t pix;
    uint8_t line;
    uint8_t column;
    
    pix = 0;
    while(pixels[pix] != 0xFFFF)
    {
        line = pixels[pix]>>5;
        column = pixels[pix] - (line<<5);
        
        if(mode == 1){        
            if(line == 0)
            {
                if(column == 0)
                {        
                    to[pixels[pix]] = to[33];                    
                }
                else if(column == 31)
                {
                    to[pixels[pix]] = to[62];                      
                }
                else
                {
                    to[pixels[pix]] = (to[pixels[pix]+31] + to[pixels[pix]+33])/2.0;                    
                }        
            }
            else if(line == 23)
            {
                if(column == 0)
                {
                    to[pixels[pix]] = to[705];                    
                }
                else if(column == 31)
                {
                    to[pixels[pix]] = to[734];                       
                }
                else
                {
                    to[pixels[pix]] = (to[pixels[pix]-33] + to[pixels[pix]-31])/2.0;                       
                }                       
            } 
            else if(column == 0)
            {
                to[pixels[pix]] = (to[pixels[pix]-31] + to[pixels[pix]+33])/2.0;                
            }
            else if(column == 31)
            {
                to[pixels[pix]] = (to[pixels[pix]-33] + to[pixels[pix]+31])/2.0;                
            } 
            else
            {
                ap[0] = to[pixels[pix]-33];
                ap[1] = to[pixels[pix]-31];
                ap[2] = to[pixels[pix]+31];
                ap[3] = to[pixels[pix]+33];
                to[pixels[pix]] = GetMedian(ap,4);
            }                   
        }else{        
            if(column == 0)
            {
                to[pixels[pix]] = to[pixels[pix]+1];            
            }
            else if(column == 1 || column == 30)
            {
                to[pixels[pix]] = (to[pixels[pix]-1]+to[pixels[pix]+1])/2.0;                
            } 
            else if(column == 31)
            {
                to[pixels[pix]] = to[pixels[pix]-1];
            } 
            else
            {
                if(IsPixelBad(pixels[pix]-2,params) == 0 && IsPixelBad(pixels[pix]+2,params) == 0)
                {
                    ap[0] = to[pixels[pix]+1] - to[pixels[pix]+2];
                    ap[1] = to[pixels[pix]-1] - to[pixels[pix]-2];
                    if(std::abs(ap[0]) > std::abs(ap[1]))
                    {
                        to[pixels[pix]] = to[pixels[pix]-1] + ap[1];                        
                    }
                    else
                    {
                        to[pixels[pix]] = to[pixels[pix]+1] + ap[0];                        
                    }
                }
                else
                {
                    to[pixels[pix]] = (to[pixels[pix]-1]+to[pixels[pix]+1])/2.0;                    
                }            
            }                      
        } 
        pix = pix + 1;    
    }    
}
#endif

}
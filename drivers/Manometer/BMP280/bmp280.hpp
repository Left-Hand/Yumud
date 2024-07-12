#ifndef __BMP280_HPP__

#define __BMP280_HPP__

#include "device_defs.h"
#include "../types/real.hpp"

#define BMP280_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
// #define BMP280_DEBUG(...)

class BMP280{
public:
    enum class Mode:uint8_t{
        Sleep, Single, Cont = 0x03
    };

    enum class TempratureSampleMode:uint8_t{
        SKIP, Bit16, Bit17, Bit18, Bit19, Bit20 = 0x07
    };

    enum class PressureSampleMode:uint8_t{
        SKIP, Bit16, Bit17, Bit18, Bit19, Bit20 = 0x07
    };

    enum class DataRate:uint8_t{
        HZ200, HZ16, HZ8, HZ4, HZ2, HZ1, HZ0_5, HZ0_25
    };

    enum class FilterCoefficient{
        OFF, FC2, FC4, FC8, FC16
    };

protected:
    I2cDrv & bus_drv;

    const uint8_t valid_chipid = 0x58;
    const uint8_t reset_key = 0xB6;

    struct ChipIDReg{
        REG8_BEGIN
        REG8_END
    };

    struct ResetReg{
        REG8_BEGIN
        REG8_END
    };

    struct StatusReg{
        REG8_BEGIN
        uint8_t im:1;
        uint8_t __resv1__:2;
        uint8_t busy:1;
        uint8_t __resv2__:4;
        REG8_END
    };

    struct CtrlReg{
        REG8_BEGIN
        uint8_t mode:2;
        uint8_t osrs_p:3;
        uint8_t osrs_t:3;
        REG8_END
    };

    struct ConfigReg{
        REG8_BEGIN
        uint8_t spi3_en:1;
        uint8_t __resv__:1;
        uint8_t filter:3;
        uint8_t t_sb:3;
        REG8_END
    };

    struct PressureReg{REG16_BEGIN REG16_END};
    struct PressureXReg{REG8_BEGIN REG8_END};
    struct TemperatureReg{REG16_BEGIN REG16_END};
    struct TemperatureXReg{REG8_BEGIN REG8_END};

    enum class RegAddress:uint8_t{
        DigT1 = 0x88,
        DigT2 = 0x8A,
        DigT3 = 0x8C,
        DigP1 = 0x8E,
        DigP2 = 0x90,
        DigP3 = 0x92,
        DigP4 = 0x94,
        DigP5 = 0x96,
        DigP6 = 0x98,
        DigP7 = 0x9A,
        DigP8 = 0x9C,
        DigP9 = 0x9E,
        ChipID=0xD0,
        Reset=0xE0,
        Status=0xF3,
        Ctrl=0xF4,
        Config=0xF5,
        Pressure=0xF7,
        PressureX=0xF9,
        Temperature=0xFA,
        TemperatureX=0xFC
    };

    struct{
        uint16_t digT1;
        int16_t digT2;
        int16_t digT3;
        uint16_t digP1;
        int16_t digP2;
        int16_t digP3;
        int16_t digP4;
        int16_t digP5;
        int16_t digP6;
        int16_t digP7;
        int16_t digP8;
        int16_t digP9;
        ChipIDReg chipIDReg;
        ResetReg resetReg;
        StatusReg statusReg;
        CtrlReg ctrlReg;
        ConfigReg configReg;
        PressureReg pressureReg;
        PressureXReg pressureXReg;
        TemperatureReg temperatureReg;
        TemperatureXReg temperatureXReg;
    };

    void writeReg(const RegAddress & regAddress, const uint16_t & regData){
        bus_drv.writeReg((uint8_t)regAddress, regData);
    }

    void readReg(const RegAddress & regAddress, uint16_t & regData){
        bus_drv.readReg((uint8_t)regAddress, regData);
    }

    void writeReg(const RegAddress & regAddress, const uint8_t & regData){
        bus_drv.writeReg((uint8_t)regAddress, regData);
    }

    void readReg(const RegAddress & regAddress, uint8_t & regData){
        bus_drv.readReg((uint8_t)regAddress, regData);
        // BMP280_DEBUG((uint8_t)regAddress, (uint8_t)regData);
    }

    void requestPool(const RegAddress & regAddress, uint8_t * datas, uint8_t size, uint8_t len){
        bus_drv.readPool((uint8_t)regAddress, datas, size, len, false);
    }

    uint32_t getPressureData(){
        uint32_t pressureData = 0;
        readReg(RegAddress::Pressure, pressureReg.data);
        pressureData = pressureReg.data << 4;
        readReg(RegAddress::PressureX, pressureXReg.data);
        pressureData |= pressureXReg.data >> 4;
        // BMP280_DEBUG("PressureData:", pressureData);
        return pressureData;
    }

    uint32_t getTemperatureData(){
        uint32_t temperatureData = 0;
        readReg(RegAddress::Temperature, temperatureReg.data);
        temperatureData = temperatureReg.data << 4;
        readReg(RegAddress::TemperatureX, temperatureXReg.data);
        temperatureData |= temperatureXReg.data >> 4;
        // BMP280_DEBUG("TempratureData:", temperatureData);
        return temperatureData;
    }

public:
    BMP280(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    ~BMP280(){;}

    bool isChipValid(){
        readReg(RegAddress::ChipID, chipIDReg.data);
        BMP280_DEBUG("CHIP code: ", chipIDReg.data);
        return (chipIDReg.data == valid_chipid);
    }

    void setTempratureSampleMode(const TempratureSampleMode tempMode){
        ctrlReg.osrs_t = (uint8_t)tempMode;
        writeReg(RegAddress::Ctrl, ctrlReg.data);
    }

    void setPressureSampleMode(const PressureSampleMode pressureMode){
        ctrlReg.osrs_p = (uint8_t)pressureMode;
        writeReg(RegAddress::Ctrl, ctrlReg.data);
    }

    void setMode(const Mode mode){
        ctrlReg.mode = (uint8_t)mode;
        writeReg(RegAddress::Ctrl, ctrlReg.data);
    }

    void setDataRate(const DataRate dataRate){
        configReg.t_sb = (uint8_t)dataRate;
        writeReg(RegAddress::Config, configReg.data);
    }

    void setFilterCoefficient(const FilterCoefficient filterCoeff){
        configReg.filter = (uint8_t)filterCoeff;
        writeReg(RegAddress::Config, configReg.data);
    }

    void reset(){
        writeReg(RegAddress::Reset, reset_key);
    }

    bool isIdle(){
        readReg(RegAddress::Status, statusReg.data);
        return (statusReg.busy == 0);
    }

    void enableSpi3(const bool en = true){
        configReg.spi3_en = en;
        writeReg(RegAddress::Config, configReg.data);
    }

    void getPressure(int32_t & pressure){

        uint32_t adc_T = getTemperatureData();
        uint32_t adc_P = getPressureData();

        if(adc_P == 0)
        {
            pressure = 0;
            return;
        }

        uint64_t begin_t = nanos();
        //Temperature
        uint32_t var1 = (((float)adc_T)/16384.0f-((float)digT1)/1024.0f)*((float)digT2);
        uint32_t var2 = ((((float)adc_T)/131072.0f-((float)digT1)/8192.0f)*(((float)adc_T)
                    /131072.0f-((float)digT1)/8192.0f))*((float)digT3);

        uint32_t t_fine = (unsigned long)(var1+var2);

        var1 = ((float)t_fine/2.0f)-64000.0f;
        var2 = var1*var1*((float)digP6)/32768.0f;
        var2 = var2 +var1*((float)digP5)*2.0f;
        var2 = (var2/4.0f)+(((float)digP4)*65536.0f);
        var1 = (((float)digP3)*var1*var1/524288.0f+((float)digP2)*var1)/524288.0f;
        var1 = (1.0f+var1/32768.0f)*((float)digP1);
        uint32_t p = 1048576.0f-(float)adc_P;
        p = (p-(var2/4096.0f))*6250.0f/var1;
        var1 = ((float)digP9)*p*p/2147483648.0f;
        var2 = p*((float)digP8)/32768.0f;
        pressure = p+(var1+var2+((float)digP7))/16.0f;

        uint64_t end_t = nanos();
        BMP280_DEBUG("cal used", (uint32_t)(end_t - begin_t));
    }

    void init(){
        reset();
        bool chip_valid = isChipValid();
        // BMP280_DEBUG("BMP280 validation:", chip_valid);
        if(!chip_valid) return;

        // setMode(Mode::Cont);
        // setTempratureSampleMode(TempratureSampleMode::Bit20);
        // setPressureSampleMode(PressureSampleMode::Bit20);
        writeReg(RegAddress::Ctrl, (uint8_t)0xFFU);

        setDataRate(DataRate::HZ200);
        setFilterCoefficient(FilterCoefficient::OFF);
        enableSpi3(false);

        // writeReg(RegAddress::Config, (uint8_t)0x00);

        // memset(&digT1, 0, 2 * 12);
        requestPool(RegAddress::DigT1, (uint8_t *)&digT1, 2, 2*12);

        // for(uint16_t * ptr = &digT1; ptr <= (uint16_t *)&digP9; ptr++)
        //     BMP280_DEBUG(String(*ptr, 16));
    }
};
#endif
#include "bmp280.hpp"

using namespace ymd::drivers;

bool BMP280::isChipValid(){
    readReg(RegAddress::ChipID, chipIDReg);
    BMP280_DEBUG("CHIP code: ", uint8_t(chipIDReg));
    return (chipIDReg == valid_chipid);
}

void BMP280::setTempratureSampleMode(const TempratureSampleMode tempMode){
    ctrlReg.osrs_t = (uint8_t)tempMode;
    writeReg(RegAddress::Ctrl, ctrlReg);
}

void BMP280::setPressureSampleMode(const PressureSampleMode pressureMode){
    ctrlReg.osrs_p = (uint8_t)pressureMode;
    writeReg(RegAddress::Ctrl, ctrlReg);
}

void BMP280::setMode(const Mode mode){
    ctrlReg.mode = (uint8_t)mode;
    writeReg(RegAddress::Ctrl, ctrlReg);
}

void BMP280::setDataRate(const DataRate dataRate){
    configReg.t_sb = (uint8_t)dataRate;
    writeReg(RegAddress::Config, configReg);
}

void BMP280::setFilterCoefficient(const FilterCoefficient filterCoeff){
    configReg.filter = (uint8_t)filterCoeff;
    writeReg(RegAddress::Config, configReg);
}

void BMP280::reset(){
    writeReg(RegAddress::Reset, reset_key);
}

bool BMP280::isIdle(){
    readReg(RegAddress::Status, statusReg);
    return (statusReg.busy == 0);
}

void BMP280::enableSpi3(const bool en){
    configReg.spi3_en = en;
    writeReg(RegAddress::Config, configReg);
}

void BMP280::getPressure(int32_t & pressure){

    uint32_t adc_T = getTemperatureData();
    uint32_t adc_P = getPressureData();

    if(adc_P == 0)
    {
        pressure = 0;
        return;
    }

    // uint64_t begin_t = nanos();
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

    // uint64_t end_t = nanos();
    // BMP280_DEBUG("cal used", (uint32_t)(end_t - begin_t));
}

void BMP280::init(){
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
    readBurst(RegAddress::DigT1, (uint8_t *)&digT1, 2, 2*12);

    // for(uint16_t * ptr = &digT1; ptr <= (uint16_t *)&digP9; ptr++)
    //     BMP280_DEBUG(String(*ptr, 16));
}
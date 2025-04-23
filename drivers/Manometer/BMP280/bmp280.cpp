#include "bmp280.hpp"

using namespace ymd::drivers;

#define BMP280_DEBUG(...)

bool BMP280::isChipValid(){
    read_reg(RegAddress::ChipID, chipIDReg);
    BMP280_DEBUG("CHIP code: ", uint8_t(chipIDReg));
    return (chipIDReg == valid_chipid);
}

void BMP280::set_temprature_sample_mode(const TempratureSampleMode tempMode){
    ctrlReg.osrs_t = (uint8_t)tempMode;
    write_reg(RegAddress::Ctrl, ctrlReg);
}

void BMP280::set_pressure_sample_mode(const PressureSampleMode pressureMode){
    ctrlReg.osrs_p = (uint8_t)pressureMode;
    write_reg(RegAddress::Ctrl, ctrlReg);
}

void BMP280::set_mode(const Mode mode){
    ctrlReg.mode = (uint8_t)mode;
    write_reg(RegAddress::Ctrl, ctrlReg);
}

void BMP280::set_datarate(const DataRate dataRate){
    configReg.t_sb = (uint8_t)dataRate;
    write_reg(RegAddress::Config, configReg);
}

void BMP280::set_filter_coefficient(const FilterCoefficient filterCoeff){
    configReg.filter = (uint8_t)filterCoeff;
    write_reg(RegAddress::Config, configReg);
}

void BMP280::reset(){
    write_reg(RegAddress::Reset, reset_key);
}

bool BMP280::is_idle(){
    read_reg(RegAddress::Status, statusReg);
    return (statusReg.busy == 0);
}

void BMP280::enable_spi3(const bool en){
    configReg.spi3_en = en;
    write_reg(RegAddress::Config, configReg);
}

int32_t BMP280::get_pressure(){

    uint32_t adc_T = get_temperature_data();
    uint32_t adc_P = get_pressure_data();

    if(adc_P == 0) return 0;

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
    return p+(var1+var2+((float)digP7))/16.0f;

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
    write_reg(RegAddress::Ctrl, (uint8_t)0xFFU);

    set_datarate(DataRate::HZ200);
    set_filter_coefficient(FilterCoefficient::OFF);
    enable_spi3(false);

    read_burst(RegAddress::DigT1, reinterpret_cast<uint8_t *>(&digT1), 2, 2*12);

}
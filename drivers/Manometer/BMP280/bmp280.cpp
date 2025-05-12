#include "bmp280.hpp"

using namespace ymd::drivers;

#define BMP280_DEBUG(...)

using Error = BMP280::Error;

template<typename T = void>
using IResult = BMP280::IResult<T>;

IResult<> BMP280::validate(){
    if(const auto res = read_reg(chipIDReg);
        res.is_err()) return res;
    BMP280_DEBUG("CHIP code: ", uint8_t(chipIDReg));
    if(chipIDReg.as_val() != VALID_CHIPID) 
        return Err(Error::WrongChipId);
    return Ok();
}

IResult<> BMP280::set_temprature_sample_mode(const TempratureSampleMode tempMode){
    ctrlReg.osrs_t = (uint8_t)tempMode;
    return write_reg(ctrlReg);
}

IResult<> BMP280::set_pressure_sample_mode(const PressureSampleMode pressureMode){
    ctrlReg.osrs_p = (uint8_t)pressureMode;
    return write_reg(ctrlReg);
}

IResult<> BMP280::set_mode(const Mode mode){
    ctrlReg.mode = (uint8_t)mode;
    return write_reg(ctrlReg);
}

IResult<> BMP280::set_datarate(const DataRate dataRate){
    configReg.t_sb = (uint8_t)dataRate;
    return write_reg(configReg);
}

IResult<> BMP280::set_filter_coefficient(const FilterCoefficient filterCoeff){
    configReg.filter = (uint8_t)filterCoeff;
    return write_reg(configReg);
}

IResult<> BMP280::reset(){
    resetReg = RESET_KEY;
    return write_reg(resetReg);
}

IResult<bool> BMP280::is_idle(){
    if(const auto res = read_reg(statusReg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(statusReg.busy == 0);
}

IResult<> BMP280::enable_spi3(const bool en){
    configReg.spi3_en = en;
    return write_reg(configReg);
}

IResult<int32_t> BMP280::get_pressure(){

    uint32_t adc_T = ({
        const auto res = read_temp_data();
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });


    uint32_t adc_P = ({
        const auto res = get_pressure_data();
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    
    if(adc_P == 0) return Err(Error::NoPressure);

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
    return Ok(p+(var1+var2+((float)digP7))/16.0f);
}

IResult<> BMP280::init(){
    if(const auto res = reset();
        res.is_err()) return res;

    if(const auto res = validate();
        res.is_err()) return res;
    // BMP280_DEBUG("BMP280 validation:", chip_valid);

    // setMode(Mode::Cont);
    // setTempratureSampleMode(TempratureSampleMode::Bit20);
    // setPressureSampleMode(PressureSampleMode::Bit20);
    if(const auto res = write_reg(CTRL_REG_ADDR, (uint8_t)0xFFU);
        res.is_err()) return res;

    if(const auto res = set_datarate(DataRate::HZ200);
        res.is_err()) return res;
    if(const auto res = set_filter_coefficient(FilterCoefficient::OFF);
        res.is_err()) return res;
    if(const auto res = enable_spi3(false);
        res.is_err()) return res;

    if(const auto res = read_burst(DIGT1_REG_ADDR, reinterpret_cast<int16_t *>(&digT1), 12);
        res.is_err()) return res;
    return Ok();
}

IResult<uint32_t> BMP280::get_pressure_data(){
    uint32_t pressureData = 0;
    if(const auto res = read_reg(pressureReg);
        res.is_err()) return Err(res.unwrap_err());
    pressureData = pressureReg << 4;
    if(const auto res = read_reg(pressureXReg);
        res.is_err()) return Err(res.unwrap_err());
    pressureData |= pressureXReg >> 4;
    // BMP280_DEBUG("PressureData:", pressureData);
    return Ok(pressureData);
}

IResult<uint32_t> BMP280::read_temp_data(){
    uint32_t temperatureData = 0;
    if(const auto res = read_reg(temperatureReg);
        res.is_err()) return Err(res.unwrap_err());
    temperatureData = temperatureReg.as_val() << 4;
    if(const auto res = read_reg(temperatureXReg);
        res.is_err()) return Err(res.unwrap_err());
    temperatureData |= temperatureXReg.as_val() >> 4;
    // BMP280_DEBUG("TempratureData:", temperatureData);
    return Ok(temperatureData);
}
#include "bmp280.hpp"

using namespace ymd::drivers;

#define BMP280_DEBUG(...)

using Error = BMP280::Error;

template<typename T = void>
using IResult = BMP280::IResult<T>;

IResult<> BMP280::validate(){
    if(const auto res = read_reg(chip_id_reg);
        res.is_err()) return res;
    BMP280_DEBUG("CHIP code: ", uint8_t(chip_id_reg));
    if(chip_id_reg.as_val() != VALID_CHIPID) 
        return Err(Error::WrongChipId);
    return Ok();
}

IResult<> BMP280::set_temprature_sample_mode(const TempratureSampleMode tempMode){
    auto reg = RegCopy(ctrl_reg);
    reg.osrs_t = (uint8_t)tempMode;
    return write_reg(reg);
}

IResult<> BMP280::set_pressure_sample_mode(const PressureSampleMode pressureMode){
    auto reg = RegCopy(ctrl_reg);
    reg.osrs_p = (uint8_t)pressureMode;
    return write_reg(reg);
}

IResult<> BMP280::set_mode(const Mode mode){
    auto reg = RegCopy(ctrl_reg);
    reg.mode = (uint8_t)mode;
    return write_reg(reg);
}


IResult<> BMP280::set_datarate(const DataRate dataRate){
    auto reg = RegCopy(config_reg);
    reg.t_sb = (uint8_t)dataRate;
    return write_reg(reg);
}

IResult<> BMP280::set_filter_coefficient(const FilterCoefficient filterCoeff){
    auto reg = RegCopy(config_reg);
    reg.filter = (uint8_t)filterCoeff;
    return write_reg(reg);
}

IResult<> BMP280::reset(){
    auto reg = RegCopy(reset_reg);
    reg.as_ref() = RESET_KEY;
    return write_reg(reg);
}

IResult<bool> BMP280::is_idle(){
    auto & reg = status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.busy == 0);
}

IResult<> BMP280::enable_spi3(const Enable en){
    auto reg = RegCopy(config_reg);
    reg.spi3_en = en == EN;
    return write_reg(reg);
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
    if(const auto res = enable_spi3(DISEN);
        res.is_err()) return res;

    if(const auto res = read_burst(DIGT1_REG_ADDR, reinterpret_cast<int16_t *>(&digT1), 12);
        res.is_err()) return res;
    return Ok();
}

IResult<uint32_t> BMP280::get_pressure_data(){
    uint32_t pressure_data = 0;
    if(const auto res = read_reg(pressure_reg);
        res.is_err()) return Err(res.unwrap_err());
    pressure_data = pressure_reg.as_val() << 4;
    if(const auto res = read_reg(pressure_x_reg);
        res.is_err()) return Err(res.unwrap_err());
    pressure_data |= pressure_x_reg.as_val() >> 4;
    return Ok(pressure_data);
}

IResult<uint32_t> BMP280::read_temp_data(){
    uint32_t temperatureData = 0;
    if(const auto res = read_reg(temperature_reg);
        res.is_err()) return Err(res.unwrap_err());
    temperatureData = temperature_reg.as_val() << 4;
    if(const auto res = read_reg(temperature_x_reg);
        res.is_err()) return Err(res.unwrap_err());
    temperatureData |= temperature_x_reg.as_val() >> 4;
    // BMP280_DEBUG("TempratureData:", temperatureData);
    return Ok(temperatureData);
}
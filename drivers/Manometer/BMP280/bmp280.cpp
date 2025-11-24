#include "bmp280.hpp"

using namespace ymd::drivers;

#define BMP280_DEBUG(...)

using Self = BMP280;

using Error = Self::Error;

template<typename T = void>
using IResult = Self::IResult<T>;

static constexpr uint8_t RESET_REG_ADDR = 0xE0;
static constexpr uint8_t DIGT1_REG_ADDR = 0x88;
static constexpr uint8_t CTRL_REG_ADDR = 0xF4;

IResult<> Self::validate(){
    auto & reg = regs_.chipid_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return res;
    BMP280_DEBUG("CHIP code: ", uint8_t(chip_id_reg));
    if(reg.to_bits() != VALID_CHIPID) 
        return Err(Error::WrongChipId);
    return Ok();
}

IResult<> Self::set_temprature_sample_mode(const TempratureSampleMode temp_mode){
    auto reg = RegCopy(regs_.ctrl_reg);
    reg.osrs_t = temp_mode;
    return write_reg(reg);
}

IResult<> Self::set_pressure_sample_mode(const PressureSampleMode pressure_mode){
    auto reg = RegCopy(regs_.ctrl_reg);
    reg.osrs_p = pressure_mode;
    return write_reg(reg);
}

IResult<> Self::set_mode(const Mode mode){
    auto reg = RegCopy(regs_.ctrl_reg);
    reg.mode = mode;
    return write_reg(reg);
}


IResult<> Self::set_datarate(const DataRate data_rate){
    auto reg = RegCopy(regs_.config_reg);
    reg.t_sb = data_rate;
    return write_reg(reg);
}

IResult<> Self::set_filter_coefficient(const FilterCoefficient filter_coeff){
    auto reg = RegCopy(regs_.config_reg);
    reg.filter = filter_coeff;
    return write_reg(reg);
}

IResult<> Self::reset(){
    auto reg = RegCopy(regs_.reset_reg);
    reg.as_bits_mut() = RESET_KEY;
    return write_reg(reg);
}

IResult<bool> Self::is_idle(){
    auto & reg = regs_.status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.busy == 0);
}

IResult<> Self::enable_3wire_spi(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.spi3_en = en == EN;
    return write_reg(reg);
}

IResult<int32_t> Self::get_pressure(){

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

    return Ok(coeffs_.convert(adc_T, adc_P));
}

IResult<> Self::init(const Config & cfg){
    if(const auto res = reset();
        res.is_err()) return res;

    if(const auto res = validate();
        res.is_err()) return res;

    if(const auto res = write_reg(CTRL_REG_ADDR, (uint8_t)0xFFU);
        res.is_err()) return res;

    if(const auto res = set_datarate(cfg.datarate);
        res.is_err()) return res;
    if(const auto res = set_filter_coefficient(FilterCoefficient::OFF);
        res.is_err()) return res;
    if(const auto res = enable_3wire_spi(DISEN);
        res.is_err()) return res;

    if(const auto res = read_burst(DIGT1_REG_ADDR, 
        std::span(reinterpret_cast<int16_t *>(&coeffs_.digT1), 12));
        res.is_err()) return res;
    return Ok();
}

IResult<uint32_t> Self::get_pressure_data(){
    uint32_t pressure_data = 0;
    if(const auto res = read_reg(regs_.pressure_reg);
        res.is_err()) return Err(res.unwrap_err());
    pressure_data = regs_.pressure_reg.to_bits() << 4;
    if(const auto res = read_reg(regs_.pressure_x_reg);
        res.is_err()) return Err(res.unwrap_err());
    pressure_data |= regs_.pressure_x_reg.to_bits() >> 4;
    return Ok(pressure_data);
}

IResult<uint32_t> Self::read_temp_data(){
    uint32_t temperatureData = 0;
    if(const auto res = read_reg(regs_.temperature_reg);
        res.is_err()) return Err(res.unwrap_err());
    temperatureData = regs_.temperature_reg.to_bits() << 4;
    if(const auto res = read_reg(regs_.temperature_x_reg);
        res.is_err()) return Err(res.unwrap_err());
    temperatureData |= regs_.temperature_x_reg.to_bits() >> 4;
    // BMP280_DEBUG("TempratureData:", temperatureData);
    return Ok(temperatureData);
}
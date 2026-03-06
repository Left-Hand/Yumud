#include "ina226.hpp"

#include "core/debug/debug.hpp"

#include "primitive/pwm_channel.hpp"


using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


#define INA226_DEBUG_EN 1
// #define INA226_DEBUG_EN 0

#if INA226_DEBUG_EN
#define INA226_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA226_PANIC(...) PANIC(__VA_ARGS__)
#define INA226_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define INA226_DEBUG(...)
#define INA226_PANIC(...)  PANIC_NSRC()
#define INA226_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif

using Self = INA226;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

template<typename T>
requires (std::is_integral_v<T>)
static constexpr T diff(T a, T b){
    return a > b ? a - b : b - a;
}



// static constexpr auto f = (float)sv_code_to_volts(volts_to_sv_code(-0.08_iq16));
// static constexpr auto f = (float)bv_code_to_volts(0x7fff);
// static constexpr auto f = (float)bv_code_to_volts(0x8300);


static_assert(Self::volts_to_sv_code(-0.08_iq16) == 0x8300);
static_assert(Self::mv_to_sv_code(-80) == 0x8300);
static_assert(Self::uv_to_sv_code(-80'000) == 0x8300);

static_assert(std::abs((float)Self::sv_code_to_volts(0x8300) - (-0.08)) < 1E-4);
static_assert(Self::sv_code_to_mv(0x8300) == -80);
static_assert(Self::sv_code_to_uv(0x8300) == -80'000);


static_assert(std::abs((float)Self::bv_code_to_volts(0x7fff) - (40.96)) < 2E-4);
static_assert(diff(Self::bv_code_to_mv(0x7fff), uint32_t(40.96 * 1000)) < 2);
static_assert(diff(Self::bv_code_to_uv(0x7fff), uint32_t(40.96 * 1000'000)) < 10);


    //    _1 = 0,
    //     _4 = 1,
    //     _16 = 2,
    //     _64 = 3,
    //     _128 = 4,
    //     _256 = 5,
    //     _512 = 6,
    //     _1024 = 7

static constexpr Self::AverageTimes times_to_avtimes(const uint16_t times){
    const uint8_t temp = __builtin_ctz(times);

    if(times <= 64){
        return std::bit_cast<Self::AverageTimes>(uint16_t(temp / 2));
    }else{
        return std::bit_cast<Self::AverageTimes>(uint16_t(4 + (temp - 7))); 
    }
} 

static_assert(times_to_avtimes(1) == Self::AverageTimes::_1);
static_assert(times_to_avtimes(4) == Self::AverageTimes::_4);
static_assert(times_to_avtimes(16) == Self::AverageTimes::_16);
static_assert(times_to_avtimes(64) == Self::AverageTimes::_64);
static_assert(times_to_avtimes(128) == Self::AverageTimes::_128);
static_assert(times_to_avtimes(256) == Self::AverageTimes::_256);
static_assert(times_to_avtimes(512) == Self::AverageTimes::_512);
static_assert(times_to_avtimes(1024) == Self::AverageTimes::_1024);


IResult<> INA226::set_average_times(const uint16_t times){
    return set_average_times(times_to_avtimes(times));
}

IResult<> INA226::set_average_times(const AverageTimes times){
    auto reg = RegCopy(regs_.config_reg);
    reg.average_times = times;
    return write_reg(reg);
}

// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/ina226-pressure-current-power-module.html

IResult<iq16> INA226::get_current(){
    return Ok(std::bit_cast<int16_t>(regs_.current_reg.bits) * current_lsb_ma_ / 1000);
    // return current_reg.to_bits() ;
}

IResult<iq16> INA226::get_power(){
    return Ok(std::bit_cast<uint16_t>(regs_.power_reg.bits) * current_lsb_ma_ / 40);
    // return power_reg.to_bits();
}

IResult<> INA226::set_bus_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.config_reg);
    reg.bus_voltage_conversion_time = time;
    return write_reg(reg);
}

IResult<> INA226::set_shunt_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.config_reg);
    reg.shunt_voltage_conversion_time = time;
    return write_reg(reg);
}

IResult<Self::BusVoltageCode> INA226::get_bus_voltage_code(){
    return Ok(regs_.bus_volt_reg.code);
}
IResult<Self::ShuntVoltageCode> INA226::get_shunt_voltage_code(){
    return Ok(regs_.shunt_volt_reg.code);
}

IResult<> INA226::update(){
    if(const auto res = this->read_reg(regs_.bus_volt_reg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(regs_.current_reg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(regs_.power_reg);
        res.is_err()) return res;

    return Ok();
}



IResult<> INA226::init(const Config & cfg){
    auto & config_reg = regs_.config_reg;
    config_reg.rst = 0b0;
    config_reg.__resv__ = 0b100;

    INA226_DEBUG("init");
    
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = reset();
        res.is_err()) return res;
    if(const auto res = set_average_times(cfg.average_times);
        res.is_err()) return res;
    if(const auto res = set_bus_conversion_time(cfg.bus_conv_time);
        res.is_err()) return res;
    if(const auto res = set_shunt_conversion_time(cfg.shunt_conv_time);
        res.is_err()) return res;
    if(const auto res = enable_bus_voltage_measure(EN);
        res.is_err()) return res;
    if(const auto res = enable_continuous_measure(EN);
        res.is_err()) return res;
    if(const auto res = enable_shunt_voltage_measure(EN);
        res.is_err()) return res;
    if(const auto res = set_scale(cfg.sample_res_mohms, cfg.max_current_a);
        res.is_err()) return res;
    return Ok();
}

IResult<> INA226::set_scale(const uint32_t sample_res_mohms, const uint32_t max_current_a){
    INA226_DEBUG(sample_res_mohms, max_current_a);
    
    INA226_ASSERT(max_current_a < (std::numeric_limits<uint32_t>::max() / 1000));
    uint32_t max_current_ma = uint32_t(max_current_a) * 1000u;
    current_lsb_ma_ = iq16(iq15::from_bits(max_current_ma));
    // INA226_DEBUG(current_lsb_ma_, sample_res_mohms * max_current_a);


    // const uint32_t cal = uint32_t(0.00512 * 32768) / (sample_res_ohms * max_current_ma);
    const uint32_t cal = uint32_t(0.00512 * 32768 * 1000) / (sample_res_mohms * max_current_a);
    const uint16_t cal_masked = cal & 0x7fff;
    INA226_ASSERT(static_cast<uint32_t>(cal_masked) == cal);

    {
        auto reg = RegCopy(regs_.calibration_reg);
        reg.as_bits_mut() = int16_t(cal);
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}



IResult<> INA226::reset(){
    auto reg = RegCopy(regs_.config_reg);
    reg.rst = 1;
    return write_reg(reg);
}

IResult<> INA226::enable_shunt_voltage_measure(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.shunt_voltage_enable = en == EN;
    return write_reg(reg);
}

IResult<> INA226::enable_bus_voltage_measure(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.bus_voltage_enable = en == EN;
    return write_reg(reg);
}

IResult<> INA226::enable_continuous_measure(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.continuos = en == EN;
    return write_reg(reg);
}

IResult<> INA226::enable_alert_latch(const Enable en){
    auto  reg = RegCopy(regs_.mask_reg);
    reg.alert_latch_enable = en == EN;
    return write_reg(reg);
}

IResult<> INA226::validate(){
    if(const auto res = i2c_drv_.validate(); res.is_err()){
        return CHECK_ERR(Err(res.unwrap_err()), "INA226 i2c lost");
    }

    auto & chip_id_reg = regs_.chip_id_reg;
    auto & manufacture_reg = regs_.manufacture_reg;

    if(const auto res = this->read_reg(chip_id_reg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(manufacture_reg);
        res.is_err()) return res;

    if((chip_id_reg.to_bits() != VALID_CHIP_ID)) 
        return CHECK_ERR(Err(Error::ChipIdVerifyFailed));
    if((manufacture_reg.to_bits() != VALID_MANU_ID)) 
        return CHECK_ERR(Err(Error::ManuIdVerifyFailed));

    return Ok();
}

IResult<> INA226::write_reg(const RegAddr addr, const uint16_t data){
    if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA226::read_reg(const RegAddr addr, uint16_t & data){
    if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA226::read_reg(const RegAddr addr, int16_t & data){
    if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}
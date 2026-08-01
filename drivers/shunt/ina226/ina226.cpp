#include "ina226.hpp"

#include "core/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


#define INA226_DEBUG_EN 1
// #define INA226_DEBUG_EN 0

#if INA226_DEBUG_EN
#define INA226_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA226_PANIC(...) PANIC(__VA_ARGS__)
#define INA226_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}




#define RAISE_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define INA226_DEBUG(...)
#define INA226_PANIC(...)  PANIC_NSRC()
#define INA226_ASSERT(cond, ...) ASSERT_NSRC(cond)


#define RAISE_ERR(x, ...) (x)
#endif

using Self = INA226;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

// from datasheet
// Current_LSB = MAX_CURRENT / 2^15
// CAL = 0.00512 / (Current_LSB * R_shunt)

constexpr uint32_t SHUNTCAL_MASK = 0x7fff;

namespace{

#define DEF_CALC_CURRLSB(max_current_ma)\
    iq16(iq15::from_bits(max_current_ma))

#define DEF_CALC_CAL(sample_res_mohms, max_current_ma)\
    uint32_t(0.00512 * 32768 * 1000 * 1000) / (sample_res_mohms * max_current_ma)


static constexpr iq16 calc_currlsb(
    const uint32_t max_current_ma
){
    return DEF_CALC_CURRLSB(max_current_ma);
}


static constexpr uint32_t calc_cal(
    const uint32_t sample_res_mohms, 
    const uint32_t max_current_ma
){

    const uint32_t cal = DEF_CALC_CAL(sample_res_mohms, max_current_ma);
    return cal;
}

[[maybe_unused]] static void test_calc(){
    {//cal
        #define TEST_CASE(sample_res_mohms, max_current_ma)\
        {\
            static constexpr auto flt_val = DEF_CALC_CAL(float(sample_res_mohms), float(max_current_ma));\
            static constexpr auto int_val = calc_cal(sample_res_mohms, max_current_ma);\
            static_assert(int_val < SHUNTCAL_MASK);\
            static_assert(math::abs(flt_val - int_val) < 2.0f);\
        }\

        TEST_CASE(6, 6000)
        TEST_CASE(6, 16000)
        TEST_CASE(1, 16000)
        TEST_CASE(100, 16000)

        #undef TEST_CASE
    }

    {
        #define TEST_CASE(max_current_ma)\
        {\
            static constexpr auto flt_val = max_current_ma * (1.0 / (1 << 15));\
            static constexpr auto fx_val = calc_currlsb(max_current_ma);\
            static_assert(math::abs(flt_val - (float)fx_val) < 1e-4);\
        }\

        TEST_CASE(6000)
        TEST_CASE(16000)
        TEST_CASE(16000)
        TEST_CASE(16000)

        #undef TEST_CASE
    }

}

#undef DEF_CALC_CAL
#undef DEF_CALC_CURRLSB

}


IResult<> INA226::init(const Config & cfg){
    regs_.reset_initial_value();

    INA226_DEBUG("init");
    
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = soft_reset();
        res.is_err()) return res;
    if(const auto res = set_average_times(cfg.average_times);
        res.is_err()) return res;
    if(const auto res = set_busbar_conversion_time(cfg.bus_conv_time);
        res.is_err()) return res;
    if(const auto res = set_shunt_conversion_time(cfg.shunt_conv_time);
        res.is_err()) return res;
    if(const auto res = enable_busbar_measure(EN);
        res.is_err()) return res;
    if(const auto res = enable_continuous_measure(EN);
        res.is_err()) return res;
    if(const auto res = enable_shunt_measure(EN);
        res.is_err()) return res;
    if(const auto res = set_scale(cfg.sample_res_mohms, cfg.max_current_ma);
        res.is_err()) return res;
    return Ok();
}


IResult<> INA226::set_scale(const uint32_t sample_res_mohms, const uint32_t max_current_ma){
    INA226_DEBUG(sample_res_mohms, max_current_ma);
    
    current_lsb_ma_ = calc_currlsb(max_current_ma);

    {
        const auto shunt_cal = ({
            const auto may_cal = calc_cal(sample_res_mohms, max_current_ma);
            if(may_cal > SHUNTCAL_MASK) return Err(Error::SolveFailed);
            may_cal;
        });

        if(const auto res = write_reg(Regset::R16_Calibration::REG_ADDR, int16_t(shunt_cal));
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}




IResult<> INA226::soft_reset(){
    auto reg = RegCopy(regs_.config_reg);
    reg.rst = 1;
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    // this bit self-clears by hardware after the reset is complete. The reset bit will always read as 0.
    reg.rst = 0;
    reg.commit_changes();
    return Ok();
}

IResult<> INA226::set_average_times(const AverageTimes times){
    auto reg = RegCopy(regs_.config_reg);
    reg.average_times = times;
    return write_reg(reg);
}

// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/ina226-pressure-current-power-module.html

IResult<iq16> INA226::get_current(){
    auto reg = Regset::R16_Current{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(std::bit_cast<int16_t>(reg.bits) * current_lsb_ma_ / 1000);
}

IResult<iq16> INA226::get_power(){
    auto reg = Regset::R16_Power{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(std::bit_cast<uint16_t>(reg.bits) * current_lsb_ma_ / 40);
}

IResult<Self::BusbarVoltageCode> INA226::get_busbar_voltage_code(){
    auto reg = Regset::R16_BusVolt{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.code);
}


IResult<Self::ShuntVoltageCode> INA226::get_shunt_voltage_code(){
    auto reg = Regset::R16_ShuntVolt{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.code);
}


IResult<> INA226::set_busbar_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.config_reg);
    reg.bus_voltage_conversion_time = time;
    return write_reg(reg);
}

IResult<> INA226::set_shunt_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.config_reg);
    reg.shunt_voltage_conversion_time = time;
    return write_reg(reg);
}




IResult<> INA226::enable_shunt_measure(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.shunt_voltage_enable = (en == EN);
    return write_reg(reg);
}

IResult<> INA226::enable_busbar_measure(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.bus_voltage_enable = (en == EN);
    return write_reg(reg);
}

IResult<> INA226::enable_continuous_measure(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.continuos = (en == EN);
    return write_reg(reg);
}

IResult<> INA226::enable_alert_latch(const Enable en){
    auto  reg = RegCopy(regs_.mask_reg);
    reg.alert_latch_en = (en == EN);
    return write_reg(reg);
}

IResult<> INA226::validate(){
    if(const auto res = i2c_drv_.validate(); res.is_err()){
        return RAISE_ERR(Err(res.unwrap_err()), "INA226 i2c lost");
    }

    auto chip_id_reg = Regset::R16_ChipId{};
    auto manufacture_reg = Regset::R16_Manufacture{};

    if(const auto res = this->read_reg(chip_id_reg);
        res.is_err()) return res;
    if(const auto res = this->read_reg(manufacture_reg);
        res.is_err()) return res;

    if((chip_id_reg.to_bits() != VALID_CHIP_ID)) 
        return RAISE_ERR(Err(Error::ChipIdVerifyFailed));
    if((manufacture_reg.to_bits() != VALID_MANU_ID)) 
        return RAISE_ERR(Err(Error::ManuIdVerifyFailed));

    return Ok();
}

IResult<> INA226::write_reg(const RegAddr reg_addr, const uint16_t reg_val){
    if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA226::read_reg(const RegAddr reg_addr, uint16_t & reg_val){
    if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA226::read_reg(const RegAddr reg_addr, int16_t & reg_val){
    if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::big);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}
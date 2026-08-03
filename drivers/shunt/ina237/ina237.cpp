#include "ina237.hpp"

#include "core/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;
using namespace ymd;


#define INA237_DEBUG_EN 1
// #define INA237_DEBUG_EN 0

#if INA237_DEBUG_EN
#define INA237_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA237_PANIC(...) PANIC(__VA_ARGS__)
#define INA237_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}




#define RAISE_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define INA237_DEBUG(...)
#define INA237_PANIC(...)  PANIC_NSRC()
#define INA237_ASSERT(cond, ...) ASSERT_NSRC(cond)


#define RAISE_ERR(x, ...) (x)
#endif

using Self = INA237;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;



// ====================
// === equations
// ====================

// from datasheet
// Current_LSB = MAX_CURRENT / 2^15
// CAL = 819.2 * 1e6 / (Current_LSB * R_shunt)

// => cal = sample_res_mohms * max_current_ma * 0.025

// Current [A] = CURRENT_LSB x CURRENT
// Power [W] = 0.2 x CURRENT_LSB x POWER


static constexpr uint32_t SHUNTCAL_MASK = 0x7fff;

namespace{


#define DEF_CALC_CURRLSB(max_current_ma)\
    iq16(iq15::from_bits(max_current_ma))

#define DEF_CALC_CAL(sample_res_mohms, max_current_ma)\
    ((max_current_ma * sample_res_mohms) / 40)

// ±163.84mV (ADCRANGE = 0)
// ±40.96mV (ADCRANGE = 1)
[[nodiscard]] static constexpr bool need_range0(
    const uint32_t sample_res_mohms, 
    const uint32_t max_current_ma
){
    //R / I > 0.04096 V
    return 1000'000 * max_current_ma > 4096 * sample_res_mohms;
}


static constexpr iq16 calc_currlsb(
    const uint32_t max_current_ma
){
    return DEF_CALC_CURRLSB(max_current_ma);
}


static constexpr uint32_t calc_cal_range0(
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
            static constexpr auto int_val = calc_cal_range0(sample_res_mohms, max_current_ma);\
            static_assert(int_val <= SHUNTCAL_MASK);\
            static_assert(math::abs(flt_val - int_val) < 2.0f);\
        }\

        TEST_CASE(6, 6000)
        TEST_CASE(6, 16000)
        TEST_CASE(1, 16000)
        TEST_CASE(10, 16000)

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


    {
        auto calc_err = [&] (const float celsius) consteval -> float{
            const auto code = Self::celsius_encode(iq20(celsius));
            const auto recovery = (float)Self::celsius_decode(code);
            return std::abs(celsius - recovery);
        };

        static_assert(calc_err(2.0f) < 1e-4);
        static_assert(calc_err(12.0f) < 1e-4);
        static_assert(calc_err(222.25f) < 1e-4);
        static_assert(calc_err(-22.0f) < 1e-4);
        static_assert(calc_err(13.75f) < 1e-4);
    }
}

#undef DEF_CALC_CAL
#undef DEF_CALC_CURRLSB

}


#define DEF_IS_RANGE0 (bool(regs_.config_reg.ADCRANGE == 0))

#define DEF_SHUNTCODE2VALUE(code) (\
    (DEF_IS_RANGE0) ? code.to_shuntvalue_range0() : code.to_shuntvalue_range1())\


#define DEF_SHUNTVALUE2CODE(value) (\
    (DEF_IS_RANGE0) ? \
        Self::ShuntVoltageCode::from_shuntvalue_range0(value) : \
        Self::ShuntVoltageCode::from_shuntvalue_range1(value))\

IResult<> INA237::init(const Config & cfg){
    regs_.reset_initial_value();

    INA237_DEBUG("init");
    
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
    TODO();
    return Ok();
}

IResult<> INA237::set_scale(const uint32_t sample_res_mohms, const uint32_t max_current_ma){
    INA237_DEBUG(sample_res_mohms, max_current_ma);
    
    const bool is_range0 = need_range0(sample_res_mohms, max_current_ma);

    {
        auto reg = RegCopy(regs_.config_reg);
        reg.ADCRANGE = is_range0 ? 0 : 1;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    current_lsb_ma_ = calc_currlsb(max_current_ma);

    {
        auto shunt_cal = calc_cal_range0(sample_res_mohms, max_current_ma);

        // page28: ADCRANGE = 1 时，SHUNT_CAL 的值必须乘以 4
        if(not is_range0) {
            shunt_cal <<= 2;
        }

        if(shunt_cal > SHUNTCAL_MASK) return Err(Error::SolveFailed);

        auto reg = RegCopy(regs_.shunt_cal_reg);
        reg.as_bits_mut() = int16_t(shunt_cal);
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}




IResult<> INA237::soft_reset(){
    auto reg = RegCopy(regs_.config_reg);
    reg.RST = 1;
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    // this bit self-clears by hardware after the reset is complete. The reset bit will always read as 0.
    reg.RST = 0;
    reg.commit_changes();
    return Ok();
}


IResult<> INA237::validate(){
    if(const auto res = i2c_drv_.validate(); res.is_err()){
        return RAISE_ERR(Err(res.unwrap_err()), "INA237 i2c lost");
    }

    auto manufacture_reg = Regset::R16_ManfId{};

    if(const auto res = this->read_reg(manufacture_reg);
        res.is_err()) return res;

    if((manufacture_reg.to_bits() != VALID_MANU_ID)) 
        return RAISE_ERR(Err(Error::ManuIdVerifyFailed));

    return Ok();
}

// datasheet: Current [A] = CURRENT_LSB x CURRENT
IResult<iq16> INA237::get_current(){
    auto reg = Regset::R16_Current{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(std::bit_cast<int16_t>(reg.bits) * current_lsb_ma_ / 1000);
}

// Power [W] = 0.2 x CURRENT_LSB x POWER
IResult<iq16> INA237::get_power(){
    auto reg = Regset::R24_Power{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(reg.bits * current_lsb_ma_ / 5000);
}

IResult<INA237::TemperatureCode> INA237::get_temperature(){
    auto reg = Regset::R16_DieTemp{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(reg.code);
}

IResult<Self::BusbarVoltageCode> INA237::get_busbar_voltage_code(){
    auto reg = Regset::R16_Vbus{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.code);
}


IResult<Self::ShuntValue> INA237::get_shunt_voltage_code(){
    auto reg = Regset::R16_Vshunt{};
    if(const auto res = this->read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    const auto code = reg.code;
    const auto sv = DEF_SHUNTCODE2VALUE(code);
    return Ok(sv);
}


IResult<> INA237::set_shunt_alert_threshold(
    const Option<ShuntValue> mi, 
    const Option<ShuntValue> ma
){

    if(mi.is_some()){
        const auto sv = mi.unwrap();
        const auto code = DEF_SHUNTVALUE2CODE(sv);
        if(const auto res = write_reg(Regset::R16_SUVL::REG_ADDR, code.bits);
            res.is_err()) return Err(res.unwrap_err());
    }

    if(ma.is_some()){
        const auto sv = ma.unwrap();
        const auto code = DEF_SHUNTVALUE2CODE(sv);
        if(const auto res = write_reg(Regset::R16_SOVL::REG_ADDR, code.bits);
            res.is_err()) return Err(res.unwrap_err());
    }


    return Ok();
}


IResult<> INA237::set_busbar_alert_threshold(
    const Option<BusbarVoltageCode> mi, 
    const Option<BusbarVoltageCode> ma
){

    if(mi.is_some()){
        const auto code = mi.unwrap();
        if(const auto res = write_reg(Regset::R16_BUVL::REG_ADDR, code.bits);
            res.is_err()) return Err(res.unwrap_err());
    }

    if(ma.is_some()){
        const auto code = ma.unwrap();
        if(const auto res = write_reg(Regset::R16_BOVL::REG_ADDR, code.bits);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}


IResult<> INA237::set_temperature_alert_threshold(const TemperatureCode ma){
    return write_reg(Regset::R16_TempLimit::REG_ADDR, ma.bits);
}

IResult<> INA237::set_average_times(const AverageTimes times){
    auto reg = RegCopy(regs_.adc_config_reg);
    reg.AVG = times;
    return write_reg(reg);
}

IResult<> INA237::set_busbar_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.adc_config_reg);
    reg.VBUSCT = time;
    return write_reg(reg);
}

IResult<> INA237::set_shunt_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.adc_config_reg);
    reg.VSHCT = time;
    return write_reg(reg);
}



IResult<> INA237::enable_shunt_measure(const Enable en){
    auto reg = RegCopy(regs_.adc_config_reg);
    reg.SHUNT_MEAS_EN = (en == EN);
    return write_reg(reg);
}

IResult<> INA237::enable_busbar_measure(const Enable en){
    auto reg = RegCopy(regs_.adc_config_reg);
    reg.BUSBAR_MEAS_EN = (en == EN);
    return write_reg(reg);
}

IResult<> INA237::enable_continuous_measure(const Enable en){
    auto reg = RegCopy(regs_.adc_config_reg);
    reg.CONTMODE_EN = (en == EN);
    return write_reg(reg);
}


IResult<> INA237::write_reg(const RegAddr reg_addr, const uint16_t reg_val){
    const uint8_t bytes[2] = {
        uint8_t(reg_val >> 8),
        uint8_t(reg_val)
    };

    return write_bulk(reg_addr, bytes, 2);
}

IResult<> INA237::read_reg(const RegAddr reg_addr, uint16_t & reg_val){
    return read_bulk(reg_addr, reinterpret_cast<uint8_t *>(&reg_val), 2);
}

IResult<> INA237::read_reg(Regset::R24_Power & reg){
    return read_bulk(reg.REG_ADDR, reinterpret_cast<uint8_t *>(reg.bits), 3);
}

IResult<> INA237::write_bulk(const uint8_t reg_addr, const uint8_t *data, uint16_t size){
    if(const auto res = i2c_drv_.write_bulk(
        reg_addr, 
        std::span<const uint8_t>(data, static_cast<size_t>(size)),
        std::endian::big
    ); res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> INA237::read_bulk(const uint8_t reg_addr, uint8_t *data, uint16_t size){
    if(const auto res = i2c_drv_.read_bulk(
        reg_addr, 
        std::span<uint8_t>(data, static_cast<size_t>(size)),
        std::endian::big
    ); res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


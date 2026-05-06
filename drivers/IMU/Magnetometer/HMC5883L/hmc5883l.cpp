#include "hmc5883l.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifdef HMC5883L_DEBUG_EN
#define HMC5883L_TODO(...) TODO()
#define HMC5883L_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define HMC5883L_PANIC(...) PANIC{__VA_ARGS__}
#define HMC5883L_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    PANIC{#x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define HMC5883L_DEBUG(...)
#define HMC5883L_TODO(...) PANIC_NSRC()
#define HMC5883L_PANIC(...)  PANIC_NSRC()
#define HMC5883L_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif


using Self = HMC5883L;

using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;


// The temperature output registers are two 8-bit registers, temperature output register H and temperature output register L.
// These registers store the measurement result from the internal temperature sensor. Temperature output register H
// contains the MSB from the measurement result, and temperature output register L contains the LSB from the
// measurement result. The value stored in these two registers is a 16-bit value in 2’s complement form, whose range is
// 0xF800 to 0x07FF. TEMPH0 through TEMPH7 and TEMPL0 through TEMPL7 indicate bit locations, with TEMPH and
// TEMPL denoting the bits that are in the temperature output registers. TEMPH7 and TEMPL7 denote the first bit of the
// data stream. The number in parenthesis indicates the default value of that bit.
// Temperature = (MSB * 2^8 + LSB) / (2^4 * 8) + 25 in C

static constexpr iq16 convert_temperature(const uint16_t bits){
    // return iq16::from_bits(std::bit_cast<int16_t>(bits) << 8);
    return iq16(std::bit_cast<int16_t>(bits)) * uq32(1.0 / (16 * 8)) + 25;
}


namespace{
[[maybe_unused]] void test_convert_temperature(){
    constexpr float f0 = (float)convert_temperature(0x0000);
    constexpr float f1 = (float)convert_temperature(0x07ff);
    constexpr float f2 = (float)convert_temperature(0xf800);
    static_assert(std::abs(f0 - 25.0) <= 1E-2);
    static_assert(std::abs(f1 - 41.0) <= 1E-2);
    static_assert(std::abs(f2 - 9.0) <= 1E-2);
}
}

static constexpr iq24 transfrom_gain_into_lsb(const Self::Gain gain){
    switch(gain){
    case Self::Gain::GL0_73:
        return iq24(0.73);
    case Self::Gain::GL0_92:
        return iq24(0.92);
    case Self::Gain::GL1_22:
        return iq24(1.22);
    case Self::Gain::GL1_52:
        return iq24(1.52);
    case Self::Gain::GL2_27:
        return iq24(2.27);
    case Self::Gain::GL2_56:
        return iq24(2.56);
    case Self::Gain::GL3_03:
        return iq24(3.03);
    case Self::Gain::GL4_35:
        return iq24(4.35);
    default: __builtin_unreachable();
    }
}

static constexpr iq16 transform_raw_to_gauss(const uint16_t data, const iq24 lsb){
    return iq16::from_bits(data & 0x8fff) * lsb;
}

void HMC5883L::set_lsb(const Gain gain){
    lsb_ = transfrom_gain_into_lsb(gain);
}


IResult<> HMC5883L::init(){
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = enable_high_speed(EN);
        res.is_err()) return res;
    if(const auto res = set_mode(Mode::Continuous);
        res.is_err()) return res;
    if(const auto res = set_odr(Odr::DR75);
        res.is_err()) return res;
    if(const auto res = set_sample_number(SampleNumber::SN1);
        res.is_err()) return res;
    if(const auto res = set_gain(Gain::GL1_52);
        res.is_err()) return res;
    return Ok();
}

IResult<> HMC5883L::enable_high_speed(const Enable en){
    auto reg = RegCopy(regs_.mode_reg);
    reg.hs = (en == EN);
    return write_reg(reg);
}


IResult<> HMC5883L::set_odr(const Odr rate){
    auto reg = RegCopy(regs_.config_a_reg);
    reg.dataRate = static_cast<uint8_t>(rate);
    return write_reg(reg);
}

IResult<> HMC5883L::set_sample_number(const SampleNumber number){
    auto reg = RegCopy(regs_.config_a_reg);
    reg.sampleNumber = static_cast<uint8_t>(number);
    return write_reg(reg);
}

IResult<> HMC5883L::set_gain(const Gain gain){
    auto reg = RegCopy(regs_.config_b_reg);
    reg.gain = gain;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    set_lsb(gain);
    return Ok();
}

IResult<> HMC5883L::set_mode(const Mode mode){
    auto reg = RegCopy(regs_.mode_reg);
    reg.mode = mode;
    return write_reg(reg);
}

IResult<math::Vec3<iq24>> HMC5883L::read_mag(){
    iq16 x = transform_raw_to_gauss(regs_.xyz[0], lsb_);
    iq16 y = transform_raw_to_gauss(regs_.xyz[1], lsb_);
    iq16 z = transform_raw_to_gauss(regs_.xyz[2], lsb_);

    return Ok(math::Vec3<iq24>(x,y,z));
}

IResult<> HMC5883L::validate(){
    std::array<uint8_t, 3> buf;
    if(const auto res = read_bulk(RegAddr::IDA, buf);
        res.is_err()) return res;

    bool passed = (
        buf[0] == 'H'
        and buf[1] == '4' 
        and buf[2] == '3');

    if(!passed){
        return Err(Error::InvalidChipId);
    }
    return Ok();
}

IResult<> HMC5883L::update(){
    return read_bulk(RegAddr::MagX, std::span(regs_.xyz));
}


IResult<bool> HMC5883L::is_data_ready(){
    auto & reg = regs_.status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.ready == false);
}

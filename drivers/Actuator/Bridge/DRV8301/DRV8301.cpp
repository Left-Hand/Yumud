#include "DRV8301.hpp"


#define DRV8301_DEBUG

#ifdef DRV8301_DEBUG
#undef DRV8301_DEBUG
#define DRV8301_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV8301_PANIC(...) PANIC(__VA_ARGS__)
#define DRV8301_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define DRV8301_DEBUG(...)
#define DRV8301_PANIC(...)  PANIC()
#define DRV8301_ASSERT(cond, ...) ASSERT(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Error = DRV8301::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> DRV8301::init(){
    TODO();
    return Ok();
}

IResult<> DRV8301::set_peak_current(const PeakCurrent peak_current){
    auto reg = RegCopy(ctrl1_reg);
    reg.gate_current = uint16_t(peak_current);
    return write_reg(reg);
}

IResult<> DRV8301::set_ocp_mode(const OcpMode ocp_mode){
    auto reg = RegCopy(ctrl1_reg);
    reg.ocp_mode = uint16_t(ocp_mode);
    return write_reg(reg);
}


IResult<> DRV8301::set_octw_mode(const OctwMode octw_mode){
    auto reg = RegCopy(ctrl2_reg);
    reg.octw_mode = uint16_t(octw_mode);
    return write_reg(reg);
}

IResult<> DRV8301::set_gain(const Gain gain){
    auto reg = RegCopy(ctrl2_reg);
    reg.gain = uint16_t(gain);
    return write_reg(reg);
}

IResult<> DRV8301::set_oc_ad_table(const OcAdTable oc_ad_table){
    auto reg = RegCopy(ctrl1_reg);
    reg.oc_adj_set = uint8_t(oc_ad_table);
    return write_reg(reg);
}

IResult<> DRV8301::enable_pwm3(const Enable en){
    auto reg = RegCopy(ctrl1_reg);
    reg.pwm3_en = en == EN;
    return write_reg(reg);
}

struct Payload{
    uint16_t data:11;
    uint16_t addr:4;
    uint16_t write:1;

    operator uint16_t() const{
        return std::bit_cast<uint16_t>(*this);
    }

    operator uint16_t &(){
        return *reinterpret_cast<uint16_t *>(this);
    }
};

static_assert(sizeof(Payload) == 2);

IResult<> DRV8301::write_reg(const RegAddress addr, const uint16_t reg){
    const Payload payload = {
        .data = reg,
        .addr = uint16_t(addr),
        .write = 0
    };

    if(const auto res = spi_drv_.write_single<uint16_t>((payload));
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> DRV8301::read_reg(const RegAddress addr, uint16_t & reg){
    Payload payload = {
        .data = 0,
        .addr = uint16_t(addr),
        .write = 1
    };

    const auto res = spi_drv_.read_single<uint16_t>((payload));
    if(res.is_err()) return Err(res.unwrap_err());
    reg = payload.data;

    return Ok();
}
#include "SC8721.hpp"

using namespace ymd::drivers;

#define SC8721_DEBUG

#ifdef SC8721_DEBUG
#undef SC8721_DEBUG
#define SC8721_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define SC8721_PANIC(...) PANIC(__VA_ARGS__)
#define SC8721_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define SC8721_DEBUG(...)
#define SC8721_PANIC(...)  PANIC()
#define SC8721_ASSERT(cond, ...) ASSERT(cond)
#endif

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

#define RETURN_ON_ERR(x) ({\
    if(const auto __res_return_on_err = (x); __res_return_on_err.is_err()){\
        return CHECK_RES(__res_return_on_err);\
    }\
});\


using namespace ymd;
using namespace ymd::drivers;

using Error = SC8721::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> SC8721::update(){
    TODO();
}

IResult<> SC8721::validate(){
    TODO();
    return Ok();
}

IResult<> SC8721::reset(){
    TODO();
}


IResult<> SC8721::set_target_voltage(const real_t volt){
    uint16_t data = int(volt * 50);

    auto vout_set_msb_reg_copy = RegCopy(regs_.vout_set_msb_reg);
    auto vout_set_lsb_reg_copy = RegCopy(regs_.vout_set_lsb_reg);
    vout_set_msb_reg_copy.vout_set_msb = data >> 2;
    vout_set_lsb_reg_copy.vout_set_lsb = data & 0b11;

    if(const auto res = write_reg(vout_set_msb_reg_copy);
        res.is_err()) return res;
    if(const auto res = write_reg(vout_set_lsb_reg_copy);
        res.is_err()) return res;
    return Ok();
}

IResult<> SC8721::enable_external_fb(const Enable en){
    auto reg = RegCopy(regs_.vout_set_lsb_reg);

    reg.fb_sel = en == EN;
    if(en == EN){
        reg.fb_on = false;
    }else{
        reg.fb_on = true;
        reg.fb_dir = false;
    }

    return write_reg(reg);
}

IResult<> SC8721::set_dead_zone(const DeadZone dz){
    auto reg = RegCopy(regs_.sys_set_reg);
    reg.ext_dt = bool(dz);
    return write_reg(reg);
}


IResult<> SC8721::set_switch_freq(const SwitchFreq freq){
    auto reg = RegCopy(regs_.freq_set_reg);
    reg.freq_set = uint8_t(freq);
    return write_reg(reg);
}

IResult<SC8721::Status> SC8721::get_status(){
    if(const auto res = read_reg(regs_.status1_reg);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(regs_.status2_reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(Status{
        .short_circuit =        regs_.status1_reg.vout_short,
        .vout_vin_h =           regs_.status1_reg.vout_vin_h,
        .thermal_shutdown =     regs_.status1_reg.thd,
        .ocp =                  regs_.status1_reg.ocp,
        .vin_ovp =              regs_.status2_reg.vinovp,
        .on_cv =                regs_.status2_reg.vinreg_flag,
        .on_cc =                regs_.status2_reg.ibus_flag
    });
}

IResult<> SC8721::set_slope_comp(const SlopComp sc){
    auto reg = RegCopy(regs_.slope_comp_reg);
    reg.slop_comp = uint8_t(sc);

    return write_reg(reg);
}
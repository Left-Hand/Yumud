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

#define WRITE_REG(reg) write_reg(reg.address, reg);
#define READ_REG(reg) read_reg(reg.address, reg);

void SC8721::update(){

}

bool SC8721::validate(){
    return true;
}

void SC8721::reset(){

}


void SC8721::setTargetVoltage(const real_t volt){
    uint16_t data = int(volt * 50);

    vout_set_msb_reg.vout_set_msb = data >> 2;
    vout_set_lsb_reg.vout_set_lsb = data & 0b11;

    WRITE_REG(vout_set_msb_reg)
    WRITE_REG(vout_set_lsb_reg)
}

void SC8721::enableExternalFb(const bool en){
    auto & reg = vout_set_lsb_reg;

    reg.fb_sel = en;
    if(en){
        reg.fb_on = false;
    }else{
        reg.fb_on = true;
        reg.fb_dir = false;
    }

    WRITE_REG(reg)
}

void SC8721::setDeadZone(const DeadZone dz){
    auto & reg = sys_set_reg;
    reg.ext_dt = bool(dz);
    WRITE_REG(reg);
}


void SC8721::setSwitchFreq(const SwitchFreq freq){
    auto & reg = freq_set_reg;
    reg.freq_set = uint8_t(freq);
    WRITE_REG(reg);
}

SC8721::Status SC8721::getStatus(){
    READ_REG(status1_reg);
    READ_REG(status2_reg);

    Status status;
    status.short_circuit = status1_reg.vout_short;
    status.vout_vin_h = status1_reg.vout_vin_h;
    status.thermal_shutdown = status1_reg.thd;
    status.ocp = status1_reg.ocp;
    status.vin_ovp = status2_reg.vinovp;
    status.on_cv = status2_reg.vinreg_flag;
    status.on_cc = status2_reg.ibus_flag;

    return status;
}

void SC8721::setSlopeComp(const SlopComp sc){
    slope_comp_reg.slop_comp = uint8_t(sc);

    WRITE_REG(slope_comp_reg)
}
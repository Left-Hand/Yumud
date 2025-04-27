#include "sgm58031.hpp"
#include "core/math/realmath.hpp"

using namespace ymd::drivers;

#ifndef SGM58031_DEBUG
#define SGM58031_DEBUG(...) DEBUG_LOG(...)
#endif


void SGM58031::init(){
    read_reg(RegAddress::Config, configReg);
    read_reg(RegAddress::LowThr, lowThrReg);
    read_reg(RegAddress::HighThr, highThrReg);
    read_reg(RegAddress::Trim, trimReg);
    read_reg(RegAddress::DeviceID, deviceIdReg);
}

void SGM58031::set_datarate(const DataRate _dr){
    uint8_t dr = (uint8_t)_dr;
    configReg.dataRate = dr & 0b111;
    config1Reg.drSel = dr >> 3;
    write_reg(RegAddress::Config, configReg);
    write_reg(RegAddress::Config1, config1Reg);
}

void SGM58031::set_fs(const FS fs){
    fullScale = fs.to_real();
    configReg.pga = fs.as_u8();
    write_reg(RegAddress::Config, configReg);
}

void SGM58031::set_fs(const real_t _fs, const real_t _vref){
    real_t ratio = abs(_fs) / _vref;
    PGA pga;
    if(ratio >= 3){
        pga = PGA::RT2_3;
    }else if(ratio >= 2){
        pga = PGA::RT1;
    }else if(ratio >= 1){
        pga = PGA::RT2;
    }else if(ratio >= real_t(0.5)){
        pga = PGA::RT4;
    }else if(ratio >= real_t(0.25)){
        pga = PGA::RT8;
    }else{
        pga = PGA::RT16;
    }
    configReg.pga = (uint8_t)pga;
    write_reg(RegAddress::Config, configReg);
}


void SGM58031::set_trim(const real_t _trim){
    real_t trim = _trim * real_t(4.0f / 3.0f);
    real_t offset = trim - real_t(1.30225f);
    trimReg.gn = int(offset * 0b01111111010);
    write_reg(RegAddress::Trim, trimReg);
}

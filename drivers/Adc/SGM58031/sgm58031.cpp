#include "sgm58031.hpp"

using namespace ymd::drivers;

void SGM58031::init(){
    readReg(RegAddress::Config, configReg);
    readReg(RegAddress::LowThr, lowThrReg);
    readReg(RegAddress::HighThr, highThrReg);
    readReg(RegAddress::Trim, trimReg);
    readReg(RegAddress::DeviceID, deviceIdReg);
}

void SGM58031::setDataRate(const DataRate _dr){
    uint8_t dr = (uint8_t)_dr;
    configReg.dataRate = dr & 0b111;
    config1Reg.drSel = dr >> 3;
    writeReg(RegAddress::Config, configReg);
    writeReg(RegAddress::Config1, config1Reg);
}

void SGM58031::setFS(const FS fs){
    configReg.pga = (uint8_t)fs;
    switch(fs){
        case FS::FS0_256:
            fullScale = real_t(0.256);
            break;
        case FS::FS0_512:
            fullScale = real_t(0.512f);
            break;
        case FS::FS1_024:
            fullScale = real_t(1.024f);
            break;
        case FS::FS2_048:
            fullScale = real_t(2.048f);
            break;
        case FS::FS4_096:
            fullScale = real_t(4.096f);
            break;
        case FS::FS6_144:
            fullScale = real_t(6.144f);
            break;
        default:
            break;
    }
    writeReg(RegAddress::Config, configReg);
}

void SGM58031::setFS(const real_t _fs, const real_t _vref){
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
    writeReg(RegAddress::Config, configReg);
}


void SGM58031::setTrim(const real_t _trim){
    real_t trim = _trim * real_t(4.0f / 3.0f);
    real_t offset = trim - real_t(1.30225f);
    trimReg.gn = (int)(offset * 0b01111111010);
    writeReg(RegAddress::Trim, trimReg);
}

#include "sgm58031.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifndef SGM58031_DEBUG
#define SGM58031_DEBUG(...) DEBUG_LOG(...)
#endif

using Error = SGM58031::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> SGM58031::init(){
    if(const auto res = read_reg(config_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(low_thr_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(high_thr_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(trim_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(device_id_reg);
        res.is_err()) return res;
    return Ok();
}

IResult<> SGM58031::set_datarate(const DataRate _dr){
    uint8_t dr = (uint8_t)_dr;
    {
        auto reg = RegCopy(config_reg);
        reg.dataRate = dr & 0b111;
        if(const auto res = write_reg(reg);
            res.is_err()) return res;
    }

    {
        auto reg = RegCopy(config1_reg);
        reg.drSel = dr >> 3;
        return write_reg(reg);
    }
}

IResult<> SGM58031::set_fs(const FS fs){
    fullScale = fs.to_real();

    auto reg = RegCopy(config_reg);
    reg.pga = fs.as_u8();
    return write_reg(reg);
}

IResult<> SGM58031::set_fs(const real_t _fs, const real_t _vref){
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

    auto reg = RegCopy(config_reg);
    reg.pga = (uint8_t)pga;
    return write_reg(reg);
}


IResult<> SGM58031::set_trim(const real_t _trim){
    real_t trim = _trim * real_t(4.0f / 3.0f);
    real_t offset = trim - real_t(1.30225f);
    auto reg = RegCopy(trim_reg);
    reg.gn = int(offset * 0b01111111010);
    return write_reg(reg);
}

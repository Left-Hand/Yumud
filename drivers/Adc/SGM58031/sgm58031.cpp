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
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = read_reg(regs_.config_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(regs_.low_thr_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(regs_.high_thr_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(regs_.trim_reg);
        res.is_err()) return res;
    return Ok();
}

IResult<> SGM58031::validate(){
    auto & reg = regs_.device_id_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return res;
    if(reg.as_u16() != reg.KEY) 
        return Err(Error::WrongChipId);
    return Ok();
}
IResult<> SGM58031::set_datarate(const DataRate dr){
    {
        auto reg = RegCopy(regs_.config_reg);
        reg.data_rate = uint8_t(std::bit_cast<uint8_t>(dr) & 0b111);
        if(const auto res = write_reg(reg);
            res.is_err()) return res;
    }

    {
        auto reg = RegCopy(regs_.config1_reg);
        reg.dr_sel = std::bit_cast<uint8_t>(dr) >> 3;
        return write_reg(reg);
    }
}

IResult<> SGM58031::set_fs(const FS fs){
    full_scale_ = fs.to_real();

    auto reg = RegCopy(regs_.config_reg);
    reg.pga = fs.as_pga();
    return write_reg(reg);
}



IResult<> SGM58031::set_fs(const q16 _fs, const q16 _vref){
    q16 ratio = abs(_fs) / _vref;
    auto reg = RegCopy(regs_.config_reg);
    reg.pga = ratio2pga(ratio);
    return write_reg(reg);
}


IResult<> SGM58031::set_trim(const q16 _trim){
    q16 trim = _trim * q16(4.0f / 3.0f);
    q16 offset = trim - q16(1.30225f);
    auto reg = RegCopy(regs_.trim_reg);
    reg.gn = int(offset * 0b01111111010);
    return write_reg(reg);
}


IResult<bool> SGM58031::is_idle(){
    auto & reg = regs_.config_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.os));
}

IResult<> SGM58031::start_conv(){
    {
        auto reg = RegCopy(regs_.config1_reg);
        reg.pd = true;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regs_.config_reg);
        reg.os = true;
        return write_reg(reg);
    }
}

IResult<int16_t> SGM58031::get_conv_data(){
    auto & reg = regs_.conv_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.as_val());
}

IResult<q16> SGM58031::get_conv_voltage(){
    const auto res = get_conv_data();
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok((res.unwrap() * full_scale_) >> 15);
}


IResult<> SGM58031::enable_cont_mode(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.mode = (en == EN);
    return write_reg(reg);
}

IResult<> SGM58031::set_mux(const MUX _mux){
    auto reg = RegCopy(regs_.config_reg);
    reg.mux = _mux;
    return write_reg(reg);
}


IResult<> SGM58031::enable_ch3_as_ref(const Enable en){
    auto reg = RegCopy(regs_.config1_reg);
    reg.ext_ref = en == EN;
    return write_reg(reg);
}
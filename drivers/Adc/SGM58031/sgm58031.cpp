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
    if(const auto res = read_reg(config_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(low_thr_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(high_thr_reg);
        res.is_err()) return res;
    if(const auto res = read_reg(trim_reg);
        res.is_err()) return res;
    return Ok();
}

IResult<> SGM58031::validate(){
    auto & reg = device_id_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return res;
    if(reg.as_u16() != reg.KEY) 
        return Err(Error::WrongChipId);
    return Ok();
}
IResult<> SGM58031::set_datarate(const DataRate dr){
    {
        auto reg = RegCopy(config_reg);
        reg.dataRate = uint8_t(std::bit_cast<uint8_t>(dr) & 0b111);
        if(const auto res = write_reg(reg);
            res.is_err()) return res;
    }

    {
        auto reg = RegCopy(config1_reg);
        reg.drSel = std::bit_cast<uint8_t>(dr) >> 3;
        return write_reg(reg);
    }
}

IResult<> SGM58031::set_fs(const FS fs){
    full_scale_ = fs.to_real();

    auto reg = RegCopy(config_reg);
    reg.pga = fs.as_pga();
    return write_reg(reg);
}



IResult<> SGM58031::set_fs(const real_t _fs, const real_t _vref){
    real_t ratio = abs(_fs) / _vref;
    auto reg = RegCopy(config_reg);
    reg.pga = ratio2pga(ratio);
    return write_reg(reg);
}


IResult<> SGM58031::set_trim(const real_t _trim){
    real_t trim = _trim * real_t(4.0f / 3.0f);
    real_t offset = trim - real_t(1.30225f);
    auto reg = RegCopy(trim_reg);
    reg.gn = int(offset * 0b01111111010);
    return write_reg(reg);
}


IResult<bool> SGM58031::is_idle(){
    if(const auto res = read_reg(config_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(config_reg.os));
}

IResult<> SGM58031::start_conv(){
    {
        auto reg = RegCopy(config1_reg);
        reg.pd = true;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(config_reg);
        reg.os = true;
        return write_reg(reg);
    }
}

IResult<int16_t> SGM58031::get_conv_data(){
    if(const auto res = read_reg(conv_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(conv_reg.as_val());
}

IResult<real_t> SGM58031::get_conv_voltage(){
    const auto res = get_conv_data();
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok((res.unwrap() * full_scale_) >> 15);
}


IResult<> SGM58031::enable_cont_mode(const Enable en){
    auto reg = RegCopy(config_reg);
    reg.mode = (en == EN);
    return write_reg(reg);
}

IResult<> SGM58031::set_datarate(const DataRate _dr);

IResult<> SGM58031::set_mux(const MUX _mux){
    auto reg = RegCopy(config_reg);
    reg.mux = _mux;
    return write_reg(reg);
}


IResult<> SGM58031::enable_ch3_as_ref(const Enable en){
    auto reg = RegCopy(config1_reg);
    reg.extRef = en == EN;
    return write_reg(reg);
}
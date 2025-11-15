#include "sgm58031.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifndef SGM58031_DEBUG
#define SGM58031_DEBUG(...) DEBUG_LOG(...)
#endif

using Self = SGM58031;

using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

[[nodiscard]] static constexpr Self::PGA ratio2pga(const uq16 ratio){
    using PGA = Self::PGA;
    if(ratio >= 3){
        return PGA::_2_3;
    }else if(ratio >= 2){
        return PGA::_1;
    }else if(ratio >= 1){
        return PGA::_2;
    }else if(ratio >= uq16(0.5)){
        return PGA::_4;
    }else if(ratio >= uq16(0.25)){
        return PGA::_8;
    }else{
        return PGA::_16;
    }
}


IResult<> Self::init(){
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

IResult<> Self::validate(){
    auto & reg = regs_.device_id_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return res;
    if(reg.as_u16() != reg.KEY) 
        return Err(Error::WrongChipId);
    return Ok();
}
IResult<> Self::set_datarate(const DataRate dr){
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

IResult<> Self::set_fs(const FS fs){
    full_scale_ = fs.to_real();

    auto reg = RegCopy(regs_.config_reg);
    reg.pga = fs.as_pga();
    return write_reg(reg);
}



IResult<> Self::set_fs(const uq16 _fs, const uq16 _vref){
    const auto ratio = abs(_fs) / _vref;
    auto reg = RegCopy(regs_.config_reg);
    reg.pga = ratio2pga(ratio);
    return write_reg(reg);
}


IResult<> Self::set_trim(const iq16 _trim){
    iq16 trim = _trim * iq16(4.0f / 3.0f);
    iq16 offset = trim - iq16(1.30225f);
    auto reg = RegCopy(regs_.trim_reg);
    reg.gn = int(offset * 0b01111111010);
    return write_reg(reg);
}


IResult<bool> Self::is_idle(){
    auto & reg = regs_.config_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.os));
}

IResult<> Self::start_conv(){
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

IResult<int16_t> Self::get_conv_data(){
    auto & reg = regs_.conv_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.as_bits());
}

IResult<iq16> Self::get_conv_voltage(){
    const auto res = get_conv_data();
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok((res.unwrap() * full_scale_) >> 15);
}


IResult<> Self::enable_cont_mode(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.mode = (en == EN);
    return write_reg(reg);
}

IResult<> Self::set_mux(const MUX _mux){
    auto reg = RegCopy(regs_.config_reg);
    reg.mux = _mux;
    return write_reg(reg);
}


IResult<> Self::enable_ch3_as_mut_bits(const Enable en){
    auto reg = RegCopy(regs_.config1_reg);
    reg.ext_ref = en == EN;
    return write_reg(reg);
}
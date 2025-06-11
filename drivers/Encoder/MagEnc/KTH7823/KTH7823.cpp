#include "KTH7823.hpp"
#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = KTH7823::Error;

template<typename T = void>
using IResult =  Result<T, Error>;


IResult<uint16_t> KTH7823_Phy::direct_read(){
    uint16_t rx;
    if(const auto res = transceive_u16(rx, 0);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(rx);
}

IResult<uint8_t> KTH7823_Phy::read_reg(const uint8_t addr){
    const uint16_t tx = (uint16_t((addr & 0b00'111111) | 0b01'000000) << 8);
    uint16_t rx;
    const auto res = transceive_u16(rx, tx);
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(rx >> 8);
}

IResult<> KTH7823_Phy::burn_reg(const uint8_t addr, const uint8_t data){
    const uint16_t tx = (uint16_t((addr & 0b00'111111) | 0b10'000000) << 8) | data;
    uint16_t rx;
    if(const auto res = transceive_u16(rx, tx);
        res.is_err()) return Err(res.unwrap_err());
    
    if((rx >> 8) != data) 
        return Err(Error::RegProgramFailed);
    
    if((rx & 0xff) != 0) 
        return Err(Error::RegProgramResponseFormatInvalid);
    return Ok();
}

IResult<> KTH7823_Phy::disable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0010);
}

IResult<> KTH7823_Phy::enable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0000);
}

IResult<> KTH7823_Phy::transceive_u16(uint16_t & rx, const uint16_t tx){
    uint16_t dummy = 0;
    if(const auto res = spi_drv_.transceive_single<uint16_t>(dummy, tx);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_single<uint16_t>(rx, dummy);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> KTH7823::update(){
    
    const auto data = ({
        const auto res = phy_.direct_read(); 
        if(res.is_err()) 
            return Err(res.unwrap_err());
        res.unwrap();
    });

    lap_position_ = u16_to_uni(data);

    return Ok();
}

IResult<> KTH7823::validate(){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_zero_position(const real_t position){
    const auto raw16 = uni_to_u16(position);

    auto reg_low = RegCopy(zero_low_reg);
    reg_low.data = raw16 & 0xff;

    auto reg_high = RegCopy(zero_high_reg);
    reg_high.data = raw16 >> 8;

    // return Ok();
    return phy_.burn_reg(reg_low)
    | phy_.burn_reg(reg_high);
}

IResult<> KTH7823::set_trim_x(const real_t k){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_trim_y(const real_t k){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_trim(const real_t am, const real_t e){
    real_t k = std::tan(am + e) / std::tan(am);
    if(k > real_t(1)) return set_trim_x(k);
    else return set_trim_y(k);
}

IResult<> KTH7823::set_mag_threshold(const MagThreshold low, const MagThreshold high){
    auto reg = RegCopy(mag_alert_reg);
    reg.mag_high = high;
    reg.mag_low = low;

    return phy_.burn_reg(reg);
    // return Ok();
}

IResult<> KTH7823::set_direction(const bool direction){
    TODO();
    return Ok();
}

IResult<MagStatus> KTH7823::get_mag_status(){
    TODO();
    return Ok(MagStatus::Proper());
}

IResult<> KTH7823::set_zparameters(const Width width, const Phase phase){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_pulse_per_turn(const uint16_t ppt){
    TODO();
    return Ok();
}
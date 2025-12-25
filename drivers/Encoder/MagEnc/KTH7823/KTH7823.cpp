#include "KTH7823.hpp"
#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers::kth7823;

IResult<uint16_t> Phy::direct_read(){
    uint16_t rx;
    if(const auto res = transceive_u16(rx, 0);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(rx);
}

IResult<uint8_t> Phy::read_reg(const uint8_t addr){
    const uint16_t tx = (uint16_t((addr & 0b00'111111) | 0b01'000000) << 8);
    uint16_t rx;
    const auto res = transceive_u16(rx, tx);
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(rx >> 8);
}

IResult<> Phy::burn_reg(const uint8_t addr, const uint8_t data){
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

IResult<> Phy::disable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0010);
}

IResult<> Phy::enable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0000);
}

IResult<> Phy::transceive_u16(uint16_t & rx, const uint16_t tx){
    uint16_t dummy = 0;
    if(const auto res = spi_drv_.transceive_single<uint16_t>(dummy, tx);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_single<uint16_t>(rx, dummy);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> KTH7823::update(){
    
    const auto data = ({
        const auto res = transport_.direct_read(); 
        if(res.is_err()) 
            return Err(res.unwrap_err());
        res.unwrap();
    });


    lap_turns_ = uq16::from_bits(data);

    return Ok();
}

IResult<> KTH7823::validate(){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_zero_angle(const Angular<uq32> angle){
    const auto raw16 = (angle.to_turns().to_bits() >> 16);

    auto reg_low = RegCopy(regset_.zero_low_reg);
    reg_low.data = raw16 & 0xff;

    auto reg_high = RegCopy(regset_.zero_high_reg);
    reg_high.data = raw16 >> 8;

    // return Ok();
    if(const auto res = transport_.burn_reg(reg_low); 
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = transport_.burn_reg(reg_high); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> KTH7823::set_trim_x(const iq16 k){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_trim_y(const iq16 k){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_trim(const iq16 am, const iq16 e){
    iq16 k = math::tan(am + e) / math::tan(am);
    if(k > iq16(1)) return set_trim_x(k);
    else return set_trim_y(k);
}

IResult<> KTH7823::set_mag_threshold(const MagThreshold low, const MagThreshold high){
    auto reg = RegCopy(regset_.mag_alert_reg);
    reg.mag_high = high;
    reg.mag_low = low;

    return transport_.burn_reg(reg);
    // return Ok();
}

IResult<> KTH7823::set_direction(const RotateDirection direction){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_zero_parameters(const ZeroPulseWidth width, const ZeroPulsePhase phase){
    TODO();
    return Ok();
}

IResult<> KTH7823::set_pulse_per_turn(const uint16_t ppt){
    TODO();
    return Ok();
}
#include "KTH7823.hpp"
#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers::kth7823;

IResult<uint16_t> Transport::direct_read(){
    uint16_t rx_bits;
    if(const auto res = transceive_u16(rx_bits, 0);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(rx_bits);
}

IResult<uint8_t> Transport::read_reg(const uint8_t addr){
    const uint16_t tx_bits = static_cast<uint16_t>(static_cast<uint16_t>((addr & 0b00'111111) | 0b01'000000) << 8);
    uint16_t rx_bits;
    const auto res = transceive_u16(rx_bits, tx_bits);
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(rx_bits >> 8);
}

IResult<> Transport::burn_reg(const uint8_t addr, const uint8_t data){
    // SPI 烧写寄存器操作由两个 16 位的帧组成。第一个帧是写请求
    // 帧，其中包含一个 2 位的写命令（10），后跟一个 6 位的寄存器
    // 地址和一个 8 位的数值。写命令指示芯片执行写操作，寄存器地
    // 址用于指定要写入的寄存器，而数值则是要写入的数据。数据传
    // 输从最高位（MSB）开始。
    // 第二个帧是返回的确认帧，其中包含了新写入寄存器的值。帧格
    // 式为 XXXX − XXXX − 0000 − 0000。这个确认帧是芯片返回的
    // 响应，用于确认数据已成功写入寄存器。
    // 在进行 SPI 烧写寄存器操作时，需要特别注意在第一个帧和第二
    // 个帧之间等待至少 20 毫秒的时间。这段等待时间用于确保写入
    // 的数据能正确存储到芯片的非易失性存储器中。如果在写入请求
    // 后没有等待足够的时间，可能会读取到寄存器先前的值。因此，
    // 执行写入寄存器操作时务必遵守这个等待时间。
    
    {
        const uint16_t tx_bits = static_cast<uint16_t>(static_cast<uint16_t>((addr & 0b00'111111) | 0b10'000000) << 8) | data;
        if(const auto res = spi_drv_.write_single<uint16_t>(tx_bits, CONT);
            res.is_err()) return Err(res.unwrap_err());
    }

    //TODO use async
    clock::delay(20ms);


    uint16_t rx_bits;
    if(const auto res = spi_drv_.read_single<uint16_t>(rx_bits);
        res.is_err()) return Err(res.unwrap_err());
    
    if((rx_bits >> 8) != data) 
        return Err(Error::RegProgramFailed);
    
    if((rx_bits & 0xff) != 0) 
        return Err(Error::RegProgramResponseFormatInvalid);
    return Ok();
}

IResult<> Transport::disable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0010);
}

IResult<> Transport::enable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0000);
}

IResult<> Transport::transceive_u16(uint16_t & rx, const uint16_t tx){
    if(const auto res = spi_drv_.write_single<uint16_t>(tx, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_single<uint16_t>(rx, tx);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> KTH7823::update(){
    
    const auto bits = ({
        const auto res = transport_.direct_read(); 
        if(res.is_err()) 
            return Err(res.unwrap_err());
        res.unwrap();
    });


    lap_turns_ = lap_turns_.from_bits(static_cast<uint32_t>(bits) << 16);

    return Ok();
}

IResult<> KTH7823::validate(){
    TODO();
    return Ok();
}

IResult<> KTH7823::burn_zero_angle(const Angular<uq32> angle){
    const uint16_t b16 = static_cast<uint16_t>(angle.to_turns().to_bits() >> 16);

    auto reg_low = RegCopy(regset_.zero_low_reg);
    reg_low.bits = static_cast<uint8_t>(b16 & 0xff);

    auto reg_high = RegCopy(regset_.zero_high_reg);
    reg_high.bits = static_cast<uint8_t>(b16 >> 8);

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
    if(k > 1) return set_trim_x(k);
    else return set_trim_y(k);
}

IResult<> KTH7823::set_mag_threshold(const MagThreshold low, const MagThreshold high){
    auto reg = RegCopy(regset_.mag_alert_reg);
    reg.mag_low = low;
    reg.mag_high = high;

    return transport_.burn_reg(reg);
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
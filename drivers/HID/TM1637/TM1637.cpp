#include "TM1637.hpp"
#include "core/debug/debug.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = TM1637_Phy::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> TM1637_Phy::write_byte(const uint8_t data){
    sda_gpio_.outod();
    for(uint8_t mask = 0x01; mask; mask <<= 1){
        scl_gpio_.clr();
        sda_gpio_.write(BoolLevel::from(mask & data));
        clock::delay(3us);
        scl_gpio_.set();
        clock::delay(3us);
    }

    return wait_ack();
}


IResult<> TM1637_Phy::wait_ack(){
    sda_gpio_.inpu();
    scl_gpio_.clr();
    clock::delay(5us);
    scl_gpio_.set();

    bool ovt = false;
    const auto m = clock::micros();
    while(sda_gpio_.read() == HIGH){
        if(clock::micros() - m >= Microseconds(2)){
            ovt = true;
            break;
        }
        clock::delay(1us);
    }

    scl_gpio_.set();
    clock::delay(2us);
    scl_gpio_.clr();
    
    if(ovt){
        return Err(hal::HalResult::WritePayloadAckTimeout);
    }else{
        return Ok();
    }
}

IResult<> TM1637_Phy::read_byte(uint8_t & data){
    uint8_t ret = 0;

    for(uint8_t i = 0; i < 8; i++){
        scl_gpio_.clr();
        ret = ret >> 1;
        clock::delay(30us);
        scl_gpio_.set();
        ret = ret | ((sda_gpio_.read() == HIGH) ? 0x80 : 0x00);
        clock::delay(30us);
    }

    data = ret;
    return Ok();
}

IResult<> TM1637_Phy::iic_start(const uint8_t data){
    scl_gpio_.outod(HIGH);
    sda_gpio_.outod(HIGH);
    clock::delay(2us);
    sda_gpio_.clr();
    if(const auto res = write_byte(data);
        res.is_err()) return Err(hal::HalResult::SlaveAddrAckTimeout);
    return Ok();
}

IResult<> TM1637_Phy::iic_stop(){
    scl_gpio_.clr();
    sda_gpio_.outod();
    clock::delay(2us);
    sda_gpio_.clr();
    clock::delay(2us);
    scl_gpio_.set();
    clock::delay(2us);
    sda_gpio_.set();

    return Ok();
}

IResult<> TM1637_Phy::write_sram(const std::span<const uint8_t> pbuf){
    if(pbuf.size() > TM1637::CGRAM_MAX_LEN) return Err(Error::DisplayLengthTooLong);

    const auto command2 = AddressCommand{
        .addr = CGRAM_BEGIN_ADDR
    }.as_u8();

    
    if(const auto res = iic_start(command2); res.is_err()) return res;
    for(size_t i = 0; i < pbuf.size(); i++){
        if(const auto res = write_byte(pbuf[i]);
            res.is_err()) return res;
    }
    return iic_stop();
}


IResult<> TM1637_Phy::set_display(const DisplayCommand command){
    const auto res = 
        iic_start(command.as_u8()) 
        | iic_stop()
    ;

    return res;
}
Result<uint8_t, Error> TM1637_Phy::read_key(){
    const auto command1 = DataCommand{
        .read_key = true,//write
        .addr_inc_disen = true
    };

    uint8_t data;

    const auto res = 
        iic_start(command1.as_u8())
        | read_byte(data)
        | iic_stop()
    ;

    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(data);
}

IResult<> TM1637_Phy::set_data_mode(const DataCommand command1){

    const auto res = 
        iic_start(command1.as_u8())
        | iic_stop()
    ;

    return res;
}

IResult<> TM1637::switch_to_display(){
    if(is_on_display_else_readkey_ == true) return Ok();
    is_on_display_else_readkey_ = true;

    return phy_.set_data_mode(
        {
            .read_key = false,//write
            .addr_inc_disen = false
        }
    );
}
IResult<>  TM1637::switch_to_readkey(){
    if(is_on_display_else_readkey_ == false) return Ok();
    is_on_display_else_readkey_ = false;

    return phy_.set_data_mode(
        {
            .read_key = true,
            .addr_inc_disen = false
        }
    );
}

IResult<> TM1637::set_display_duty(const real_t duty){
    if(duty > 1) return Err(Error::DutyGreatThanOne);
    if(duty < 0) return Err(Error::DutyLessThanZero);

    const auto pw_opt = PulseWidth::from_duty(duty);
    if(pw_opt.is_some()){
        disp_cmd_.pulse_width = pw_opt.unwrap().kind();
        disp_cmd_.display_en = true;
    }else{
        disp_cmd_.display_en = false;
    }

    return Ok();
}


static constexpr Result<Option<KeyPlacement>, Error> map_raw_to_keyplace(const uint8_t raw){
    if(raw == 0xff){
        return Ok(None);
    }
    const auto col = [&] -> Option<uint8_t>{
        const uint8_t key = raw & 0x0f;
        switch(key){
            case 0b11101: return Some<uint8_t>(0);
            case 0b01001: return Some<uint8_t>(1);
            case 0b10101: return Some<uint8_t>(2);
            case 0b00101: return Some<uint8_t>(3);
            case 0b11111: return Some<uint8_t>(0);
            case 0b01011: return Some<uint8_t>(1);
            case 0b10111: return Some<uint8_t>(2);
            case 0b00111: return Some<uint8_t>(3);
            case 0b11010: return Some<uint8_t>(0);
            case 0b01010: return Some<uint8_t>(1);
            case 0b10010: return Some<uint8_t>(2);
            case 0b00010: return Some<uint8_t>(3);
            case 0b11110: return Some<uint8_t>(0);
            case 0b01110: return Some<uint8_t>(1);
            case 0b10110: return Some<uint8_t>(2);
            case 0b00110: return Some<uint8_t>(3);
            default: return None;
        }
    }();

    const auto row = [&] -> Option<uint8_t>{
        const uint8_t key = raw >> 4;
        switch(key){
            case 0b11101: return Some<uint8_t>(0);
            case 0b01001: return Some<uint8_t>(1);
            case 0b10101: return Some<uint8_t>(2);
            case 0b00101: return Some<uint8_t>(3);
            case 0b11111: return Some<uint8_t>(0);
            case 0b01011: return Some<uint8_t>(1);
            case 0b10111: return Some<uint8_t>(2);
            case 0b00111: return Some<uint8_t>(3);
            case 0b11010: return Some<uint8_t>(0);
            case 0b01010: return Some<uint8_t>(1);
            case 0b10010: return Some<uint8_t>(2);
            case 0b00010: return Some<uint8_t>(3);
            case 0b11110: return Some<uint8_t>(0);
            case 0b01110: return Some<uint8_t>(1);
            case 0b10110: return Some<uint8_t>(2);
            case 0b00110: return Some<uint8_t>(3);
            default: return None;
        }
    }();

    return Ok(Some(
        KeyPlacement(col, row)
    ));
}

Result<Option<KeyPlacement>, Error> TM1637::read_key(){
    const auto res = phy_.read_key();
    if(res.is_err()) return Err(res.unwrap_err());
    return map_raw_to_keyplace(res.unwrap());
}

IResult<> TM1637::flush(){
    //TODO 更换为数据利用率更高的更新算法

    const bool changed = buf_.changed();
    if(changed){
        // const auto res = phy_.write_screen(buf_.to_span());
        const auto res = phy_.write_sram(buf_.to_span());
        if(res.is_err())return res;
        buf_.flush();
    }

    return phy_.set_display(DisplayCommand{
        .pulse_width = PulseWidth::_4_16,
        .display_en = true
    });
}

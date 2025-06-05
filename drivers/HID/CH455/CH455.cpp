#include "CH455.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = CH455::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> CH455::init() {
    phy_.init();

    return enable_seg7_mode(DISEN);
}

IResult<> CH455::enable_seg7_mode(Enable en){
    return phy_.write_u16(
            (en == EN ? CH455_7SEG_ON : CH455_8SEG_ON));
}

IResult<> CH455::set_brightness(const uint8_t br){
    if(br < 1) return Err(Error::BrightnessLessThanMin);
    if(br > 8) return Err(Error::BrightnessGreaterThanMax);

    if(br == 8){
        return phy_.write_u16( CH455_SYSON_8);
    }else{
        return phy_.write_u16( CH455_SYSON | (br<<4));
    }
}

IResult<> CH455::set_digit(const uint8_t digit, const uint8_t code){
    const auto cmd = dig2raw(digit, code);
    return phy_.write_u16(cmd);
}

IResult<> CH455::display_digit(const uint8_t digit, const uint8_t value){
    const auto cmd = dig2raw(digit, BCD_DEC_TABLE[value % BCD_DEC_TABLE.size()]);
    return phy_.write_u16(cmd);
}

IResult<KeyEvent> CH455::get_key() {
    const auto res =  phy_.read_u8();
    if(res.is_err()) return Err(res.unwrap_err());

    // CH455所提供的按键代码为8位，位7始终为0，位2始终为1，位1～位0是列扫描码，位5～
    // 位3是行扫描码，位6是状态码（键按下为1，键释放为0）。例如，连接DIG1与SEG4的键被按下，
    // 则按键代码是01100101B或者65H，键被释放后，按键代码通常是00100101B或者25H（也可能是其
    // 它值，但是肯定小于40H），其中，对应DIG1的列扫描码为01B，对应SEG4的行扫描码为100B。单
    // 片机可以在任何时候读取按键代码，但一般在CH455检测到有效按键而产生键盘中断时读取按键代
    // 码，此时按键代码的位6总是1，另外，如果需要了解按键何时释放，单片机可以通过查询方式定期
    // 读取按键代码，直到按键代码的位6为0。

    struct CH455KeyCode{
        uint8_t col:2;
        uint8_t bit2:1;
        uint8_t row:3;
        uint8_t is_pressed:1;
        uint8_t bit7:1;
    };

    static_assert(sizeof(CH455KeyCode) == 1);

    const CH455KeyCode code = std::bit_cast<CH455KeyCode>(res.unwrap());
    if(code.bit2 != 1) return Err(Error::ReadKeyBit2VerifyFailed);
    if(code.bit7 != 0) return Err(Error::ReadKeyBit7VerifyFailed);

    return Ok(KeyEvent{
        KeyPlacement{
            Some(uint8_t(code.col)),
            Some(uint8_t(code.row))
        }, 
        code.is_pressed ? KeyEvent::Type::Press : KeyEvent::Type::Release
    });
}
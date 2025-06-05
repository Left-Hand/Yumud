#include "HT16K33.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = HT16K33::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> HT16K33::commit_gcram_to_displayer(){
    // return phy_.write_burst(SetDataPtr{}, gc_ram_);
    TODO();
}

IResult<> HT16K33::set_display_bit(
    const size_t num, const bool value
){
    static constexpr size_t VALUE_WIDTH = 
        magic::type_to_bits_v<GcRam::value_type>;

    static constexpr size_t MAX_NUM = VALUE_WIDTH * GC_RAM_SIZE;

    if(num >= MAX_NUM) 
        return Err(Error::DisplayBitIndexOutOfRange);

    const auto i = num / VALUE_WIDTH;
    const auto mask = 1 << (num % VALUE_WIDTH);
    if(value) gc_ram_[i] |= mask;
    else gc_ram_[i] &= ~mask;

    return Ok();
}

IResult<> HT16K33::set_display_byte(
    const size_t index, const uint8_t value
){
    if(index >= GC_RAM_SIZE) 
        return Err(Error::DisplayByteIndexOutOfRange);

    gc_ram_[index] = value;
    return Ok();
}

IResult<> HT16K33::clear_display_content(){
    std::fill(gc_ram_.begin(), gc_ram_.end(), 0);
    return commit_gcram_to_displayer();
}

IResult<> HT16K33::write_command(const Command cmd){
    return phy_.write_command(cmd);
}

IResult<> HT16K33::enable_system_setup(const Enable en){
    return write_command(SystemSetupCommand{
        (en == EN)
    });
}

IResult<HT16K33::KeyData> HT16K33::get_key_data(){
    static constexpr auto KEY0_REGADDR = 0x40;
    KeyData ret;
    if(const auto res = phy_.read_burst(KEY0_REGADDR, ret.as_bytes());
        res.is_err()) return Err(res.unwrap_err());
    return Ok(std::move(ret));
}

IResult<> HT16K33::set_int_pin_func(const IntPinFunc func){
    return write_command(IntSet{
        func
    });
}
IResult<std::bitset<8>> HT16K33::get_int_status(){
    static constexpr auto INT_FLAG_REGADDR = 0x60;
    uint8_t ret = 0;
    if(const auto res = phy_.read_data(INT_FLAG_REGADDR, ret);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(std::bitset<8>(ret));
}

IResult<> HT16K33::init(){
    if(const auto res = validate(); 
        res.is_err()) return res;

    if(const auto res = enable_system_setup(EN);
        res.is_err()) return res;

    if(const auto res = set_int_pin_func(IntPinFunc::InterruptActiveLow);
        res.is_err()) return res;

    return Ok();
}

IResult<> HT16K33::validate(){
    return Ok();
}
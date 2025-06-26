#include "HT16K33.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = HT16K33::Error;

template<typename T = void>
using IResult = Result<T, Error>;


#define HT16K33_DEBUG_EN 0


#if HT16K33_DEBUG_EN
#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif

// IResult<> HT16K33::set_display_bit(
//     const size_t num, const bool value
// ){
//     static constexpr size_t VALUE_WIDTH = 
//         magic::type_to_bits_v<GcRam::value_type>;

//     static constexpr size_t MAX_NUM = VALUE_WIDTH * GC_RAM_SIZE;

//     if(num >= MAX_NUM) 
//         return Err(Error::DisplayBitIndexOutOfRange);

//     const auto i = num / VALUE_WIDTH;
//     const auto mask = 1 << (num % VALUE_WIDTH);
//     if(value) gc_ram_[i] |= mask;
//     else gc_ram_[i] &= ~mask;

//     return Ok();
// }

// IResult<> HT16K33::set_display_byte(
//     const size_t index, const uint8_t value
// ){
//     if(index >= GC_RAM_SIZE) 
//         return Err(Error::DisplayByteIndexOutOfRange);

//     gc_ram_[index] = value;
//     return Ok();
// }

IResult<bool> HT16K33::is_any_key_pressed(){
    if(phy_.has_int_io()){
        return Ok(phy_.is_int_io_active());
    }else{
        // DEBUG_PRINTLN("reading");
        return get_intreg_status()
            .map([](const BoolLevel st){
                return st == HIGH;
            });
    }
}

// IResult<> HT16K33::clear_display_content(){
//     std::fill(gc_ram_.begin(), gc_ram_.end(), 0);
//     return commit_gcram_to_displayer();
// }

IResult<> HT16K33::write_command(const Command cmd){
    return phy_.write_command(cmd);
}

IResult<> HT16K33::setup_system(const Enable en){
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
IResult<BoolLevel> HT16K33::get_intreg_status(){
    static constexpr auto INT_FLAG_REGADDR = 0x60;
    uint8_t ret = 0;
    if(const auto res = phy_.read_data(INT_FLAG_REGADDR, ret);
        res.is_err()) return Err(res.unwrap_err());
    switch(ret){
        case 0: return Ok(LOW);
        case 0xff: return Ok(HIGH);
        default: return Err(Error::UnknownInterruptCode);
    }
}

IResult<> HT16K33::init(const Config & cfg){
    if(const auto res = validate(); 
        res.is_err()) return CHECK_RES(res);

    if(const auto res = setup_system();
        res.is_err()) return CHECK_RES(res);

    if(const auto res = set_int_pin_func(cfg.int_pin_func);
        res.is_err()) return CHECK_RES(res);

    if(const auto res = clear_displayer();
        res.is_err()) return CHECK_RES(res);

    if(const auto res = set_pulse_duty(cfg.pulse_duty);
        res.is_err()) return CHECK_RES(res);

    if(const auto res = setup_displayer(cfg.blink_freq);
        res.is_err()) return CHECK_RES(res);

    return Ok();
}

IResult<> HT16K33::validate(){
    return Ok();
}

IResult<> HT16K33::update_displayer(
    const size_t offset, std::span<const uint8_t> pbuf){

    static constexpr uint8_t ADDR_BASE = 0x00;

    const auto start_addr = offset + ADDR_BASE;
    const auto stop_addr = start_addr + pbuf.size();

    if(stop_addr > GC_RAM_SIZE)
        return Err(Error::DisplayPayloadOversize);

    return phy_.write_burst(start_addr, pbuf);
}

IResult<> HT16K33::clear_displayer(){
    const std::array<uint8_t, 16> zeros = {0};
    return update_displayer(0, std::span(zeros));
}


IResult<> HT16K33::setup_displayer(const BlinkFreq freq, const Enable en){
    return phy_.write_command(Command(DisplaySetupCommand{en, freq}));
}

IResult<> HT16K33::set_pulse_duty(const PulseDuty duty){
    return phy_.write_command(Command(DimmingSet{duty}));
}
#include "ssd1306.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = SSD13XX::Error;
using Vector2u16 = Vector2<uint16_t>;

template<typename T = void>
using IResult = Result<T, Error>;
IResult<> SSD13XX::set_offset(const Vector2u16 offset){
    if(const auto res = phy_.write_command(0xD3); res.is_err()) return res; 
    if(const auto res = phy_.write_command(offset.y); res.is_err()) return res;
    return Ok();
}


IResult<> SSD13XX::set_flush_pos(const Vector2u16 pos){
    const auto [x, y] = pos + offset_;
    if(const auto res = phy_.write_command(0xb0 | size_t(y / 8));
        res.is_err()) return res;
    if(const auto res = phy_.write_command(((x & 0xf0 )>>4) |0x10);
        res.is_err()) return res;
    if(const auto res = phy_.write_command((x & 0x0f));
        res.is_err()) return res;
    return Ok();
}
IResult<> SSD13XX::init(){   
    // DEBUG_PRINTLN(std::showbase, std::hex, init_cmds_list_);
    if(const auto res = phy_.init() ; 
        res.is_err()) return res;
    if(const auto res = preinit_by_cmds(); 
        res.is_err()) return res;
    if(const auto res = enable_display(); 
        res.is_err()) return res;
    if(const auto res = set_offset(offset_); 
        res.is_err()) return res;
    if(const auto res = enable_flip_x(flip_x_en_); 
        res.is_err()) return res;
    if(const auto res = enable_flip_y(flip_y_en_); 
        res.is_err()) return res;
    return Ok();
}
IResult<> SSD13XX::enable_display(const Enable en){
    
    if(en == EN){
        if(const auto res = phy_.write_command(0x8D);
            res.is_err()) return res;
        if(const auto res = phy_.write_command(0x14);
            res.is_err()) return res;
        if(const auto res = phy_.write_command(0xAF);
            res.is_err()) return res;
    }else{
        if(const auto res = phy_.write_command(0x8D);
            res.is_err()) return res;
        if(const auto res = phy_.write_command(0x10);
            res.is_err()) return res;
        if(const auto res = phy_.write_command(0xAE);
            res.is_err()) return res;
    }

    return Ok();
}

IResult<> SSD13XX::update(){
    auto & frame = fetch_frame();
    for(size_t y = 0; y < size().y; y += 8){
        if(const auto res = set_flush_pos(Vector2u16(0, y)); 
            res.is_err()) return res;

        const auto line = std::span<const uint8_t>(
            &frame[(y / 8) * size().x], size().x);

        if(const auto res = phy_.write_burst(line);
            res.is_err()) return res;
    }
    return Ok();
}

IResult<> SSD13XX::preinit_by_cmds(){
    // DEBUG_PRINTLN(init_cmds_list_);
    for(const auto cmd:init_cmds_list_){
        if(const auto res = phy_.write_command(cmd);
            res.is_err()) return res;
    }

    return Ok();
}

// IResult<> SSD13XX::enable_inversion(const bool i){
//     if(const auto res = phy_.write_command(0xC8 - 8*uint8_t(i));
//         res.is_err()) return res;  //正常显示
//     return phy_.write_command(0xA1 - uint8_t(i));
// }

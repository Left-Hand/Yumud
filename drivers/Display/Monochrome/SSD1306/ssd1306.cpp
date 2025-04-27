#include "ssd1306.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd;

void SSD13XX::setpos_unsafe(const Vector2i & pos){
    // auto & frame = fetch_frame();
    // frame.setpos_unsafe(pos);
    TODO("not implemented");
}

Result<void, DisplayerError> SSD13XX::init(){   
    // DEBUG_PRINTLN(std::showbase, std::hex, cmds_);
    return interface_.init() | 
        preinit_by_cmds() |
        enable() |
        set_offset();
}

Result<void, DisplayerError> SSD13XX::update(){
    auto & frame = fetch_frame();
    // frame[0] = 0xff;
    // frame[22] = 0xff;
    for(int i = 0; i < size().y;i += 8){
        set_flush_pos(Vector2i(0, i));
        const auto line = std::span<const uint8_t>(&frame[(i / 8) * size_t(size().x)], size().x);
        // DEBUG_PRINTLN(size());
        const auto res = interface_.write_u8(&line[0], line.size());
        if(res.is_err()) return res;
    }
    return Ok();
}

Result<void, DisplayerError> SSD13XX::preinit_by_cmds(){
    // DEBUG_PRINTLN(cmds_);
    for(const auto cmd:cmds_){
        const auto res = interface_.write_command(cmd);
        if(res.is_err()) return res;
    }

    return Ok();
}

Result<void, DisplayerError> SSD13XX::turn_display(const bool i){
    if(const auto res = interface_.write_command(0xC8 - 8*uint8_t(i));
        res.is_err()) return res;  //正常显示
    return interface_.write_command(0xA1 - uint8_t(i));
}

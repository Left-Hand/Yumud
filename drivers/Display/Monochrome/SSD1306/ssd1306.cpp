#include "ssd1306.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd;

void SSD13XX::setpos_unsafe(const Vector2i & pos){
    // auto & frame = fetchFrame();
    // frame.setpos_unsafe(pos);
    DEBUG_PRINTLN("not implemented");
}

void SSD13XX::init(){   
    interface_.init(); 
    preinit_by_cmds();
    enable();
    set_offset();
}

void SSD13XX::update(){
    auto & frame = fetch_frame();
    for(int i = 0; i < size().y;i += 8){
        set_flush_pos(Vector2i(0, i));
        interface_.write_u8(&frame[(i / 8) * size_t(size().x)], size().x);
    }
}

void SSD13XX::preinit_by_cmds(){
    for(const auto cmd:cmds_){
        interface_.write_command(cmd);
    }
}
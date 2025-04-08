#include "st7789.hpp"
#include "types/range/range.hpp"
#include "core/debug/debug.hpp"


using namespace ymd::drivers;
using namespace ymd;

// #define ST7789_EXPRIMENTAL_SKIP


//判断刷新命令符必要性判断算法 以提高spi dma的吞吐率
bool ST7789::ST7789_ReflashAlgo::update(const Rect2_t<uint16_t> rect){
    const auto curr_pt_range = get_point_index(curr_area_);
    const auto desired_pt_range = get_point_index(rect);

    if(desired_pt_range.inside(curr_pt_range) and rect.inside(curr_area_)){
        last_point_ += desired_pt_range.length();
        return false;
    }else{
        curr_area_ = rect;
        last_point_ = desired_pt_range.from;
        return true;
    }
} 

void ST7789::init(){
    interface_.init();
    
    write_command(0x01);

    delayMicroseconds(50);
	write_command(0x11);
    delayMicroseconds(50);
	write_command(0x3A);
	write_data(0x55);
	write_command(0x36);
	write_data(0x00);
	write_command(0x21);
	write_command(0x13);
	write_command(0x29);
}

void ST7789::setarea_unsafe(const Rect2i & rect){

    #ifdef ST7789_EXPRIMENTAL_SKIP
    bool need = algo_.update(rect);
    if(!need) return;
    #endif

    const Vector2_t<uint16_t> p1 = offset_ + rect.position;
    const Vector2_t<uint16_t> p2 = offset_ + rect.get_end() - Vector2i(1,1);

    write_command(0x2a);
    write_data16(p1.x);
    write_data16(p2.x);

    write_command(0x2b);
    write_data16(p1.y);
    write_data16(p2.y);

    write_command(0x2c);
}

void ST7789::setpos_unsafe(const Vector2i & pos){

    #ifdef ST7789_EXPRIMENTAL_SKIP
    bool need = algo_.update(rect);
    if(!need) return;
    #endif

    write_command(0x2a);
    write_data16(pos.x + offset_.x);

    write_command(0x2b);
    write_data16(pos.y + offset_.y);

    write_command(0x2c);
}



void ST7789::putrect_unsafe(const Rect2i & rect, const RGB565 color){
    setarea_unsafe(rect);
    interface_.write_burst<RGB565>(color, size_t(rect));
}

void ST7789::puttexture_unsafe(const Rect2i & rect, const RGB565 * color_ptr){
    setarea_unsafe(rect);
    interface_.write_burst<RGB565>(color_ptr, size_t(rect));
}

void ST7789::putseg_v8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color){
    // PANIC()
    auto & self = *this;
    auto pos_ = pos;
    for(uint8_t i = 0x01; i; i <<= 1){
        if(i & mask) self.putpixel_unsafe(pos_, color);
        pos_.y++;
    }

    // DEBUG_PRINTLN(mask);
}

void ST7789::putseg_h8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color){
    PANIC();
}
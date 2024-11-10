#include "st7789.hpp"

using namespace yumud::drivers;
using namespace yumud;



void ST7789::init(){
    interface.init();
    
    writeCommand(0x01);

    delayMicroseconds(50);
	writeCommand(0x11);
    delayMicroseconds(50);
	writeCommand(0x3A);
	writeData(0x55);
	writeCommand(0x36);
	writeData(0x00);
	writeCommand(0x21);
	writeCommand(0x13);
	writeCommand(0x29);
}

void ST7789::setarea_unsafe(const Rect2i & rect){

    last_point_index = getPointIndex(rect.get_end().x-1, rect.get_end().y-1);

    Vector2_t<uint16_t> p1 = offset + rect.position;
    Vector2_t<uint16_t> p2 = offset + rect.get_end() - Vector2i(1,1);

    writeCommand(0x2a);
    writeData16(p1.x);
    writeData16(p2.x);

    writeCommand(0x2b);
    writeData16(p1.y);
    writeData16(p2.y);

    writeCommand(0x2c);
    area_locked = true;
}

void ST7789::setpos_unsafe(const Vector2i & pos){
    uint32_t this_point_index = getPointIndex(pos.x, pos.y);
    uint32_t last_point_index_temp = last_point_index;
    last_point_index = this_point_index;

    if((this_point_index == last_point_index_temp + 1) && (!area_locked)){
        return;
    }

    writeCommand(0x2a);
    writeData16(pos.x + offset.x);

    writeCommand(0x2b);
    writeData16(pos.y + offset.y);

    writeCommand(0x2c);
    area_locked = false;
}



void ST7789::putrect_unsafe(const Rect2i & rect, const RGB565 color){
    setarea_unsafe(rect);
    interface.writeMulti<RGB565>(color, size_t(rect));
}

void ST7789::puttexture_unsafe(const Rect2i & rect, const RGB565 * color_ptr){
    setarea_unsafe(rect);
    interface.writeMulti<RGB565>(color_ptr, size_t(rect));
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
    PANIC()
}
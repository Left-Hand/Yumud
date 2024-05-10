#include "st7789.hpp"


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

void ST7789::setArea_Unsafe(const Rect2i & rect){
    int x = rect.position.x;
    int y = rect.position.y;
    int w = rect.size.x;
    int h = rect.size.y;
    // uint32_t this_point_index = getPointIndex(x, y);
    // uint32_t last_point_index_temp = last_point_index;
    last_point_index = getPointIndex(x + w - 1, y + h - 1);
    // uart1.println(this_point_index, last_point_index, w, h);

    // if((this_point_index == last_point_index_temp + 1) && (!area_locked)){
    //     if(area_locked){

    //     }
    // }

    uint16_t x_offset = offset.x;
    uint16_t y_offset = offset.y;
    uint16_t x1 = x + x_offset;
    uint16_t x2 = x + w + x_offset - 1;
    uint16_t y1 = y + y_offset;
    uint16_t y2 = y + h + y_offset - 1;

    writeCommand(0x2a);
    writeData16(x1);
    writeData16(x2);

    writeCommand(0x2b);
    writeData16(y1);
    writeData16(y2);

    writeCommand(0x2c);
    area_locked = true;
}

void ST7789::setPosition_Unsafe(const Vector2i & pos){
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
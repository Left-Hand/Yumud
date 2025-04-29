//TODO 移除所有的unwrap


#include "st7789.hpp"
#include "types/range/range.hpp"
#include "core/debug/debug.hpp"

#define ST7789_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__)

using namespace ymd::drivers;
using namespace ymd;

template<typename T = void>
using IResult = ST7789::IResult<T>;

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

IResult<> ST7789::init(){
    if(const auto res = phy_.init(); res.is_err())          return res;
    udelay(50);
    if(const auto res = write_command(0x01); res.is_err())  return res;
    udelay(50);
	if(const auto res = write_command(0x11); res.is_err())  return res;
	if(const auto res = write_command(0x3A); res.is_err())  return res;
	if(const auto res = write_data8(0x55); res.is_err())    return res;
	if(const auto res = write_command(0x36); res.is_err())  return res;
	if(const auto res = write_data8(0x00); res.is_err())    return res;
	if(const auto res = write_command(0x21); res.is_err())  return res;
	if(const auto res = write_command(0x13); res.is_err())  return res;
	if(const auto res = write_command(0x29); res.is_err())  return res;
    return Ok();
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
    phy_.write_repeat<uint16_t>(color, rect.get_area()).unwrap();
}

void ST7789::puttexture_unsafe(const Rect2i & rect, const RGB565 * color_ptr){
    setarea_unsafe(rect);
    phy_.write_burst<uint16_t>(color_ptr, rect.get_area()).unwrap();
}

void ST7789::putseg_v8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color){
    auto & self = *this;
    auto pos_ = pos;
    for(uint8_t i = 0x01; i; i <<= 1){
        if(i & mask) self.putpixel_unsafe(pos_, color);
        pos_.y++;
    }
}

void ST7789::putseg_h8_unsafe(const Vector2i & pos, const uint8_t mask, const RGB565 color){
    TODO();
}

IResult<> ST7789::modify_ctrl(const bool yes,const uint8_t pos){
    uint8_t temp = 0x01 << pos;
    if (yes) scr_ctrl_ |= temp;
    else scr_ctrl_ &= ~temp;

    if(const auto res = write_command(0x36); res.is_err()) return res;
    if(const auto res = write_data8(scr_ctrl_); res.is_err()) return res;
    return Ok();
}


namespace ymd::drivers{
Result<void, DisplayerError> init_lcd(ST7789 & displayer, const ST7789_Presets preset){
    using enum ST7789_Presets;
    displayer.init();
    switch(preset){
        case _120X80:
            displayer.set_flip_x(false);
            displayer.set_flip_y(true);
            displayer.set_swap_xy(true);
            displayer.set_display_offset({40, 52}); 
            displayer.set_format_rgb(true);
            displayer.set_flush_dir_h(false);
            displayer.set_flush_dir_v(false);
            displayer.set_inversion(true);
            break;
        case _240X135:
            displayer.set_flip_x(true);
            displayer.set_flip_y(true);
            displayer.set_swap_xy(false);
            displayer.set_display_offset({52, 40}); 
            displayer.set_format_rgb(true);
            displayer.set_flush_dir_h(false);
            displayer.set_flush_dir_v(false);
            displayer.set_inversion(true);
            break;
        case _320X170:
            // displayer.set_flip_x(false);
            // displayer.set_flip_y(false);
            // displayer.set_swap_xy(true);
            // displayer.set_display_offset({-30, 30}); 
            // displayer.set_format_rgb(true);
            // displayer.set_flush_dir_h(false);
            // displayer.set_flush_dir_v(false);
            // displayer.set_inversion(true);

            displayer.set_flip_x(false);
            displayer.set_flip_y(true);
            displayer.set_swap_xy(true);
            // displayer.set_swap_xy(false);
            displayer.set_display_offset({0, 35}); 
            // displayer.set_display_offset({70, 0}); 
            // displayer.set_display_offset({35, 0}); 
            // displayer.set_display_offset({-0, 0}); 
            displayer.set_format_rgb(true);
            displayer.set_flush_dir_h(false);
            displayer.set_flush_dir_v(false);
            // displayer.set_inversion(false);
            displayer.set_inversion(true);
            break;
    }
    return Ok();
}

}


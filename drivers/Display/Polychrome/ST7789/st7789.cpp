//TODO 移除所有的unwrap


#include "st7789.hpp"
#include "algebra/regions/range2.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

template<typename T = void>
using IResult = ST7789::IResult<T>;



//判断刷新命令符必要性判断算法 以提高spi dma的吞吐率
bool ST7789::ST7789_ReflashAlgo::update(const math::Rect2<uint16_t> rect){
    const auto now_pt_range = get_point_index(now_area_);
    const auto desired_pt_range = get_point_index(rect);

    if(desired_pt_range.is_inside(now_pt_range) and rect.is_inside(now_area_)){
        last_point_ += desired_pt_range.length();
        return false;
    }else{
        now_area_ = rect;
        last_point_ = desired_pt_range.start;
        return true;
    }
} 

IResult<> ST7789::common_init(){
    if(const auto res = transport_.init(); 
        res.is_err())  return res;
    clock::delay(50us);
    if(const auto res = write_command(0x01); 
        res.is_err())  return res;
    clock::delay(50us);
	if(const auto res = write_command(0x11); 
        res.is_err())  return res;
	if(const auto res = write_command(0x3A); 
        res.is_err())  return res;
	if(const auto res = write_data8(0x55); 
        res.is_err())  return res;
	if(const auto res = write_command(0x36); 
        res.is_err())  return res;
	if(const auto res = write_data8(0x00); 
        res.is_err())  return res;
	if(const auto res = write_command(0x21); 
        res.is_err())  return res;
	if(const auto res = write_command(0x13); 
        res.is_err())  return res;
	if(const auto res = write_command(0x29); 
        res.is_err())  return res;
    return Ok();
}

IResult<> ST7789::setarea_unchecked(const math::Rect2<uint16_t> rect){

    #ifdef ST7789_EXPRIMENTAL_SKIP_EN
    bool need = algo_.update(rect);
    if(!need) return;
    #endif

    const math::Vec2<uint16_t> p1 = offset_ + rect.top_left;
    const math::Vec2<uint16_t> p2 = offset_ + rect.bottom_right() - math::Vec2<uint16_t>(1,1);

    if(const auto res = write_command(0x2a);
        res.is_err()) return res;
    if(const auto res = write_data16(p1.x);
        res.is_err()) return res;
    if(const auto res = write_data16(p2.x);
        res.is_err()) return res;
    if(const auto res = write_command(0x2b);
        res.is_err()) return res;
    if(const auto res = write_data16(p1.y);
        res.is_err()) return res;
    if(const auto res = write_data16(p2.y);
        res.is_err()) return res;
    if(const auto res = write_command(0x2c);
        res.is_err()) return res;

    return Ok();
}

IResult<> ST7789::setpos_unchecked(const math::Vec2<uint16_t> pos){

    #ifdef ST7789_EXPRIMENTAL_SKIP_EN
    bool need = algo_.update(rect);
    if(!need) return;
    #endif

    if(const auto res = write_command(0x2a);
        res.is_err()) return res;
    if(const auto res = write_data16(pos.x + offset_.x);
        res.is_err()) return res;
    if(const auto res = write_command(0x2b);
        res.is_err()) return res;
    if(const auto res = write_data16(pos.y + offset_.y);
        res.is_err()) return res;
    if(const auto res = write_command(0x2c);
        res.is_err()) return res;

    return Ok();
}



IResult<> ST7789::putrect_unchecked(const math::Rect2<uint16_t> rect, const RGB565 color){
    if(const auto res = setarea_unchecked(rect);
        res.is_err()) return res;
    if(const auto res = transport_.write_repeat_pixels(
            color, rect.area());
        res.is_err()) return res;
    return Ok();
}

IResult<> ST7789::put_texture_unchecked(const math::Rect2<uint16_t> rect, const RGB565 * color_ptr){
    if(const auto res = setarea_unchecked(rect);
        res.is_err()) return res;
    if(const auto res = transport_.write_burst_pixels(
            std::span<const RGB565>(color_ptr, rect.area()));
        res.is_err()) return res;
    return Ok();
}


IResult<> ST7789::modify_ctrl_reg(const bool is_high,const uint8_t pos){
    uint8_t temp = 0x01 << pos;
    if (is_high) scr_ctrl_ |= temp;
    else scr_ctrl_ &= ~temp;

    if(const auto res = write_command(0x36); 
        res.is_err()) return res;
    if(const auto res = write_data8(scr_ctrl_); 
        res.is_err()) return res;

    return Ok();
}


ST7789_Prelude::IResult<void> st7789_preset::_320X170::advanced_init(ST7789 & displayer){

    #if 0
    if(const auto res = displayer.enable_flip_x(DISEN);
        res.is_err()) return res;
    if(const auto res = displayer.enable_flip_y(EN);
        res.is_err()) return res;
    if(const auto res = displayer.enable_swap_xy(EN);
        res.is_err()) return res;
    #else
    if(const auto res = displayer.enable_flip_x(EN);
        res.is_err()) return res;
    if(const auto res = displayer.enable_flip_y(DISEN);
        res.is_err()) return res;
    if(const auto res = displayer.enable_swap_xy(EN);
        res.is_err()) return res;

    #endif
    if(const auto res = displayer.set_display_offset({0, 35}); 
        res.is_err()) return res;
    if(const auto res = displayer.enable_format_rgb(EN);
        res.is_err()) return res;
    if(const auto res = displayer.enable_flush_dir_h(EN);
        res.is_err()) return res;
    if(const auto res = displayer.enable_flush_dir_v(DISEN);
        res.is_err()) return res;
    if(const auto res = displayer.enable_inversion(EN);
        res.is_err()) return res;
    return Ok();
}
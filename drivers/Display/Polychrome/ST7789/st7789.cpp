//TODO 移除所有的unwrap


#include "st7789.hpp"
#include "types/regions/range2.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

template<typename T = void>
using IResult = ST7789::IResult<T>;

// #_ENdefine ST7789_EXPRIMENTAL_SKIP


//判断刷新命令符必要性判断算法 以提高spi dma的吞吐率
bool ST7789::ST7789_ReflashAlgo::update(const Rect2<uint16_t> rect){
    const auto curr_pt_range = get_point_index(curr_area_);
    const auto desired_pt_range = get_point_index(rect);

    if(desired_pt_range.is_inside(curr_pt_range) and rect.is_inside(curr_area_)){
        last_point_ += desired_pt_range.length();
        return false;
    }else{
        curr_area_ = rect;
        last_point_ = desired_pt_range.start;
        return true;
    }
} 

IResult<> ST7789::init(){
    if(const auto res = phy_.init(); 
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

IResult<> ST7789::setarea_unchecked(const Rect2<uint16_t> rect){

    #ifdef ST7789_EXPRIMENTAL_SKIP_EN
    bool need = algo_.update(rect);
    if(!need) return;
    #endif

    const Vec2<uint16_t> p1 = offset_ + rect.position;
    const Vec2<uint16_t> p2 = offset_ + rect.get_end() - Vec2<uint16_t>(1,1);

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

IResult<> ST7789::setpos_unchecked(const Vec2<uint16_t> pos){

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



IResult<> ST7789::putrect_unchecked(const Rect2<uint16_t> rect, const RGB565 color){
    if(const auto res = setarea_unchecked(rect);
        res.is_err()) return res;
    if(const auto res = phy_.write_repeat_pixels(
            color, rect.get_area());
        res.is_err()) return res;
    return Ok();
}

IResult<> ST7789::puttexture_unchecked(const Rect2<uint16_t> rect, const RGB565 * color_ptr){
    if(const auto res = setarea_unchecked(rect);
        res.is_err()) return res;
    if(const auto res = phy_.write_burst_pixels(
            std::span<const RGB565>(color_ptr, rect.get_area()));
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
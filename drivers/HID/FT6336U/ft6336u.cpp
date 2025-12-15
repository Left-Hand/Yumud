#include "ft6336u.hpp"

// https://www.iotword.com/23534.html

#define FT6636_DEBUG_EN

#ifdef FT6636_DEBUG_EN
#define FT6636_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define FT6636_PANIC(...) PANIC(__VA_ARGS__)
#define FT6636_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define FT6636_DEBUG(...)
#define FT6636_PANIC(...)  PANIC_NSRC()
#define FT6636_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Self = FT6336U;

using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;



// Touch Parameter
static constexpr uint8_t FT6336U_PRES_DOWN = 0x2;
static constexpr uint8_t FT6336U_COORD_UD  = 0x1;

// Registers
static constexpr uint8_t FT6336U_ADDR_DEVICE_MODE  =	0x00;

static constexpr uint8_t FT6336U_ADDR_GESTURE_ID     = 0x01;
static constexpr uint8_t FT6336U_ADDR_TD_STATUS 	 =	0x02;

static constexpr uint8_t FT6336U_ADDR_TOUCH1_EVENT  =	0x03;
static constexpr uint8_t FT6336U_ADDR_TOUCH1_ID 	 =	0x05;
static constexpr uint8_t FT6336U_ADDR_TOUCH1_X 	 =	0x03;
static constexpr uint8_t FT6336U_ADDR_TOUCH1_Y 	 =	0x05;
static constexpr uint8_t FT6336U_ADDR_TOUCH1_WEIGHT  = 0x07;
static constexpr uint8_t FT6336U_ADDR_TOUCH1_MISC    = 0x08;

static constexpr uint8_t FT6336U_ADDR_TOUCH2_EVENT  =	0x09;
static constexpr uint8_t FT6336U_ADDR_TOUCH2_ID 	 =	0x0B;
static constexpr uint8_t FT6336U_ADDR_TOUCH2_X 	 =	0x09;
static constexpr uint8_t FT6336U_ADDR_TOUCH2_Y 	 =	0x0B;
static constexpr uint8_t FT6336U_ADDR_TOUCH2_WEIGHT  = 0x0D;
static constexpr uint8_t FT6336U_ADDR_TOUCH2_MISC    = 0x0E;

static constexpr uint8_t FT6336U_ADDR_THRESHOLD          = 0x80;
static constexpr uint8_t FT6336U_ADDR_FILTER_COE         = 0x85;
static constexpr uint8_t FT6336U_ADDR_CTRL               = 0x86;

static constexpr uint8_t FT6336U_ADDR_TIME_ENTER_MONITOR = 0x87;
static constexpr uint8_t FT6336U_ADDR_ACTIVE_MODE_RATE   = 0x88;
static constexpr uint8_t FT6336U_ADDR_MONITOR_MODE_RATE  = 0x89;

static constexpr uint8_t FT6336U_ADDR_RADIAN_VALUE           = 0x91;
static constexpr uint8_t FT6336U_ADDR_OFFSET_LEFT_RIGHT      = 0x92;
static constexpr uint8_t FT6336U_ADDR_OFFSET_UP_DOWN         = 0x93;
static constexpr uint8_t FT6336U_ADDR_DISTANCE_LEFT_RIGHT    = 0x94;
static constexpr uint8_t FT6336U_ADDR_DISTANCE_UP_DOWN       = 0x95;
static constexpr uint8_t FT6336U_ADDR_DISTANCE_ZOOM          = 0x96;

static constexpr uint8_t FT6336U_ADDR_LIBRARY_VERSION_H  = 0xA1;
static constexpr uint8_t FT6336U_ADDR_LIBRARY_VERSION_L  = 0xA2;
static constexpr uint8_t FT6336U_ADDR_CHIP_ID            = 0xA3;
static constexpr uint8_t FT6336U_ADDR_G_MODE             = 0xA4;

static constexpr uint8_t FT6336U_ADDR_POWER_MODE         = 0xA5;
static constexpr uint8_t FT6336U_ADDR_FIRMARE_ID         = 0xA6;
static constexpr uint8_t FT6336U_ADDR_FOCALTECH_ID       = 0xA8;
static constexpr uint8_t FT6336U_ADDR_RELEASE_CODE_ID    = 0xAF;
static constexpr uint8_t FT6336U_ADDR_STATE              = 0xBC;

// Function Specific Type
IResult<> Self::init(){
    return Ok();
}

IResult<Self::DeviceMode> Self::get_device_mode() {
    auto & reg = regs_.devmode;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(DeviceMode(reg.device_mode));
}

IResult<> Self::set_device_mode(DeviceMode mode) {
    auto reg = RegCopy(regs_.devmode);
    reg.device_mode = mode;
    return write_reg(reg);
}

IResult<Self::GestureId> Self::get_gesture_id() {
    auto & reg = regs_.gestid;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return GestureId::from_u8(reg.gesture_id);
}

IResult<uint8_t> Self::get_touch_count() {
    auto & reg = regs_.td_status;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(uint8_t(reg.touch_cnt));
}

IResult<Self::TouchPoints> Self::get_touch_points(){
    uint8_t buf[9] = {0};
    if(const auto res = read_burst(regs_.td_status.ADDRESS, std::span(buf));
        res.is_err()) return Err(res.unwrap_err());
    const uint8_t touch_cnt = buf[0] & 0x0f;

    auto u12_from_be_bytes = [](const uint8_t b1, const uint8_t b2) -> uint16_t{
        return (uint16_t(b1) << 8 | uint16_t(b2)) & 0x0fff;
    };
    
    const TouchPoint p1 = TouchPoint{
        u12_from_be_bytes(buf[1], buf[2]), 
        u12_from_be_bytes(buf[3], buf[4])
    };

    const TouchPoint p2 = TouchPoint{
        u12_from_be_bytes(buf[5], buf[6]), 
        u12_from_be_bytes(buf[7], buf[8])
    };

    switch(touch_cnt){
        case 0: return Ok(points_ = points_.map_to_next(TouchPoints::from_none()));
        case 1: return Ok(points_ = points_.map_to_next(TouchPoints{p1}));
        case 2: return Ok(points_ = points_.map_to_next(TouchPoints{p1, p2}));
        default: return Err(Error::NthGreatThan1);
    }
}

IResult<uint8_t> Self::get_touch_event(const ChannelSelection nth) {
    const auto reg_addr = [&]{
        switch(nth){
            case ChannelSelection::_1: return FT6336U_ADDR_TOUCH1_EVENT;
            case ChannelSelection::_2: return FT6336U_ADDR_TOUCH2_EVENT;
            default: __builtin_unreachable();
        }
    }();

    return ({
        const auto res = read_reg(reg_addr);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 6);
    });
}

IResult<uint8_t> Self::get_touch_id(const ChannelSelection nth) {
    const auto reg_addr = [&]{
        switch(nth){
            case ChannelSelection::_1: return FT6336U_ADDR_TOUCH1_ID;
            case ChannelSelection::_2: return FT6336U_ADDR_TOUCH2_ID;
            default: __builtin_unreachable();
        }
    }();

    return ({
        const auto res = read_reg(reg_addr);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 4);
    });
}


IResult<uint8_t> Self::get_touch_weight(const ChannelSelection nth) {
    const auto reg_addr = [&]{
        switch(nth){
            case ChannelSelection::_1: return FT6336U_ADDR_TOUCH1_WEIGHT;
            case ChannelSelection::_2: return FT6336U_ADDR_TOUCH2_WEIGHT;
            default: __builtin_unreachable();
        }
    }();

    return read_reg(reg_addr);
}


IResult<uint8_t> Self::get_touch_misc(const ChannelSelection nth) {
    const auto reg_addr = [&]{
        switch(nth){
            case ChannelSelection::_1: return FT6336U_ADDR_TOUCH1_MISC;
            case ChannelSelection::_2: return FT6336U_ADDR_TOUCH2_MISC;
            default: __builtin_unreachable();
        }
    }();

    return ({
        const auto res = read_reg(reg_addr);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 4);
    });
}


IResult<uint8_t> Self::get_touch_threshold() {
    return read_reg(FT6336U_ADDR_THRESHOLD);
}

IResult<uint8_t> Self::get_filter_coefficient() {
    return read_reg(FT6336U_ADDR_FILTER_COE);
}

IResult<uint8_t> Self::get_ctrl_mode() {
    return read_reg(FT6336U_ADDR_CTRL);
}

IResult<uint8_t> Self::get_time_period_enter_monitor() {
    return read_reg(FT6336U_ADDR_TIME_ENTER_MONITOR);
}

IResult<uint8_t> Self::get_active_rate() {
    return read_reg(FT6336U_ADDR_ACTIVE_MODE_RATE);
}

IResult<uint8_t> Self::get_monitor_rate() {
    return read_reg(FT6336U_ADDR_MONITOR_MODE_RATE);
}


// Gesture Parameters
IResult<uint8_t> Self::get_radian_value() {
    return read_reg(FT6336U_ADDR_RADIAN_VALUE);
}

IResult<> Self::set_radian_value(uint8_t val) {
    return write_reg(FT6336U_ADDR_RADIAN_VALUE, val);
}

IResult<uint8_t> Self::get_offset_left_right() {
    return read_reg(FT6336U_ADDR_OFFSET_LEFT_RIGHT);
}

IResult<> Self::set_offset_left_right(uint8_t val) {
    return write_reg(FT6336U_ADDR_OFFSET_LEFT_RIGHT, val);
}

IResult<uint8_t> Self::get_offset_up_down() {
    return read_reg(FT6336U_ADDR_OFFSET_UP_DOWN);
}

IResult<> Self::set_offset_up_down(uint8_t val) {
    return write_reg(FT6336U_ADDR_OFFSET_UP_DOWN, val);
}

IResult<uint8_t> Self::get_distance_left_right() {
    return read_reg(FT6336U_ADDR_DISTANCE_LEFT_RIGHT);
}

IResult<> Self::set_distance_left_right(uint8_t val) {
    return write_reg(FT6336U_ADDR_DISTANCE_LEFT_RIGHT, val);
}

IResult<uint8_t> Self::get_distance_up_down() {
    return read_reg(FT6336U_ADDR_DISTANCE_UP_DOWN);
}

IResult<> Self::set_distance_up_down(uint8_t val) {
    return write_reg(FT6336U_ADDR_DISTANCE_UP_DOWN, val);
}

IResult<uint8_t> Self::get_distance_zoom() {
    return read_reg(FT6336U_ADDR_DISTANCE_ZOOM);
}

IResult<> Self::set_distance_zoom(uint8_t val) {
    return write_reg(FT6336U_ADDR_DISTANCE_ZOOM, val);
}


IResult<> Self::write_reg(const uint8_t addr, const uint8_t val) {
    if(const auto res = i2c_drv_.write_reg(addr, val);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<uint8_t> Self::read_reg(const uint8_t addr) {
    uint8_t val;
    if(const auto res = i2c_drv_.read_reg(addr, val);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(val);
}

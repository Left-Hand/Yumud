#include "FT6336.hpp"

// https://www.iotword.com/23534.html
// https://github.com/aselectroworks/Arduino-FT6336U/blob/master/src/FT6336U.h

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

using Error = FT6336U::Error;

template<typename T = void>
using IResult = Result<T, Error>;



// Touch Parameter
#define FT6336U_PRES_DOWN 0x2
#define FT6336U_COORD_UD  0x1

// Registers
#define FT6336U_ADDR_DEVICE_MODE 	0x00

#define FT6336U_ADDR_GESTURE_ID     0x01
#define FT6336U_ADDR_TD_STATUS 		0x02

#define FT6336U_ADDR_TOUCH1_EVENT 	0x03
#define FT6336U_ADDR_TOUCH1_ID 		0x05
#define FT6336U_ADDR_TOUCH1_X 		0x03
#define FT6336U_ADDR_TOUCH1_Y 		0x05
#define FT6336U_ADDR_TOUCH1_WEIGHT  0x07
#define FT6336U_ADDR_TOUCH1_MISC    0x08

#define FT6336U_ADDR_TOUCH2_EVENT 	0x09
#define FT6336U_ADDR_TOUCH2_ID 		0x0B
#define FT6336U_ADDR_TOUCH2_X 		0x09
#define FT6336U_ADDR_TOUCH2_Y 		0x0B
#define FT6336U_ADDR_TOUCH2_WEIGHT  0x0D
#define FT6336U_ADDR_TOUCH2_MISC    0x0E

#define FT6336U_ADDR_THRESHOLD          0x80
#define FT6336U_ADDR_FILTER_COE         0x85
#define FT6336U_ADDR_CTRL               0x86

#define FT6336U_ADDR_TIME_ENTER_MONITOR 0x87
#define FT6336U_ADDR_ACTIVE_MODE_RATE   0x88
#define FT6336U_ADDR_MONITOR_MODE_RATE  0x89

#define FT6336U_ADDR_RADIAN_VALUE           0x91
#define FT6336U_ADDR_OFFSET_LEFT_RIGHT      0x92
#define FT6336U_ADDR_OFFSET_UP_DOWN         0x93
#define FT6336U_ADDR_DISTANCE_LEFT_RIGHT    0x94
#define FT6336U_ADDR_DISTANCE_UP_DOWN       0x95
#define FT6336U_ADDR_DISTANCE_ZOOM          0x96

#define FT6336U_ADDR_LIBRARY_VERSION_H  0xA1
#define FT6336U_ADDR_LIBRARY_VERSION_L  0xA2
#define FT6336U_ADDR_CHIP_ID            0xA3
#define FT6336U_ADDR_G_MODE             0xA4

#define FT6336U_ADDR_POWER_MODE         0xA5
#define FT6336U_ADDR_FIRMARE_ID         0xA6
#define FT6336U_ADDR_FOCALTECH_ID       0xA8
#define FT6336U_ADDR_RELEASE_CODE_ID    0xAF
#define FT6336U_ADDR_STATE              0xBC

// Function Specific Type
IResult<> FT6336U::init(){
    return Ok();
}

// Result<size_t, Error> FT6336::get_touch_cnt(){
//     uint8_t buf[4];
//     uint8_t i = 0;
//     uint8_t set = FT_FALSE;;
//     uint8_t pointNub = 0;
//     static uint8_t cnt = 0;
    
//     if( touch_isOK == FT_FALSE )
//         return set;
    
//     cnt++;
//     if((cnt%10)==0 || cnt<10)
//     {
//         // read number of touch points 
//         ft6336_RdReg(FT_REG_NUM_FINGER,&pointNub,1);

//         pointNub= pointNub&0x0f;
//         if( pointNub && (pointNub < 3) )
//         {
//             cnt=0;
//             // read the point value 
//             pPos->status_bit.tpDown = 1;
//             pPos->status_bit.tpPress = 1;
//             pPos->status_bit.ptNum = pointNub;
            
//             for( i=0; i < CTP_MAX_TOUCH; i++)
//             {

//                 ft6336_RdReg( touch_press_reg[i], buf, 4 );
//                 if( pPos->status_bit.ptNum )
//                 {
//                     switch(lcddev.dir)
//                     {
//                         case 0:
//                             pPos->xpox[i]=((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
//                             pPos->ypox[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
//                             break;
//                         case 1:
//                             pPos->ypox[i]=lcddev.height-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
//                             pPos->xpox[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
//                             break;
//                         case 2:
//                             pPos->xpox[i]=lcddev.width-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
//                             pPos->ypox[i]=lcddev.height-(((uint16_t)(buf[2]&0X0F)<<8)+buf[3]);
//                             break;
//                         case 3:
//                             pPos->ypox[i] = ((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
//                             pPos->xpox[i] = lcddev.width-(((uint16_t)(buf[2]&0X0F)<<8)+buf[3]);
//                             break;
//                     } 
//                 }
//             } 
            
//             set = FT_TRUE;
//             if( pPos->xpox[0]==0 && pPos->ypox[0]==0)
//             {
//                 pPos->status = 0;
//             }
//         }
//     }
    
//     if( pPos->status_bit.ptNum == 0)
//     { 
//         if( pPos->status_bit.tpDown )
//         {   
//             pPos->status_bit.tpDown = 0;
//         }
//         else
//         { 
//             pPos->xpox[0] = 0xffff;
//             pPos->ypox[0] = 0xffff;
//             pPos->status = 0;
//         }
//     }
    
//     if( cnt>240 )
//        cnt=10;
    
//     return set;

//     return Err(Error::Unspecified);
// };


IResult<uint8_t> FT6336U::get_device_mode() {
    return read_reg(FT6336U_ADDR_DEVICE_MODE)
        .map([](const uint8_t b) -> uint8_t{return (b& 0x70) >> 4;});
}
IResult<> FT6336U::set_device_mode(DEVICE_MODE_Enum mode) {
    return write_reg(FT6336U_ADDR_DEVICE_MODE, (mode & 0x07) << 4);
}
IResult<FT6336U::GestureID> FT6336U::get_gesture_id() {
    return read_reg(FT6336U_ADDR_GESTURE_ID)
        .map([](const uint8_t b){return std::bit_cast<GestureID>(b);});
}
IResult<uint8_t> FT6336U::get_td_status() {
    return read_reg(FT6336U_ADDR_TD_STATUS);
}
IResult<uint8_t> FT6336U::get_touch_number() {
    return read_reg(FT6336U_ADDR_TD_STATUS)
        .map([](const uint8_t b) -> uint8_t{return (b & 0x0F);})
    ;
}
// Touch 1 functions
IResult<uint16_t> FT6336U::get_touch1_x() {
    uint8_t read_buf[2];
    read_buf[0] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH1_X);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    read_buf[1] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH1_X + 1);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    return Ok((read_buf[0] & 0x0f) << 8 | read_buf[1]);
}
IResult<uint16_t> FT6336U::get_touch1_y() {
    uint8_t read_buf[2];
    read_buf[0] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH1_Y);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    read_buf[1] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH1_Y + 1);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    return Ok((read_buf[0] & 0x0f) << 8 | read_buf[1]);
}
IResult<uint8_t> FT6336U::get_touch1_event() {
    return ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH1_EVENT);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 6);
    });
}
IResult<uint8_t> FT6336U::get_touch1_id() {
    return ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH1_ID);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 4);
    });
}


IResult<uint8_t> FT6336U::get_touch1_weight() {
    return read_reg(FT6336U_ADDR_TOUCH1_WEIGHT);
}
IResult<uint8_t> FT6336U::get_touch1_misc() {
    return ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH1_MISC);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 4);
    });
}
// Touch 2 functions
IResult<uint16_t> FT6336U::get_touch2_x() {
    uint8_t read_buf[2];
    read_buf[0] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH2_X);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    read_buf[1] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH2_X + 1);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    return Ok(((read_buf[0] & 0x0f) << 8) | read_buf[1]);
}
IResult<uint16_t> FT6336U::get_touch2_y() {
    uint8_t read_buf[2];
    read_buf[0] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH2_Y);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    read_buf[1] = ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH2_Y + 1);
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    return Ok(((read_buf[0] & 0x0f) << 8) | read_buf[1]);
}
IResult<uint8_t> FT6336U::get_touch2_event() {
    return ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH2_EVENT);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 6);
    });
}
IResult<uint8_t> FT6336U::get_touch2_id() {
    return ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH2_ID);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 4);
    });
}
IResult<uint8_t> FT6336U::get_touch2_weight() {
    return read_reg(FT6336U_ADDR_TOUCH2_WEIGHT);
}
IResult<uint8_t> FT6336U::get_touch2_misc() {
    return ({
        const auto res = read_reg(FT6336U_ADDR_TOUCH2_MISC);
        if(res.is_err()) return Err(res.unwrap_err());
        Ok(res.unwrap() >> 4);
    });
}

// Mode Parameter Register
IResult<uint8_t> FT6336U::get_touch_threshold() {
    return read_reg(FT6336U_ADDR_THRESHOLD);
}
IResult<uint8_t> FT6336U::get_filter_coefficient() {
    return read_reg(FT6336U_ADDR_FILTER_COE);
}
IResult<uint8_t> FT6336U::get_ctrl_mode() {
    return read_reg(FT6336U_ADDR_CTRL);
}
// IResult<> FT6336U::set_ctrl_mode(CTRL_MODE_Enum mode) {
//     return write_reg(FT6336U_ADDR_CTRL, mode);
// }
IResult<uint8_t> FT6336U::get_time_period_enter_monitor() {
    return read_reg(FT6336U_ADDR_TIME_ENTER_MONITOR);
}
IResult<uint8_t> FT6336U::get_active_rate() {
    return read_reg(FT6336U_ADDR_ACTIVE_MODE_RATE);
}
IResult<uint8_t> FT6336U::get_monitor_rate() {
    return read_reg(FT6336U_ADDR_MONITOR_MODE_RATE);
}

// Gesture Parameters
IResult<uint8_t> FT6336U::get_radian_value() {
    return read_reg(FT6336U_ADDR_RADIAN_VALUE);
}
IResult<> FT6336U::set_radian_value(uint8_t val) {
    return write_reg(FT6336U_ADDR_RADIAN_VALUE, val);
}
IResult<uint8_t> FT6336U::get_offset_left_right() {
    return read_reg(FT6336U_ADDR_OFFSET_LEFT_RIGHT);
}
IResult<> FT6336U::set_offset_left_right(uint8_t val) {
    return write_reg(FT6336U_ADDR_OFFSET_LEFT_RIGHT, val);
}
IResult<uint8_t> FT6336U::get_offset_up_down() {
    return read_reg(FT6336U_ADDR_OFFSET_UP_DOWN);
}
IResult<> FT6336U::set_offset_up_down(uint8_t val) {
    return write_reg(FT6336U_ADDR_OFFSET_UP_DOWN, val);
}
IResult<uint8_t> FT6336U::get_distance_left_right() {
    return read_reg(FT6336U_ADDR_DISTANCE_LEFT_RIGHT);
}
IResult<> FT6336U::set_distance_left_right(uint8_t val) {
    return write_reg(FT6336U_ADDR_DISTANCE_LEFT_RIGHT, val);
}
IResult<uint8_t> FT6336U::get_distance_up_down() {
    return read_reg(FT6336U_ADDR_DISTANCE_UP_DOWN);
}
IResult<> FT6336U::set_distance_up_down(uint8_t val) {
    return write_reg(FT6336U_ADDR_DISTANCE_UP_DOWN, val);
}
IResult<uint8_t> FT6336U::get_distance_zoom() {
    return read_reg(FT6336U_ADDR_DISTANCE_ZOOM);
}
IResult<> FT6336U::set_distance_zoom(uint8_t val) {
    return write_reg(FT6336U_ADDR_DISTANCE_ZOOM, val);
}


IResult<> FT6336U::write_reg(const uint8_t addr, const uint8_t val) {
    if(const auto res = i2c_drv_.write_reg(addr, val);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<uint8_t> FT6336U::read_reg(const uint8_t addr) {
    uint8_t val;
    if(const auto res = i2c_drv_.read_reg(addr, val);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(val);
}

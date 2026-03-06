//这个驱动已经完成
//这个驱动已经测试

//FT6336是一款电容触摸屏芯片

#pragma once

#include "ft6336u_prelude.hpp"
#include "core/utils/nth.hpp"

namespace ymd::drivers{


class FT6336U:public FT6336_Prelude{
public:
    explicit FT6336U(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit FT6336U(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit FT6336U(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    IResult<> init();


    IResult<GestureId>    get_gesture_id();
    IResult<DeviceMode>   get_device_mode();
    IResult<>             set_device_mode(DeviceMode);
    IResult<uint8_t>      get_touch_count(); 
    IResult<TouchPoints>       get_touch_points(); 


    IResult<uint8_t>      get_touch_event(const ChannelSelection nth);
    IResult<uint8_t>      get_touch_id(const ChannelSelection nth);
    IResult<uint8_t>      get_touch_weight(const ChannelSelection nth);
    IResult<uint8_t>      get_touch_misc(const ChannelSelection nth);

    // Mode Parameter Register
    IResult<uint8_t>  get_touch_threshold();
    IResult<uint8_t>  get_filter_coefficient();
    IResult<uint8_t>  get_ctrl_mode();
    IResult<>         set_ctrl_mode(CtrlMode mode);
    IResult<uint8_t>  get_time_period_enter_monitor();
    IResult<uint8_t>  get_active_rate();
    IResult<uint8_t>  get_monitor_rate();

    // Gestrue Parameter Register
    IResult<uint8_t>  get_radian_value();
    IResult<>         set_radian_value(uint8_t val);
    IResult<uint8_t>  get_offset_left_right();
    IResult<>         set_offset_left_right(uint8_t val);
    IResult<uint8_t>  get_offset_up_down();
    IResult<>         set_offset_up_down(uint8_t val);
    IResult<uint8_t>  get_distance_left_right();
    IResult<>         set_distance_left_right(uint8_t val);
    IResult<uint8_t>  get_distance_up_down();
    IResult<>         set_distance_up_down(uint8_t val);
    IResult<uint8_t>  get_distance_zoom();
    IResult<>         set_distance_zoom(uint8_t val);

    // System Information
    IResult<uint16_t> get_library_version();
    IResult<uint8_t>  get_chip_id();
    IResult<uint8_t>  get_g_mode();
    IResult<>         set_g_mode(GMode mode);
    IResult<uint8_t>  get_pwrmode();
    IResult<uint8_t>  get_firmware_id();
    IResult<uint8_t>  get_focaltech_id();
    IResult<uint8_t>  get_release_code_id();
    IResult<uint8_t>  get_state();

private:
    hal::I2cDrv i2c_drv_;
    FT6336_Regs regs_;
    TouchPoints points_ = TouchPoints::from_none();

    IResult<> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        return write_reg(T::REG_ADDR, reg.to_bits());
    }

    IResult<uint8_t> read_reg(const uint8_t addr);

    template<typename T>
    IResult<> read_reg(T & reg){
        const auto res = read_reg(T::REG_ADDR);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.as_bits_mut() = res.unwrap();
        return Ok();
    }

    IResult<> read_burst(const uint8_t addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<uint16_t> read_u12(const uint8_t addr){
        uint16_t ret;
        if(const auto res = read_burst_u12(addr, std::span(&ret, 1));
            res.is_err()) return Err(res.unwrap_err());
        return Ok(ret);
    }

    IResult<> read_burst_u12(const uint8_t addr, std::span<uint16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        for(auto & item : pbuf){
            item = item & 0x0fff;
        }
        return Ok();
    }
};

}
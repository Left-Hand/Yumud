//这个驱动还未完成

//FT6336是一款电容触摸屏芯片

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct FT6336_Prelude{
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x38);
    static constexpr auto MAX_I2C_BAUDRATE = 200000;
    static constexpr uint8_t PANEL_ID = 0x11;
    
    enum class Error_Kind:uint8_t{
        Unspecified
    };

    FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;



    static constexpr size_t CTP_MAX_TOUCH = 2;
    typedef enum {
        working_mode = 0b000,
        factory_mode = 0b100,
    } DEVICE_MODE_Enum;

    typedef enum {
        pollingMode = 0,
        triggerMode = 1,
    } G_MODE_Enum;


    typedef enum {
        keep_active_mode = 0,
        switch_to_monitor_mode = 1,
    } CTRL_MODE_Enum;
    typedef enum {
        touch = 0,
        stream,
        release,
    } TouchStatusEnum;
    typedef struct {
        TouchStatusEnum status;
        uint16_t x;
        uint16_t y;
    } TouchPointType;
    typedef struct {
        uint8_t touch_count;
        TouchPointType tp[2];
    } FT6336U_TouchPointType;

    typedef struct
    {
        uint8_t ptNum                    : 4;
        uint8_t tpDown                   : 1;
        uint8_t tpPress                  : 1;
        uint8_t res                      : 2;
    } Status_bit;

    typedef struct
    {
        uint16_t xpox[CTP_MAX_TOUCH];
        uint16_t ypox[CTP_MAX_TOUCH];
        union
        {
            uint8_t status;
            Status_bit status_bit;
        };
    }stru_pos;

};


struct FT6336_Regs:public FT6336_Prelude{

    enum class GestureID: uint8_t{
        Up = 0x10,
        Right = 0x14,
        Down = 0x18,
        Left = 0x1C,
        ZoomIn = 0x48,
        ZoomOut = 0x49,
        None = 0x00
    };

    struct R8_DevMode:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x00;
        uint8_t :4;
        uint8_t device_mode:3;
        uint8_t :1;
    }DEF_R8(devmode_reg)

    struct R8_GestId:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x01;
        uint8_t gesture_id;
    }DEF_R8(gestid_reg)

    struct R8_TdStatus:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x02;
        uint8_t touch_cnt:4;
        uint8_t :4;
    }DEF_R8(td_status_reg)

    struct R8_P1xh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x03;
        uint8_t p1xh:4;
        uint8_t :2;
        uint8_t p1ev:2;
    }DEF_R8(p1xh_reg)

    struct R8_P1xl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x04;
        uint8_t p1xl;
    }DEF_R8(p1xl_reg)

    struct R8_P1yh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x05;
        uint8_t p1yh:4;
        uint8_t :2;
        uint8_t p1id:2;
    }DEF_R8(p1yh_reg)

    struct R8_P1yl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x06;
        uint8_t p1yl;
    }DEF_R8(p1yl_reg)

    struct R8_P1Weight:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x07;
        uint8_t p1weight;
    }DEF_R8(p1weight_reg)

    struct R8_P1Misc:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x08;
        uint8_t touch_area:4;
        uint8_t :4;
    }DEF_R8(p1misc_reg)

    struct R8_P2xh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x09;
        uint8_t p2xh:4;
        uint8_t :2;
        uint8_t p2ev:2;
    }DEF_R8(p2xh_reg)

    struct R8_P2xl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0a;
        uint8_t p2xl;
    }DEF_R8(p2xl_reg)

    struct R8_P2yh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0b;
        uint8_t p2yh:4;
        uint8_t :2;
        uint8_t p2id:2;
    }DEF_R8(p2yh_reg)

    struct R8_P2yl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0c;
        uint8_t p2yl;
    }DEF_R8(p2yl_reg)

    struct R8_P2Weight:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0d;
        uint8_t p2weight;
    }DEF_R8(p2weight_reg)

    struct R8_P2Misc:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0e;
        uint8_t touch_area:4;
        uint8_t :4;
    }DEF_R8(p2misc_reg)

    struct R8_ThGroup:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x80;
        uint8_t touch_threshold;
    }DEF_R8(th_group_reg)

    struct R8_ThDiff:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x85;
        uint8_t touch_threshold;
    }DEF_R8(th_diff_reg)

    struct R8_Ctrl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x86;
        uint8_t enable_touch_detect:1;
        uint8_t :7;
    }DEF_R8(ctrl_reg)

    struct R8_TimeEnterMonnitor:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x87;
        uint8_t time_enter_monitor;
    }DEF_R8(time_enter_reg)

    struct R8_PeriodActive:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x88;
        uint8_t report_rate_when_active;
    }DEF_R8(period_active_reg)

    struct R8_PeriodMonitor:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x89;
        uint8_t report_rate_when_monitor;
    }DEF_R8(period_monitor_reg)

    struct R8_RadianValue:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x91;
        uint8_t minimal_allowed_angle;
    }DEF_R8(radian_value_reg)

    struct R8_OffsetLeftReight:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x92;
        uint8_t maxmimul_offset;
    }DEF_R8(offset_left_reg)
};

class FT6336U:public FT6336_Regs{
public:
    FT6336U(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    FT6336U(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    FT6336U(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    [[nodiscard]] IResult<> init();


    [[nodiscard]] IResult<GestureID> get_gesture_id();

    IResult<uint8_t> get_device_mode();
    IResult<>    set_device_mode(DEVICE_MODE_Enum);
    IResult<uint8_t> get_td_status();
    IResult<uint8_t> get_touch_number();
    IResult<uint16_t> get_touch1_x();
    IResult<uint16_t> get_touch1_y();
    IResult<uint8_t> get_touch1_event();
    IResult<uint8_t> get_touch1_id();
    IResult<uint8_t> get_touch1_weight();
    IResult<uint8_t> get_touch1_misc();
    IResult<uint16_t> get_touch2_x();
    IResult<uint16_t> get_touch2_y();
    IResult<uint8_t> get_touch2_event();
    IResult<uint8_t> get_touch2_id();
    IResult<uint8_t> get_touch2_weight();
    IResult<uint8_t> get_touch2_misc();

    // Mode Parameter Register
    IResult<uint8_t> get_touch_threshold();
    IResult<uint8_t> get_filter_coefficient();
    IResult<uint8_t> get_ctrl_mode();
    IResult<> set_ctrl_mode(CTRL_MODE_Enum mode);
    IResult<uint8_t> get_time_period_enter_monitor();
    IResult<uint8_t> get_active_rate();
    IResult<uint8_t> get_monitor_rate();

    // Gestrue Parameter Register
    IResult<uint8_t> get_radian_value();
    IResult<> set_radian_value(uint8_t val);
    IResult<uint8_t> get_offset_left_right();
    IResult<> set_offset_left_right(uint8_t val);
    IResult<uint8_t> get_offset_up_down();
    IResult<> set_offset_up_down(uint8_t val);
    IResult<uint8_t> get_distance_left_right();
    IResult<> set_distance_left_right(uint8_t val);
    IResult<uint8_t> get_distance_up_down();
    IResult<> set_distance_up_down(uint8_t val);
    IResult<uint8_t> get_distance_zoom();
    IResult<> set_distance_zoom(uint8_t val);

    // System Information
    IResult<uint16_t> get_library_version();
    IResult<uint8_t> get_chip_id();
    IResult<uint8_t> get_g_mode();
    IResult<> set_g_mode(G_MODE_Enum mode);
    IResult<uint8_t> get_pwrmode();
    IResult<uint8_t> get_firmware_id();
    IResult<uint8_t> get_focaltech_id();
    IResult<uint8_t> get_release_code_id();
    IResult<uint8_t> get_state();

private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){return write_reg(T::ADDRESS, reg);}

    [[nodiscard]] IResult<uint8_t> read_reg(const uint8_t addr);

    // template<typename T>
    // [[nodiscard]] IResult<> read_reg(T & reg){return read_reg(T::ADDRESS, reg);}

    [[nodiscard]] IResult<> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);
};

}
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
    enum class Error_Kind{
        Unspecified
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x38);
    static constexpr uint8_t PANEL_ID = 0x11;
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

class FT6336:public FT6336_Regs{
public:


    FT6336(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    FT6336(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    FT6336(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    [[nodiscard]] IResult<size_t> get_touch_cnt();

    [[nodiscard]] IResult<GestureID> get_gesture_id();

private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){return write_reg(T::ADDRESS, reg);}

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data);

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){return read_reg(T::ADDRESS, reg);}

    [[nodiscard]] IResult<> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);
};

}
#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{
class FT6336 {
public: 
    enum class Error{

    };
protected:

    using RegAddress = uint8_t;

    enum class GestureID: uint8_t{
        Up = 0x10,
        Right = 0x14,
        Down = 0x18,
        Left = 0x1C,
        ZoomIn = 0x48,
        ZoomOut = 0x49,
        None = 0x00
    };

    struct DevModeReg:public Reg8<>{
        RegAddress address = 0x00;
        uint8_t :4;
        uint8_t device_mode:3;
        uint8_t :1;
    };

    struct GestIdReg:public Reg8<>{
        RegAddress address = 0x01;
        uint8_t gesture_id;
    };

    struct TdStatusReg:public Reg8<>{
        RegAddress address = 0x02;
        uint8_t touch_cnt:4;
        uint8_t :4;
    };

    struct P1xhReg:public Reg8<>{
        RegAddress address = 0x03;
        uint8_t p1xh:4;
        uint8_t :2;
        uint8_t p1ev:2;
    };

    struct P1xlReg:public Reg8<>{
        RegAddress address = 0x04;
        uint8_t p1xl;
    };

    struct P1yhReg:public Reg8<>{
        RegAddress address = 0x05;
        uint8_t p1yh:4;
        uint8_t :2;
        uint8_t p1id:2;
    };

    struct P1ylReg:public Reg8<>{
        RegAddress address = 0x06;
        uint8_t p1yl;
    };

    struct P1WeightReg:public Reg8<>{
        RegAddress address = 0x07;
        uint8_t p1weight;
    };

    struct P1MiscReg:public Reg8<>{
        RegAddress address = 0x08;
        uint8_t touch_area:4;
    };

    struct P2xhReg:public Reg8<>{
        RegAddress address = 0x09;
        uint8_t p1xh:4;
        uint8_t :2;
        uint8_t p1ev:2;
    };

    struct P2xlReg:public Reg8<>{
        RegAddress address = 0x0a;
        uint8_t p1xl;
    };

    struct P2yhReg:public Reg8<>{
        RegAddress address = 0x0b;
        uint8_t p1yh:4;
        uint8_t :2;
        uint8_t p1id:2;
    };

    struct P2ylReg:public Reg8<>{
        RegAddress address = 0x0c;
        uint8_t p1yl;
    };

    struct P2WeightReg:public Reg8<>{
        RegAddress address = 0x0d;
        uint8_t p1weight;
    };

    struct P2MiscReg:public Reg8<>{
        RegAddress address = 0x0e;
        uint8_t touch_area:4;
    };

    struct ThGroupReg:public Reg8<>{
        RegAddress address = 0x80;
        uint8_t touch_threshold;
    };

    struct ThDiffReg:public Reg8<>{
        RegAddress address = 0x85;
        uint8_t touch_threshold;
    };

    struct CtrlReg:public Reg8<>{
        RegAddress address = 0x86;
        bool enable_touch_detect:1;
    };

    struct TimeEnterMonnitorReg:public Reg8<>{
        RegAddress address = 0x87;
        uint8_t time_enter_monitor;
    };

    struct PeriodActiveReg{
        RegAddress address = 0x88;
        uint8_t report_rate_when_active;
    };

    struct PeriodMonitorReg{
        RegAddress address = 0x89;
        uint8_t report_rate_when_monitor;
    };

    struct RadianValueReg{
        RegAddress address = 0x91;
        uint8_t minimal_allowed_angle;
    };

    struct OffsetLeftReightReg{
        RegAddress address = 0x92;
        uint8_t maxmimul_offset;
    };

    // struct OffsetLeftReightReg{
    //     RegAddress address = 0x92;
    //     uint8_t maxmimul_offset;
    // };

protected:
    hal::I2cDrv i2c_drv_;

    
    // [[nodiscard]] virtual Result<void, BusError> write_reg(const uint8_t addr, const uint8_t data);
    [[nodiscard]] Result<void, BusError> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> write_reg(const T & reg){return write_reg(reg.address, reg);}

    // [[nodiscard]] virtual Result<void, BusError> read_reg(const uint8_t addr, uint8_t & data);
    [[nodiscard]] Result<void, BusError> read_reg(const uint8_t addr, uint8_t & data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> read_reg(T & reg){return read_reg(reg.address, reg);}

    // [[nodiscard]] virtual Result<void, BusError> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);
    [[nodiscard]] Result<void, BusError> read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len);
    
public:
    static constexpr uint8_t DEFAULT_I2C_ADDR = 0x38;
    static constexpr uint8_t PANEL_ID = 0x11;

    FT6336(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    FT6336(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    FT6336(hal::I2c & i2c, const uint8_t i2c_addr = DEFAULT_I2C_ADDR):i2c_drv_(hal::I2cDrv{i2c, i2c_addr}){;}

    Result<size_t, Error> get_touch_cnt();

    Result<GestureID, Error> get_gesture_id();
};

}
#pragma once

#include "core/utils/Errno.hpp"
#include "core/utils/Result.hpp"
// #include "drivers/Camera/Camera.hpp" 
#include "hal/bus/sccb/sccb_drv.hpp"

#include "hal/bus/dvp/dvp.hpp"
#include "types/image/image.hpp"

namespace ymd::drivers{
struct MT9V034_Collections{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x5c);
    // static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x3d);
    static constexpr Vector2u CAMERA_SIZE = {188, 120};

    enum class RegAddress : uint8_t {
        ChipId = 0,
        RowStart,
        ColumnStart,
        WindowHeight,
        WindowWidth,
        HriBlanking,
        
        VertBlanking,
        Ctrl = 0x07,
        ShutterWidth1,
        ShutterWidth2,
        ShutterWidth3,
        ShutterWidthTotal = 0x0B,
        Reset,
        ReadMode = 0x0D,
        TypeCtrl = 0x0F,
        AdcCompandingMode = 0x1C,
        AecAgcDesired = 0xA5,
        AecLpf = 0xA8,
        AgcLpf = 0xAA,
        AgcAecBin = 0xAE,
        AgcAecEnable = 0xAF
    };

    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct MT9V034_Regs:public MT9V034_Collections{
    uint16_t rowStartReg = 0x01;
    uint16_t columnStartReg = 0x04;
    uint16_t windowHeightReg = 480;
    uint16_t windowWidthReg = 752;
    uint16_t hriBlankingReg = 91;
    uint16_t vertBlankingReg = 479;
    uint16_t ctrlReg = 0x188;
    uint16_t shutterWidth1Reg = 0x1BB;
    uint16_t shutterWidth2Reg = 0x1D9;
    uint16_t shutterWidth3Reg = 0x164;
    uint16_t shutterWidthTotalReg = 480;
    uint16_t readModeReg = 0x3A;
    uint16_t typeCtrlReg = 0x0;
    uint16_t adcCompandingModeReg = 0x202;
    uint16_t aecAgcDesiredReg = 0x3A;
    uint16_t aecLpfReg = 0x01;
    uint16_t agcLpfReg = 0x02;
    uint16_t agcAecBinReg = 0x0A;
    uint16_t agcAecEnableReg = 0x02;
};

class MT9V034:
    public Camera<Grayscale>,
    public MT9V034_Collections{
public:
    MT9V034(const hal::SccbDrv & sccb_drv):
        ImageBasics(CAMERA_SIZE),
        Camera<Grayscale>(CAMERA_SIZE),
        sccb_drv_(sccb_drv)
        {;}
    MT9V034(hal::SccbDrv && sccb_drv):
        ImageBasics(CAMERA_SIZE), 
        Camera<Grayscale>(CAMERA_SIZE),
        sccb_drv_(sccb_drv)
        {;}
    MT9V034(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        MT9V034(hal::SccbDrv{i2c, addr}){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_exposure_value(const uint16_t value);
private:
    hal::SccbDrv sccb_drv_;

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint16_t reg_data);

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint16_t & pData);

    void getpixel_unsafe(const Vector2u & pos, Grayscale & color) const {
        color = data[pos.x + pos.y * size().x];
    };

};

}
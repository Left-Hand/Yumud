#pragma once

#include "drivers/Camera/Camera.hpp" 


namespace ymd::drivers{

class MT9V034:public CameraWithSccb<Grayscale>{
protected:
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

    struct{
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


    void write_reg(const uint8_t addr, const uint16_t reg_data){
        sccb_drv_.write_reg(addr, reg_data);
    }

    void read_reg(const uint8_t addr, uint16_t & pData){
        sccb_drv_.read_reg(addr, pData);
    }

    void getpixel_unsafe(const Vector2i & pos, Grayscale & color) const override{
        color = data[pos.x + pos.y * size().x];
    };

public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x5c);
    static constexpr Vector2i CAMERA_SIZE = {188, 120};
public:
    MT9V034(const hal::SccbDrv & sccb_drv):
        ImageBasics(CAMERA_SIZE), CameraWithSccb<Grayscale>(sccb_drv, CAMERA_SIZE){;}
    MT9V034(hal::SccbDrv && sccb_drv):
        ImageBasics(CAMERA_SIZE), CameraWithSccb<Grayscale>(std::move(sccb_drv), CAMERA_SIZE){;}
    MT9V034(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        ImageBasics(CAMERA_SIZE), CameraWithSccb<Grayscale>(hal::SccbDrv{i2c, addr}, CAMERA_SIZE){;}

    bool init();

    bool verify();

    void setExposureValue(const uint16_t value);
};

}
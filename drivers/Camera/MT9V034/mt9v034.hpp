#pragma once

#include "core/utils/Errno.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/enum_array.hpp"
#include "hal/bus/sccb/sccb_drv.hpp"

#include "hal/bus/dvp/dvp.hpp"
#include "types/image/image.hpp"

namespace ymd::drivers{
struct MT9V034_Prelude{
    /// Camera i2c address configuration for {S_CTRL_ADR1, S_CTRL_ADR0} inputs
    /// (see Table 6 "address modes" in rev. 7 datasheet)

    // pub const CAM_PERIPH_ADDRESS_00: u8 = 0x90 >> 1;
    // pub const CAM_PERIPH_ADDRESS_01: u8 = 0x98 >> 1;
    // pub const CAM_PERIPH_ADDRESS_10: u8 = 0xB0 >> 1;
    // pub const CAM_PERIPH_ADDRESS_11: u8 = 0xB8 >> 1;
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x5c);

    enum class ParamContext:uint16_t {
        ContextA = 0x0188,
        ContextB = 0x8188,
    };


    
    // static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x3d);
    static constexpr Vector2u FRAME_SIZE = {188, 120};


    enum class GeneralRegAddress{
        ChipVersion = 0x00,
        /// Control register: used for eg switching config contexts
        Control = 0x07,
        /// Soft Reset of Logic
        SoftReset = 0x0c,
        /// High Dynamic Range enable
        HdrEnable = 0x0f,
        /// ADC Resolution Control
        AdcResCtrl = 0x1c,
        /// Row Noise Correction Control 1
        RowNoiseCorrCtrl = 0x70,
        /// Row Noise Constant
        RowNoiseConstant = 0x72,
        /// Test pattern storage
        TestPattern = 0x7f,
        /// Tiled digital gain
        TiledDigitalGain = 0x80,
        /// Desired luminance
        AgcAecDesiredBin = 0xa5,
        /// Exposure skip (number of frames to skip between changes in AEC, 0..15)
        AecUpdate = 0xa6,
        /// AEC Lowpass filter (0..2)
        AecLowpass = 0xa8,
        /// Gain skip (number of frames to skip between changes in AGC, 0-15)
        AgcUpdate = 0xa9,
        /// AGC Lowpass filter (0..2)
        AgcLowpass = 0xaa,
        /// AGC Max Gain
        AgcMaxGain = 0xab,
        /// Minimum coarse shutter width
        MinExposure = 0xac,
        /// Maximum coarse shutter width
        MaxExposure = 0xad,
        /// AEC/AGC Enable
        AecAgcEnable = 0xaf,
        /// Histogram pixel count
        AgcAecPixelCount = 0xb0,

        /// Register locking (either All/RW or just RO)
        RegisterLock = 0xfe,
    };

    enum class ContextARegAddress:uint8_t{
        ColumnStart = 0x01,
        RowStart = 0x02,
        WindowHeight = 0x03,
        WindowWidth = 0x04,
        /// Horizontal Blanking
        HBlanking = 0x05,
        /// Vertical Blanking
        VBlanking = 0x06,
        /// Coarse Shutter Width 1
        CoarseShutter1 = 0x08,
        /// Coarse Shutter Width 2
        CoarseShutter2 = 0x09,
        /// Coarse Shutter Width Control
        CoarseShutterCtrl = 0x0A,
        /// Coarse Shutter Width Total
        CoarseShutterTotal = 0x0B,
        ReadMode = 0x0D,
        V1Ctrl = 0x31,
        V2Ctrl = 0x32,
        V3Ctrl = 0x33,
        V4Ctrl = 0x34,
        /// Analog Gain Control
        AnalogGainCtrl = 0x35,
        /// Fine Shutter Width 1
        FineShutter1 = 0xD3,
        /// Fine Shutter Width 2
        FineShutter2 = 0xD4,
        /// Fine Shutter Width Total
        FineShutterTotal = 0xD5,
    };

    enum class ContextBRegAddress:uint8_t{
        ColumnStart = 0xC9,
        RowStart = 0xCA,
        WindowHeight = 0xCB,
        WindowWidth = 0xCC,
        HBlanking = 0xCD,
        VBlanking = 0xCE,
        CoarseShutter1 = 0xCF,
        CoarseShutter2 = 0xD0,
        CoarseShutterCtrl = 0xD1,
        CoarseShutterTotal = 0xD2,
        ReadMode = 0x0E,
        V1Ctrl = 0x39,
        V2Ctrl = 0x3A,
        V3Ctrl = 0x3B,
        V4Ctrl = 0x3C,
        AnalogGainCtrl = 0x36,
        FineShutter1 = 0xD6,
        FineShutter2 = 0xD7,
        FineShutterTotal = 0xD8,
    };

    enum class PixelTestPattern:uint16_t {
        None = 0x0000,
        VerticalShade = 0x0800,
        HorizontalShade = 0x1000,
        DiagonalShade = 0x1800,
    };

    enum class BinningFactor:uint8_t{
        /// No binning (full resolution)
        None = 1,
        /// Binning 2: combine two adjacent pixels
        Two = 2,
        /// Binning 4: combine four adjacent pixels
        Four = 4,
    };

    enum class BinningSelector:uint8_t{
        /// No binning (full resolution)
        None = 0b00,
        /// Binning 2: combine two adjacent pixels
        Two = 0b01,
        /// Binning 4: combine four adjacent pixels
        Four = 0b10,
    };



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

    enum class Error_Kind:uint8_t{
        WrongChipVersion
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    FRIEND_DERIVE_DEBUG(Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct MT9V034_Regs:public MT9V034_Prelude{
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

class MT9V034: public MT9V034_Prelude{
public:
    MT9V034(const hal::SccbDrv & sccb_drv):
        sccb_drv_(sccb_drv)
        {;}
    MT9V034(hal::SccbDrv && sccb_drv):
        sccb_drv_(std::move(sccb_drv))
        {;}
    MT9V034(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        MT9V034(hal::SccbDrv{i2c, addr}){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_exposure_value(const uint16_t value);
    [[nodiscard]] IResult<> set_gain(const real_t gain);

    [[nodiscard]] const auto & frame() const {return frame_;}
    [[nodiscard]] constexpr auto size() const {return FRAME_SIZE;}
private:
    hal::SccbDrv sccb_drv_;
    Image<Gray> frame_ = {FRAME_SIZE};

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint16_t data);

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint16_t & data);

    [[nodiscard]] IResult<> write_general_reg(
        const GeneralRegAddress addr, 
        const uint16_t data
    ){
        return write_reg(static_cast<uint8_t>(addr), data);
    }

    [[nodiscard]] IResult<> write_context_a_reg(
        const ContextARegAddress addr, 
        const uint16_t data
    ){
        return write_reg(static_cast<uint8_t>(addr), data);
    }

    [[nodiscard]] IResult<> write_context_b_reg(
        const ContextBRegAddress addr, 
        const uint16_t data
    ){
        return write_reg(static_cast<uint8_t>(addr), data);
    }

    [[nodiscard]] IResult<> set_context(const ParamContext context){
        return write_general_reg(GeneralRegAddress::Control, uint16_t(context));
    }

    [[nodiscard]] IResult<> set_exposure_range(const Range2u range);

    [[nodiscard]] IResult<> enable_pixel_test_pattern(
        const Enable en,
        const PixelTestPattern pattern
    );

    [[nodiscard]] IResult<> init_general_regs(const uint16_t max_pixel_count);

    /// Set just the maximum pixels to be used for adjusting automatic gain control
    /// Note this the _output_ pixel count, ie the pixels post-binning
    [[nodiscard]] IResult<> set_agc_pixel_count(uint16_t max_pixels){
        return write_general_reg(GeneralRegAddress::AgcAecPixelCount, max_pixels);
    }
};

}

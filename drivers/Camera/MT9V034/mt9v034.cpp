
#include "MT9V034.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

template<typename T = void>
using IResult = Result<T, MT9V034::Error>;

static constexpr uint8_t DEVICE_ADDR = 0xB8;

static constexpr uint8_t CHIP_ID_ADDR = 0x00;
static constexpr uint16_t CHIP_ID_VALUE = 0x1324;

static constexpr uint8_t ROW_START_ADDR = 0x01;
static constexpr uint8_t ROW_START_VALUE = 0x01;

static constexpr uint8_t COL_START_ADDR = 0x02;
static constexpr uint8_t COL_START_VALUE = 0x04;

static constexpr uint8_t WINDOWS_HEIGHT_ADDR = 0x03;
static constexpr uint16_t WINDOWS_HEIGHT_VALUE = 480;

static constexpr uint8_t WINDOW_WIDTH_ADDR = 0x04;
static constexpr uint16_t WINDOW_WIDTH_VALUE = 752;

static constexpr uint8_t HORIZONTAL_BLANKING_ADDR = 0x05;
static constexpr uint8_t HORIZONTAL_BLANKING_VALUE = 91;

static constexpr uint8_t CHIP_CONTROL_ADDR = 0x07;
static constexpr uint16_t CHIP_CONTROL_VALUE = 0x188;

static constexpr uint8_t COARSE_SHUTTER_WIDTH_1_ADDR = 0x08;
static constexpr uint16_t COARSE_SHUTTER_WIDTH_1_VALUE = 0x1BB;
static constexpr uint8_t COARSE_SHUTTER_WIDTH_2_ADDR = 0x09;
static constexpr uint16_t COARSE_SHUTTER_WIDTH_2_VALUE = 0x1D9;
static constexpr uint8_t COARSE_SHUTTER_WIDTH_3_ADDR = 0x0A;
static constexpr uint16_t COARSE_SHUTTER_WIDTH_3_VALUE = 0x164;
static constexpr uint8_t COARSE_SHUTTER_WIDTH_TOTAL_ADDR = 0x0B;
static constexpr uint16_t COARSE_SHUTTER_WIDTH_TOTAL_VALUE = (60 * 7); // Related to exposure time

static constexpr uint8_t VERTICAL_BLANKING_ADDR = 0x06;
// Value = COARSE_SHUTTER_WIDTH_TOTAL_VALUE-COARSE_SHUTTER_WIDTH_1_ADDR+7

static constexpr uint8_t READ_MODE_ADDR = 0x0D;
static constexpr uint8_t READ_MODE_VALUE = 0x3A; // 4*4 binning mode & flip row and column

static constexpr uint8_t SENSOR_TYPE_CONTROL_ADDR = 0x0F;
static constexpr uint8_t SENSOR_TYPE_CONTROL_VALUE = 0x00; // Linear opeartion & mono sensor

static constexpr uint8_t ADC_COMPANDING_MODE_ADDR = 0x1C;
static constexpr uint16_t ADC_COMPANDING_MODE_VALUE = 0x202;

static constexpr uint8_t AEC_AGC_DESIRED_BIN_ADDR = 0xA5;
static constexpr uint8_t AEC_AGC_DESIRED_BIN_VALUE = 0x3A;

static constexpr uint8_t AEC_LOW_PASS_FILTER_ADDR = 0xA8;
static constexpr uint8_t AEC_LOW_PASS_FILTER_VALUE = 0x01;

static constexpr uint8_t AGC_LOW_PASS_FILTER_ADDR = 0xAA;
static constexpr uint8_t AGC_LOW_PASS_FILTER_VALUE = 0x02;

static constexpr uint8_t AGC_AEC_BIN_DIFFERENCE_THRESHOLD_ADDR = 0xAE;
static constexpr uint8_t AGC_AEC_BIN_DIFFERENCE_THRESHOLD_VALUE = 0x0A;

static constexpr uint8_t AGC_AEC_ENABLE_ADDR = 0xAF;
static constexpr uint8_t AGC_AEC_ENABLE_VALUE = 0x02; // Enable AGC & disable AEC

static constexpr uint8_t GAIN_A = 0x35;
static constexpr uint8_t GAIN_B = 0x36; // Enable AGC & disable AEC

IResult<> MT9V034::init(){
    // DEBUG_PRINTLN(size);

    if(const auto res = validate();
        res.is_err()) return res;

    if(const auto res = write_reg(ROW_START_ADDR, ROW_START_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(COL_START_ADDR, COL_START_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(WINDOWS_HEIGHT_ADDR, WINDOWS_HEIGHT_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(WINDOW_WIDTH_ADDR, WINDOW_WIDTH_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(HORIZONTAL_BLANKING_ADDR, HORIZONTAL_BLANKING_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(VERTICAL_BLANKING_ADDR, COARSE_SHUTTER_WIDTH_TOTAL_VALUE-COARSE_SHUTTER_WIDTH_1_ADDR+7);
        res.is_err()) return res;
    if(const auto res = write_reg(CHIP_CONTROL_ADDR, CHIP_CONTROL_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(COARSE_SHUTTER_WIDTH_1_ADDR, COARSE_SHUTTER_WIDTH_1_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(COARSE_SHUTTER_WIDTH_2_ADDR, COARSE_SHUTTER_WIDTH_2_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(COARSE_SHUTTER_WIDTH_3_ADDR, COARSE_SHUTTER_WIDTH_3_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(COARSE_SHUTTER_WIDTH_TOTAL_ADDR, COARSE_SHUTTER_WIDTH_TOTAL_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(SENSOR_TYPE_CONTROL_ADDR, SENSOR_TYPE_CONTROL_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(READ_MODE_ADDR, READ_MODE_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(ADC_COMPANDING_MODE_ADDR, ADC_COMPANDING_MODE_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(AEC_AGC_DESIRED_BIN_ADDR, AEC_AGC_DESIRED_BIN_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(AEC_LOW_PASS_FILTER_ADDR, AEC_LOW_PASS_FILTER_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(AGC_LOW_PASS_FILTER_ADDR, AGC_LOW_PASS_FILTER_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(AGC_AEC_BIN_DIFFERENCE_THRESHOLD_ADDR, AGC_AEC_BIN_DIFFERENCE_THRESHOLD_VALUE);
        res.is_err()) return res;
    if(const auto res = write_reg(AGC_AEC_ENABLE_ADDR, AGC_AEC_ENABLE_VALUE);
        res.is_err()) return res;

    // if(const auto res = set_exposure_range({1, 0x1f4});
    //     res.is_err()) return res;


    // if(const auto res = init_general_regs(4096);
    //     res.is_err()) return res;

    #ifdef ENABLE_DVP
    const auto size = this->size();
    dvp.init((uint32_t *)data_.get(), (uint32_t *)data_.get(), size.x * size.y, size.y);
    #endif

    return Ok();
}


IResult<> MT9V034::validate(){

    uint16_t chip_version = 0;
    if(const auto res = read_reg(CHIP_ID_ADDR, chip_version);
        res.is_err()) return res;

    if(chip_version != CHIP_ID_VALUE)
        return Err(Error::WrongChipVersion);

    return Ok();
}

IResult<> MT9V034::set_exposure_value(const uint16_t value){
    return write_reg(0x0B, value);
}


IResult<> MT9V034::write_reg(const uint8_t addr, const uint16_t reg_data){
    if(const auto res = sccb_drv_.write_reg(addr, reg_data);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MT9V034::read_reg(const uint8_t addr, uint16_t & pData){
    if(const auto res = sccb_drv_.read_reg(addr, pData);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MT9V034::set_gain(const real_t gain){
    static constexpr auto GAIN_LSB = 0.0625_q16;
    static constexpr auto INV_GAIN_LSB = 1 / GAIN_LSB;
    static constexpr auto MAX_GAIN = 127 * GAIN_LSB;
    const uint16_t raw = uint16_t(CLAMP(gain, 0, MAX_GAIN) * INV_GAIN_LSB);

    if(const auto res = sccb_drv_.write_reg(GAIN_A, raw);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = sccb_drv_.write_reg(GAIN_B, raw);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MT9V034::init_general_regs(const uint16_t max_pixel_count){
            // reserved register recommendations from:
    // "Table 8. RECOMMENDED REGISTER SETTINGS AND PERFORMANCE IMPACT (RESERVED REGISTERS)"
    if(const auto res = write_reg(0x13, 0x2D2E);
        res.is_err()) return res; // reg 0x13 = 0x2d32 (11570)
    if(const auto res = write_reg(0x20, 0x03C7);
        res.is_err()) return res; // reg 0x20 = 0x1c1 (449)
    if(const auto res = write_reg(0x24, 0x001B);
        res.is_err()) return res; // reg 0x24 = 0x10 (16)
    if(const auto res = write_reg(0x2B, 0x0003);
        res.is_err()) return res; // reg 0x2B = 0x4 (4)
    if(const auto res = write_reg(0x2F, 0x0003);
        res.is_err()) return res; // reg 0x2F = 0x4 (4)

    // disable any test pattern by default
    if(const auto res = write_general_reg(GeneralRegAddress::TestPattern, 0x0000);
        res.is_err()) return res;

    if(const auto res = write_general_reg(GeneralRegAddress::RowNoiseCorrCtrl, 0x0101);
        res.is_err()) return res; //default noise correction
    if(const auto res = write_general_reg(GeneralRegAddress::AecAgcEnable, 0x0011);
        res.is_err()) return res; //enable both AEC and AGC
    if(const auto res = write_general_reg(GeneralRegAddress::HdrEnable, 0x0001);
        res.is_err()) return res; // enable HDR
    if(const auto res = write_general_reg(GeneralRegAddress::MinExposure, 0x0001);
        res.is_err()) return res;
    if(const auto res = write_general_reg(GeneralRegAddress::MaxExposure, 0x1F4);
        res.is_err()) return res;

    if(const auto res = write_general_reg(GeneralRegAddress::AgcMaxGain, 0x0010);
        res.is_err()) return res;
    if(const auto res = set_agc_pixel_count(max_pixel_count);
        res.is_err()) return res;
    if(const auto res = write_general_reg(GeneralRegAddress::AgcAecDesiredBin, 20);
        res.is_err()) return res; //desired luminance
    if(const auto res = write_general_reg(GeneralRegAddress::AdcResCtrl, 0x0303);
        res.is_err()) return res; // 12 bit ADC

    if(const auto res = write_general_reg(GeneralRegAddress::AecUpdate, 0x02);
        res.is_err()) return res;
    if(const auto res = write_general_reg(GeneralRegAddress::AecLowpass, 0x01);
        res.is_err()) return res;

    if(const auto res = write_general_reg(GeneralRegAddress::AgcUpdate, 0x02);
        res.is_err()) return res;
    if(const auto res = write_general_reg(GeneralRegAddress::AgcLowpass, 0x02);
        res.is_err()) return res;

    return Ok();
}

IResult<> MT9V034::enable_pixel_test_pattern(
    const Enable en,
    const PixelTestPattern pattern
){
    auto & self = *this;
    if(en == EN){
        if(const auto res = self.write_general_reg(
            GeneralRegAddress::TestPattern,
            uint16_t(pattern) | 0x2000
        ); res.is_err()) return res;
        //disable row noise correction as well (pass through test pixels)
        return self.write_general_reg(GeneralRegAddress::RowNoiseCorrCtrl, 0x0000);
    } else {
        // clear the test pattern
        if(const auto res = self.write_general_reg(GeneralRegAddress::TestPattern, 0x0000);
            res.is_err()) return res;
        //enable default noise correction
        return self.write_general_reg(GeneralRegAddress::RowNoiseCorrCtrl, 0x0101);
    }
}

IResult<> MT9V034::set_exposure_range(const Range2u range){
    if(const auto res = write_general_reg(GeneralRegAddress::MinExposure, range.from);
        res.is_err()) return res;

    if(const auto res = write_general_reg(GeneralRegAddress::MaxExposure, range.to);
        res.is_err()) return res;

    return Ok();
}
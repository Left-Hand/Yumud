
#include "MT9V034.hpp"

using namespace ymd::hal;
using namespace ymd::drivers;

#define DEVICE_ADDR 0xB8

#define CHIP_ID_ADDR 0x00
#define CHIP_ID_VALUE 0x1324

#define ROW_START_ADDR 0x01
#define ROW_START_VALUE 0x01

#define COL_START_ADDR 0x02
#define COL_START_VALUE 0x04

#define WINDOWS_HEIGHT_ADDR 0x03
#define WINDOWS_HEIGHT_VALUE 480

#define WINDOW_WIDTH_ADDR 0x04
#define WINDOW_WIDTH_VALUE 752

#define HORIZONTAL_BLANKING_ADDR 0x05
#define HORIZONTAL_BLANKING_VALUE 91

#define CHIP_CONTROL_ADDR 0x07
#define CHIP_CONTROL_VALUE 0x188

#define COARSE_SHUTTER_WIDTH_1_ADDR 0x08
#define COARSE_SHUTTER_WIDTH_1_VALUE 0x1BB
#define COARSE_SHUTTER_WIDTH_2_ADDR 0x09
#define COARSE_SHUTTER_WIDTH_2_VALUE 0x1D9
#define COARSE_SHUTTER_WIDTH_3_ADDR 0x0A
#define COARSE_SHUTTER_WIDTH_3_VALUE 0x164
#define COARSE_SHUTTER_WIDTH_TOTAL_ADDR 0x0B
#define COARSE_SHUTTER_WIDTH_TOTAL_VALUE (60 * 7) // Related to exposure time

#define VERTICAL_BLANKING_ADDR 0x06
// Value = COARSE_SHUTTER_WIDTH_TOTAL_VALUE-COARSE_SHUTTER_WIDTH_1_ADDR+7

#define READ_MODE_ADDR 0x0D
#define READ_MODE_VALUE 0x3A // 4*4 binning mode & flip row and column

#define SENSOR_TYPE_CONTROL_ADDR 0x0F
#define SENSOR_TYPE_CONTROL_VALUE 0x00 // Linear opeartion & mono sensor

#define ADC_COMPANDING_MODE_ADDR 0x1C
#define ADC_COMPANDING_MODE_VALUE 0x202

#define AEC_AGC_DESIRED_BIN_ADDR 0xA5
#define AEC_AGC_DESIRED_BIN_VALUE 0x3A

#define AEC_LOW_PASS_FILTER_ADDR 0xA8
#define AEC_LOW_PASS_FILTER_VALUE 0x01

#define AGC_LOW_PASS_FILTER_ADDR 0xAA
#define AGC_LOW_PASS_FILTER_VALUE 0x02

#define AGC_AEC_BIN_DIFFERENCE_THRESHOLD_ADDR 0xAE
#define AGC_AEC_BIN_DIFFERENCE_THRESHOLD_VALUE 0x0A

#define AGC_AEC_ENABLE_ADDR 0xAF
#define AGC_AEC_ENABLE_VALUE 0x02 // Enable AGC & disable AEC

bool MT9V034::init(){
    // DEBUG_PRINTLN(size);

    if(false == verify()){
        return false;
    }

    writeReg(ROW_START_ADDR, ROW_START_VALUE);
    writeReg(COL_START_ADDR, COL_START_VALUE);
    writeReg(WINDOWS_HEIGHT_ADDR, WINDOWS_HEIGHT_VALUE);
    writeReg(WINDOW_WIDTH_ADDR, WINDOW_WIDTH_VALUE);
    writeReg(HORIZONTAL_BLANKING_ADDR, HORIZONTAL_BLANKING_VALUE);
    writeReg(VERTICAL_BLANKING_ADDR, COARSE_SHUTTER_WIDTH_TOTAL_VALUE-COARSE_SHUTTER_WIDTH_1_ADDR+7);
    writeReg(CHIP_CONTROL_ADDR, CHIP_CONTROL_VALUE);
    writeReg(COARSE_SHUTTER_WIDTH_1_ADDR, COARSE_SHUTTER_WIDTH_1_VALUE);
    writeReg(COARSE_SHUTTER_WIDTH_2_ADDR, COARSE_SHUTTER_WIDTH_2_VALUE);
    writeReg(COARSE_SHUTTER_WIDTH_3_ADDR, COARSE_SHUTTER_WIDTH_3_VALUE);
    writeReg(COARSE_SHUTTER_WIDTH_TOTAL_ADDR, COARSE_SHUTTER_WIDTH_TOTAL_VALUE);
    writeReg(SENSOR_TYPE_CONTROL_ADDR, SENSOR_TYPE_CONTROL_VALUE);
    writeReg(READ_MODE_ADDR, READ_MODE_VALUE);
    writeReg(ADC_COMPANDING_MODE_ADDR, ADC_COMPANDING_MODE_VALUE);
    writeReg(AEC_AGC_DESIRED_BIN_ADDR, AEC_AGC_DESIRED_BIN_VALUE);
    writeReg(AEC_LOW_PASS_FILTER_ADDR, AEC_LOW_PASS_FILTER_VALUE);
    writeReg(AGC_LOW_PASS_FILTER_ADDR, AGC_LOW_PASS_FILTER_VALUE);
    writeReg(AGC_AEC_BIN_DIFFERENCE_THRESHOLD_ADDR, AGC_AEC_BIN_DIFFERENCE_THRESHOLD_VALUE);
    writeReg(AGC_AEC_ENABLE_ADDR, AGC_AEC_ENABLE_VALUE);

    #ifdef ENABLE_DVP
    dvp.init((uint32_t *)data.get(), (uint32_t *)data.get(), size.x * size.y, size.y);
    #endif


    return true;
}


bool MT9V034::verify(){
    uint16_t chip_version = 0;
    [[maybe_unused]]scexpr uint16_t valid_version = 0x1324;
    readReg(CHIP_ID_ADDR, chip_version);
    // DEBUG_PRINTLN("mt9v id is", chip_version);
    // return (chip_version == valid_version);
    return true;
}

void MT9V034::setExposureValue(const uint16_t value){
    writeReg(0x0B, value);
}
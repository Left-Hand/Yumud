#include "PMW3901.hpp"

#define PMW3901_REG_Product_ID      0x00
#define PMW3901_REG_Revision_ID     0x01
#define PMW3901_REG_Motion          0x02
#define PMW3901_REG_Delta_X_L       0x03
#define PMW3901_REG_Delta_X_H       0x04
#define PMW3901_REG_Delta_Y_L       0x05
#define PMW3901_REG_Delta_Y_H       0x06
#define PMW3901_REG_Squal           0x07
#define PMW3901_REG_RawData_Sum     0x08
#define PMW3901_REG_RawData_Max     0x09
#define PMW3901_REG_RawData_Min_    0x0A
#define PMW3901_REG_Shutter_Lower   0x0B
#define PMW3901_REG_Shutter_Upper   0x0C
#define PMW3901_REG_Observation     0x15
#define PMW3901_REG_Motion_Burst    0x16
#define PMW3901_REG_Power_Up_Reset  0x3A
#define PMW3901_REG_Shutdown        0x3B
#define PMW3901_REG_RawData_Grab    0x58
#define PMW3901_REG_RawData_Grab_Status     0x59
#define PMW3901_REG_Inverse_Product_ID      0x5F

using namespace ymd::drivers;

void PMW3901::writeByte(const uint8_t command, const uint8_t data){
    spi_drv_.writeSingle<uint8_t>(command, CONT);
    spi_drv_.writeSingle<uint8_t>(data);
}


void PMW3901::readByte(const uint8_t command, uint8_t & data){
    spi_drv_.writeSingle<uint8_t>(command, CONT);
    spi_drv_.readSingle<uint8_t>(data);
}

void PMW3901::readBytes(const uint8_t command, uint8_t * data, const size_t len){
    spi_drv_.writeSingle<uint8_t>(command, CONT);
    spi_drv_.readMulti<uint8_t>(data, len);
}

void PMW3901::init() {
    this->isInited = 0;
    uint8_t Data = 0;

    // CS_L;
    delay(100);
    // CS_H;
    delay(100);

    writeByte(PMW3901_REG_Power_Up_Reset, 0x5A);
    delay(100);

    readByte(PMW3901_REG_Product_ID, Data);
    if (Data != 0x49) {
        return;
    }

    readByte(PMW3901_REG_Inverse_Product_ID, Data);
    if (Data != 0xB6) {
        return;
    }
    update();

    writeByte(0x7F, 0x00);
    writeByte(0x61, 0xAD);
    writeByte(0x7F, 0x03);
    writeByte(0x40, 0x00);
    writeByte(0x7F, 0x05);
    writeByte(0x41, 0xB3);
    writeByte(0x43, 0xF1);
    writeByte(0x45, 0x14);
    writeByte(0x5B, 0x32);
    writeByte(0x5F, 0x34);
    writeByte(0x7B, 0x08);
    writeByte(0x7F, 0x06);
    writeByte(0x44, 0x1B);
    writeByte(0x40, 0xBF);
    writeByte(0x4E, 0x3F);
    writeByte(0x7F, 0x08);
    writeByte(0x65, 0x20);
    writeByte(0x6A, 0x18);
    writeByte(0x7F, 0x09);
    writeByte(0x4F, 0xAF);
    writeByte(0x5F, 0x40);
    writeByte(0x48, 0x80);
    writeByte(0x49, 0x80);
    writeByte(0x57, 0x77);
    writeByte(0x60, 0x78);
    writeByte(0x61, 0x78);
    writeByte(0x62, 0x08);
    writeByte(0x63, 0x50);
    writeByte(0x7F, 0x0A);
    writeByte(0x45, 0x60);
    writeByte(0x7F, 0x00);
    writeByte(0x4D, 0x11);
    writeByte(0x55, 0x80);
    writeByte(0x74, 0x1F);
    writeByte(0x75, 0x1F);
    writeByte(0x4A, 0x78);
    writeByte(0x4B, 0x78);
    writeByte(0x44, 0x08);
    writeByte(0x45, 0x50);
    writeByte(0x64, 0xFF);
    writeByte(0x65, 0x1F);
    writeByte(0x7F, 0x14);
    writeByte(0x65, 0x60);
    writeByte(0x66, 0x08);
    writeByte(0x63, 0x78);
    writeByte(0x7F, 0x15);
    writeByte(0x48, 0x58);
    writeByte(0x7F, 0x07);
    writeByte(0x41, 0x0D);
    writeByte(0x43, 0x14);
    writeByte(0x4B, 0x0E);
    writeByte(0x45, 0x0F);
    writeByte(0x44, 0x42);
    writeByte(0x4C, 0x80);
    writeByte(0x7F, 0x10);
    writeByte(0x5B, 0x02);
    writeByte(0x7F, 0x07);
    writeByte(0x40, 0x41);
    writeByte(0x70, 0x00);
    delay(10);
    writeByte(0x32, 0x44);
    writeByte(0x7F, 0x07);
    writeByte(0x40, 0x40);
    writeByte(0x7F, 0x06);
    writeByte(0x62, 0xf0);
    writeByte(0x63, 0x00);
    writeByte(0x7F, 0x0D);
    writeByte(0x48, 0xC0);
    writeByte(0x6F, 0xd5);
    writeByte(0x7F, 0x00);
    writeByte(0x5B, 0xa0);
    writeByte(0x4E, 0xA8);
    writeByte(0x5A, 0x50);
    writeByte(0x40, 0x80);


    this->isInited = 1;
}

void PMW3901::update(){
    if (this->isInited) {
        uint8_t Data[4] = {0, 0, 0, 0};
        readByte(PMW3901_REG_Motion, Data[0]);
        readBytes(PMW3901_REG_Delta_X_L, Data, 4);

        this->deltaX = (int16_t) ((Data[1] << 8) | Data[0]);
        this->deltaY = (int16_t) ((Data[3] << 8) | Data[2]);
    }
}
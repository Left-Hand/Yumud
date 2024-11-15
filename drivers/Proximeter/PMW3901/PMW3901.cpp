#include "PMW3901.hpp"
#include "types/image/image.hpp"

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

void PMW3901::writeReg(const uint8_t command, const uint8_t data){
    spi_drv_.writeSingle<uint8_t>(command, CONT);
    spi_drv_.writeSingle<uint8_t>(data);
}


void PMW3901::readReg(const uint8_t command, uint8_t & data){
    spi_drv_.writeSingle<uint8_t>(command, CONT);
    spi_drv_.readSingle<uint8_t>(data);
}


void PMW3901::readImage(ImageWritable<Grayscale> & img){
    int count = 0;
    scexpr uint8_t mask = 0x0c; //mask to take bits 2 and 3 from b

    for (int ii = 0; ii < 1225; ii++) { //for 1 frame of 1225 pixels (35*35)
        uint8_t a,b, hold;
        do { 
        //if data is either invalid status
        //check status bits 6 and 7
        //if 01 move upper 6 bits into temp value
        //if 00 or 11, reread
        //else lower 2 bits into temp value
        readReg(0x58,a); //read register
        hold = a >> 6; //right shift to leave top two bits for ease of check.
        } while((hold == 0x03) || (hold == 0x00));
        
        if (hold == 0x01) { //if data is upper 6 bits
        readReg(0x58, b); //read next set to get lower 2 bits
        uint8_t pixel = a; //set pixel to a
        pixel = pixel << 2; //push left to 7:2
        pixel += (b & mask); //set lower 2 from b to 1:0
        img[count++] = Grayscale(pixel); //put temp value in fbuffer array
        //delayMicroseconds(100);
        }
    }

    writeReg(0x70, 0x00);   //More magic? 
    writeReg(0x58, 0xFF);

    uint8_t temp, check; 

    do { //keep reading and testing
        readReg(0x58, temp); //read status register
        check = temp>>6; //rightshift 6 bits so only top two stay 
    } while(check == 0x03); //while bits aren't set denoting ready state
}

void PMW3901::init() {

    writeReg(PMW3901_REG_Power_Up_Reset, 0x5A);
    delay(5);

    if(!assertReg(PMW3901_REG_Product_ID, 0x49)) return;
    if(!assertReg(PMW3901_REG_Inverse_Product_ID, 0xB6)) return;
    update();

    writeReg(0x7F, 0x00);
    writeReg(0x61, 0xAD);
    writeReg(0x7F, 0x03);
    writeReg(0x40, 0x00);
    writeReg(0x7F, 0x05);
    writeReg(0x41, 0xB3);
    writeReg(0x43, 0xF1);
    writeReg(0x45, 0x14);
    writeReg(0x5B, 0x32);
    writeReg(0x5F, 0x34);
    writeReg(0x7B, 0x08);
    writeReg(0x7F, 0x06);
    writeReg(0x44, 0x1B);
    writeReg(0x40, 0xBF);
    writeReg(0x4E, 0x3F);
    writeReg(0x7F, 0x08);
    writeReg(0x65, 0x20);
    writeReg(0x6A, 0x18);
    writeReg(0x7F, 0x09);
    writeReg(0x4F, 0xAF);
    writeReg(0x5F, 0x40);
    writeReg(0x48, 0x80);
    writeReg(0x49, 0x80);
    writeReg(0x57, 0x77);
    writeReg(0x60, 0x78);
    writeReg(0x61, 0x78);
    writeReg(0x62, 0x08);
    writeReg(0x63, 0x50);
    writeReg(0x7F, 0x0A);
    writeReg(0x45, 0x60);
    writeReg(0x7F, 0x00);
    writeReg(0x4D, 0x11);
    writeReg(0x55, 0x80);
    writeReg(0x74, 0x1F);
    writeReg(0x75, 0x1F);
    writeReg(0x4A, 0x78);
    writeReg(0x4B, 0x78);
    writeReg(0x44, 0x08);
    writeReg(0x45, 0x50);
    writeReg(0x64, 0xFF);
    writeReg(0x65, 0x1F);
    writeReg(0x7F, 0x14);
    writeReg(0x65, 0x60);
    writeReg(0x66, 0x08);
    writeReg(0x63, 0x78);
    writeReg(0x7F, 0x15);
    writeReg(0x48, 0x58);
    writeReg(0x7F, 0x07);
    writeReg(0x41, 0x0D);
    writeReg(0x43, 0x14);
    writeReg(0x4B, 0x0E);
    writeReg(0x45, 0x0F);
    writeReg(0x44, 0x42);
    writeReg(0x4C, 0x80);
    writeReg(0x7F, 0x10);
    writeReg(0x5B, 0x02);
    writeReg(0x7F, 0x07);
    writeReg(0x40, 0x41);
    writeReg(0x70, 0x00);
    delay(10);
    writeReg(0x32, 0x44);
    writeReg(0x7F, 0x07);
    writeReg(0x40, 0x40);
    writeReg(0x7F, 0x06);
    writeReg(0x62, 0xf0);
    writeReg(0x63, 0x00);
    writeReg(0x7F, 0x0D);
    writeReg(0x48, 0xC0);
    writeReg(0x6F, 0xd5);
    writeReg(0x7F, 0x00);
    writeReg(0x5B, 0xa0);
    writeReg(0x4E, 0xA8);
    writeReg(0x5A, 0x50);
    writeReg(0x40, 0x80);

}

void PMW3901::setLed(bool ledOn){
    writeReg(0x7f, 0x14);
    writeReg(0x6f, ledOn ? 0x1c : 0x00);
    writeReg(0x7f, 0x00);
}

void PMW3901::update(){
    spi_drv_.readMulti<uint8_t>(&Motion, 5);
}
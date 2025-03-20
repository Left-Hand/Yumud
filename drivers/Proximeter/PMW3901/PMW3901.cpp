#include "PMW3901.hpp"
#include "types/image/image.hpp"
#include "sys/debug/debug.hpp"

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

#define PMW3901_DEBUG

#ifdef PMW3901_DEBUG
#undef PMW3901_DEBUG
#define PMW3901_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define PMW3901_PANIC(...) PANIC(__VA_ARGS__)
#define PMW3901_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define PMW3901_DEBUG(...)
#define PMW3901_PANIC(...)  PANIC()
#define PMW3901_ASSERT(cond, ...) ASSERT(cond)
#endif


using namespace ymd;
using namespace ymd::drivers;

BusError PMW3901::write_reg(const uint8_t command, const uint8_t data){
    spi_drv_.writeSingle<uint8_t>(command | 0x80, CONT).unwrap();
    return spi_drv_.writeSingle<uint8_t>(data);
}


BusError PMW3901::read_reg(const uint8_t command, uint8_t & data){
    spi_drv_.writeSingle<uint8_t>(command & 0x7f, CONT).unwrap();
    return spi_drv_.readSingle<uint8_t>(data);
}

BusError PMW3901::read_burst(const uint8_t command, uint8_t * data, const size_t len){
    spi_drv_.writeSingle<uint8_t>(command & 0x7f, CONT).unwrap();
    return spi_drv_.read_burst<uint8_t>(data, len);
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
        read_reg(0x58,a); //read register
        hold = a >> 6; //right shift to leave top two bits for ease of check.
        } while((hold == 0x03) || (hold == 0x00));
        
        if (hold == 0x01) { //if data is upper 6 bits
        read_reg(0x58, b); //read next set to get lower 2 bits
        uint8_t pixel = a; //set pixel to a
        pixel = pixel << 2; //push left to 7:2
        pixel += (b & mask); //set lower 2 from b to 1:0
        img[count++] = Grayscale(pixel); //put temp value in fbuffer array
        //delayMicroseconds(100);
        }
    }

    write_reg(0x70, 0x00);   //More magic? 
    write_reg(0x58, 0xFF);

    uint8_t temp, check; 

    do { //keep reading and testing
        read_reg(0x58, temp); //read status register
        check = temp>>6; //rightshift 6 bits so only top two stay 
    } while(check == 0x03); //while bits aren't set denoting ready state
}

bool PMW3901::verify(){
    if(!assertReg(PMW3901_REG_Inverse_Product_ID, 0xB6)) return false;
    if(!assertReg(PMW3901_REG_Product_ID, 0x49)) return false;

    return true;
}

void PMW3901::setLed(bool ledOn){
    write_reg(0x7f, 0x14);
    write_reg(0x6f, ledOn ? 0x1c : 0x00);
    write_reg(0x7f, 0x00);
}
void PMW3901::readData(){
    // readDataSlow();
    readDataBurst();
}
void PMW3901::readDataSlow(){
    uint8_t data[5];

    for(uint8_t i = 0; i < 5; i++){
        read_reg(0x02 + i, data[i]);
    }

    motion = data[0];
    dx = (data[2] << 8) | data[1];
    dy = (data[4] << 8) | data[3];
}

void PMW3901::readDataBurst(){
    read_burst(0x16, &motion, 6);
}


scexpr real_t scale = real_t(13.0/2000);
void PMW3901::update(){
    readData();

    x_cm += int16_t(dx) * scale;
    y_cm += int16_t(dy) * scale;
}

void PMW3901::update(const real_t rad){
    readData();
    
    Vector2 delta = Vector2(real_t(int16_t(dx)), real_t(int16_t(dy))).rotated(rad - real_t(PI/2)) * scale;
    x_cm += delta.x;
    y_cm += delta.y;
}


bool PMW3901::assertReg(const uint8_t command, const uint8_t data){
    uint8_t temp = 0;
    read_reg(command, temp);
    // PMW3901_DEBUG(command, data, temp);
    // return PMW3901_ASSERT(temp == data, "reg:", command, "is not equal to", data, "fact is", temp);
    return (temp == data);
}

void PMW3901::init() {
    spi_drv_.release().unwrap();
    write_reg(PMW3901_REG_Power_Up_Reset, 0x5A);
    delay(5);

    PMW3901_ASSERT(verify(), "PMW3901 not found!");
    PMW3901_DEBUG("PMW3901 founded!");
    update();

    write_reg(0x7F, 0x00);
    write_reg(0x61, 0xAD);
    write_reg(0x7F, 0x03);
    write_reg(0x40, 0x00);
    write_reg(0x7F, 0x05);
    write_reg(0x41, 0xB3);
    write_reg(0x43, 0xF1);
    write_reg(0x45, 0x14);
    write_reg(0x5B, 0x32);
    write_reg(0x5F, 0x34);
    write_reg(0x7B, 0x08);
    write_reg(0x7F, 0x06);
    write_reg(0x44, 0x1B);
    write_reg(0x40, 0xBF);
    write_reg(0x4E, 0x3F);
    write_reg(0x7F, 0x08);
    write_reg(0x65, 0x20);
    write_reg(0x6A, 0x18);
    write_reg(0x7F, 0x09);
    write_reg(0x4F, 0xAF);
    write_reg(0x5F, 0x40);
    write_reg(0x48, 0x80);
    write_reg(0x49, 0x80);
    write_reg(0x57, 0x77);
    write_reg(0x60, 0x78);
    write_reg(0x61, 0x78);
    write_reg(0x62, 0x08);
    write_reg(0x63, 0x50);
    write_reg(0x7F, 0x0A);
    write_reg(0x45, 0x60);
    write_reg(0x7F, 0x00);
    write_reg(0x4D, 0x11);
    write_reg(0x55, 0x80);
    write_reg(0x74, 0x1F);
    write_reg(0x75, 0x1F);
    write_reg(0x4A, 0x78);
    write_reg(0x4B, 0x78);
    write_reg(0x44, 0x08);
    write_reg(0x45, 0x50);
    write_reg(0x64, 0xFF);
    write_reg(0x65, 0x1F);
    write_reg(0x7F, 0x14);
    write_reg(0x65, 0x60);
    write_reg(0x66, 0x08);
    write_reg(0x63, 0x78);
    write_reg(0x7F, 0x15);
    write_reg(0x48, 0x58);
    write_reg(0x7F, 0x07);
    write_reg(0x41, 0x0D);
    write_reg(0x43, 0x14);
    write_reg(0x4B, 0x0E);
    write_reg(0x45, 0x0F);
    write_reg(0x44, 0x42);
    write_reg(0x4C, 0x80);
    write_reg(0x7F, 0x10);
    write_reg(0x5B, 0x02);
    write_reg(0x7F, 0x07);
    write_reg(0x40, 0x41);
    write_reg(0x70, 0x00);

    delay(10);
    
    write_reg(0x32, 0x44);
    write_reg(0x7F, 0x07);
    write_reg(0x40, 0x40);
    write_reg(0x7F, 0x06);
    write_reg(0x62, 0xf0);
    write_reg(0x63, 0x00);
    write_reg(0x7F, 0x0D);
    write_reg(0x48, 0xC0);
    write_reg(0x6F, 0xd5);
    write_reg(0x7F, 0x00);
    write_reg(0x5B, 0xa0);
    write_reg(0x4E, 0xA8);
    write_reg(0x5A, 0x50);
    write_reg(0x40, 0x80);

}

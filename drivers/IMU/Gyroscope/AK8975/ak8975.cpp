#include "ak8975.hpp"


void AK8975::init(){
    if(verify() == false) return;
    readAdj();
    update();
}


void AK8975::readAdj(){
    readPool(0x10, &x_adj, 3);
}


void AK8975::update(){
    readPool(0x03, &x, 2 * 3);
}


bool AK8975::verify(){
    // <Self-test Sequence> 
    // (1) Set Power-down mode 
    // (2) Write “1” to SELF bit of ASTC register 
    // (3) Set Self-test Mode 
    // (4) Check Data Ready or not by any of the following method. 
    // - Polling DRDY bit of ST1 register 
    // - Monitor DRDY pin 
    // When Data Ready, proceed to the next step. 
    // (5) Read measurement data (HXL to HZH) 
    // (6) Write “0” to SELF bit of ASTC register

    scexpr uint timeout_ms = 10;

    
    {
        uint8_t id = 0;
        readReg(0x00, id);
        if(id != 0x48) return false;
        //id not correct
    }

    //1
    setMode(Mode::SelfTest);

    //2
    writeReg(0x0c, 0x40);

    //3
    setMode(Mode::PowerDown);

    //4
    auto ms = millis();
    bool readed = false;
    while(millis() - ms < timeout_ms){
        if(isIdle()){
            readed = true;
            break;
        }
    }

    if(readed == false) return false; 

    //5
    update();

    //6
    writeReg(0x0c, 0x00);
    
    return stable();
}

bool AK8975::isIdle(){
    uint8_t stat;
    readReg(0x00, stat);
    return stat;
}

bool AK8975::stable(){
    uint8_t stat;
    readReg(0x09, stat);
    if(stat != 0) return false;
    
    update();
    auto [a,b,c] = getMagnet();
    if(ABS(a) + ABS(b) + ABS(c) > real_t(2.4)) return false;

    return true;
}


void AK8975::setMode(const Mode mode){
    writeReg(0x0A, (uint8_t)mode);
}

void AK8975::disableI2c(){
    writeReg(0x0F, 0x01);
}

std::tuple<real_t, real_t, real_t> AK8975::getMagnet(){
    scexpr real_t max_mT = real_t(1.229);
    #define CONV(n) ((n * max_mT) / 4095) * ((real_t(n##_adj - 128) >> 8) + 1)
    return {CONV(x), CONV(y), CONV(z)};
    #undef CONV
}

void AK8975::writeReg(const uint8_t addr, const uint8_t data){
    if(i2c_drv) i2c_drv->writeReg(addr, data);
    if(spi_drv){
        SpiDrv & drv = spi_drv.value();
        drv.write(uint16_t(uint16_t(addr) << 8 | data));
    }
}
    
void AK8975::readReg(const RegAddress addr, uint8_t & data){
    if(i2c_drv) i2c_drv->readReg((uint8_t)addr, data);
    if(spi_drv){
        SpiDrv & drv = spi_drv.value();
        drv.write(uint8_t(uint8_t(addr) | 0x80), false);
        drv.read(data);
    }
}

void AK8975::readPool(const RegAddress addr, void * datas, const size_t len){
    if(i2c_drv) i2c_drv->readPool(uint8_t(addr), (uint8_t *)datas, len);
    if(spi_drv){
        SpiDrv & drv = spi_drv.value();
        drv.write(uint8_t(uint8_t(addr) | 0x80), false);
        
        drv.read((uint8_t *)(datas), len);
    }
}

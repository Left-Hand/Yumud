#include "ak8975.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define AK8975_DEBUG_EN

#ifdef AK8975_DEBUG_EN
#define AK8975_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define AK8975_PANIC(...) PANIC(__VA_ARGS__)
#define AK8975_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define AK8975_DEBUG(...)
#define AK8975_PANIC(...)  PANIC()
#define AK8975_ASSERT(cond, ...) ASSERT(cond)
#endif



void AK8975::init(){
    if(validate() == false) return;
    readAdj();
    update();
}


void AK8975::readAdj(){
    read_burst(0x10, &x_adj, 3);
}


void AK8975::update(){
    read_burst(0x03, &x, 2 * 3);
}


bool AK8975::validate(){
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
        read_reg(0x00, id);
        if(id != 0x48) return false;
        //id not correct
    }

    //1
    setMode(Mode::SelfTest);

    //2
    write_reg(0x0c, 0x40);

    //3
    setMode(Mode::PowerDown);

    //4
    auto ms = millis();
    bool readed = false;
    while(millis() - ms < timeout_ms){
        if(this->busy() == false){
            readed = true;
            break;
        }
    }

    if(readed == false) return false; 

    //5
    update();

    //6
    write_reg(0x0c, 0x00);
    
    return stable();
}

bool AK8975::busy(){
    uint8_t stat;
    read_reg(0x00, stat);
    return stat == 0;
}

bool AK8975::stable(){
    uint8_t stat;
    read_reg(0x09, stat);
    if(stat != 0) return false;
    
    update();
    const auto mag = get_magnet();
    if(mag.is_none()) return false;

    auto [a, b, c] = get_magnet().unwrap();
    if(ABS(a) + ABS(b) + ABS(c) > real_t(2.4)) return false;

    return true;
}


void AK8975::setMode(const Mode mode){
    write_reg(0x0A, (uint8_t)mode);
}

void AK8975::disableI2c(){
    write_reg(0x0F, 0x01);
}

Option<Vector3_t<real_t>> AK8975::get_magnet(){
    scexpr real_t max_mT = real_t(1.229);
    #define CONV(n) ((n * max_mT) / 4095) * ((real_t(n##_adj - 128) >> 8) + 1)
    return Some{Vector3_t<real_t>{CONV(x), CONV(y), CONV(z)}};
    #undef CONV
}

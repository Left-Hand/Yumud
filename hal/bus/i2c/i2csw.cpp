#include "i2csw.hpp"
#include "core/clock/time_stamp.hpp"
#include "core/debug/debug.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd;
using namespace ymd::hal;

#define I2CSW_SCL_USE_PP_THAN_OD
// #define I2CSW_DISCARD_ACK
// #define I2CSW_TEST_TIMEOUT (1000)


void I2cSw::delay_dur(){
    if(delays_) clock::delay(Microseconds(delays_));
    else for(size_t i = 0; i < 3; i++) __nopn(5);
}

hal::HalResult I2cSw::wait_ack(){

    delay_dur();
    sda().set();
    sda().inpu();
    delay_dur();
    scl().set();
    // TimeStamp delta;

    bool ovt = false;

    #ifndef I2CSW_DISCARD_ACK
    const auto m = clock::micros();
    while(sda().read() == HIGH){
        if(clock::micros() - m >= 
        #if I2CSW_TEST_TIMEOUT
            I2CSW_TEST_TIMEOUT
        #else
            timeout_
        #endif
        ){
            ovt = true;
            break;
        }
        __nopn(4);
    }
    #else
    for(size_t i = 0; i < 3; i++)delay_dur();
    #endif

    delay_dur();
    scl().clr();
    // delay_dur();
    sda().outod();
    
    if(ovt and (discard_ack_ == false)){
        return hal::HalResult::WritePayloadAckTimeout;
    }else{
        return hal::HalResult::Ok();
    }
}

hal::HalResult I2cSw::lead(const LockRequest req){
    #ifdef I2CSW_SCL_USE_PP_THAN_OD
    scl().outpp();
    #else
    scl().outod();
    #endif
    sda().outod();
    sda().set();
    scl().set();
    delay_dur();
    sda().clr();
    delay_dur();
    scl().clr();
    delay_dur();

    constexpr auto header_err_transform = 
    [](const HalResult res) -> HalResult{
        if(res == HalResult::WritePayloadAckTimeout) 
            return HalResult::SlaveAddrAckTimeout;
        return res;
    };

    switch(req.custom_len()){
        case 0:return header_err_transform(write(req.id()));
        case 1:return header_err_transform(write(req.id() << 1 | req.custom()));
        default: break;
    }
    return HalResult::InvalidArgument;
}

void I2cSw::trail(){
    scl().clr();
    sda().outod();
    sda().clr();
    delay_dur();
    scl().set();
    delay_dur();
    sda().set();
    delay_dur();
}



hal::HalResult I2cSw::write(const uint32_t data){
    sda().outod();

    for(uint8_t mask = 0x80; mask; mask >>= 1){
        sda().write(BoolLevel::from(mask & data));
        delay_dur();
        scl().set();
        delay_dur();
        scl().clr();
    }

    return wait_ack();
}

hal::HalResult I2cSw::read(uint32_t & data, const Ack ack){
    uint8_t ret = 0;

    sda().set();
    sda().inpu();
    delay_dur();

    for(uint8_t i = 0; i < 8; i++){
        scl().set();
        ret <<= 1; ret |= sda().read().to_bool();
        delay_dur();
        scl().clr();
        delay_dur();
    }

    sda().write((ack == ACK) ? LOW : HIGH);
    sda().outod();
    scl().set();
    delay_dur();

    scl().clr();
    sda().inpu();

    data = ret;
    return hal::HalResult::Ok();
}

void I2cSw::init(const uint32_t baudrate){

    sda().set();
    sda().outod();
    scl().set();

    #ifdef I2CSW_SCL_USE_PP_THAN_OD
    scl().outpp();
    #else
    scl().outod();
    #endif

    set_baudrate(baudrate);
}

hal::HalResult I2cSw::set_baudrate(const uint32_t baudrate) {
    if(baudrate == 0){
        delays_ = 0;
    }else{
        uint32_t b = baudrate / 1000;
        delays_ = 400 / b;
    }

    return hal::HalResult::Ok();
}

hal::HalResult I2cSw::reset(){
    return hal::HalResult::Ok();
}

hal::HalResult I2cSw::unlock_bus(){
    return hal::HalResult::Ok();
}
#include "soft_i2c.hpp"
#include "core/clock/monotic_clock.hpp"
#include "core/debug/debug.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd;
using namespace ymd::hal;

#define SoftI2c_SCL_USE_PP_THAN_OD
// #define SoftI2c_DISCARD_ACK
// #define SoftI2c_TEST_TIMEOUT (1000)


void SoftI2c::delay_dur(){
    if(delays_) clock::delay(Microseconds(delays_));
    else for(size_t i = 0; i < 3; i++) __nopn(5);
}

HalResult SoftI2c::wait_ack(){

    delay_dur();
    sda().set_high();
    sda().inpu();
    delay_dur();
    scl().set_high();
    // TimeStamp delta;

    bool ovt = false;

    #ifndef SoftI2c_DISCARD_ACK
    const auto m = clock::micros();
    while(sda().read() == HIGH){
        if(clock::micros() - m >= 
        #if SoftI2c_TEST_TIMEOUT
            SoftI2c_TEST_TIMEOUT
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
    scl().set_low();
    // delay_dur();
    sda().outod();
    
    if(ovt and (discard_ack_ == false)){
        return HalResult::WritePayloadAckTimeout;
    }else{
        return HalResult::Ok();
    }
}

HalResult SoftI2c::borrow(const I2cSlaveAddrWithRw req){
    if(false == owner_.is_borrowed()){
        owner_.borrow(req);
        return lead(req);
    }else if(owner_.is_borrowed_by(req)){
        owner_.borrow(req);
        return lead(req);
    }else{
        return HalResult::OccuipedByOther;
    }
}


HalResult SoftI2c::lead(const I2cSlaveAddrWithRw req){
    #ifdef SoftI2c_SCL_USE_PP_THAN_OD
    scl().outpp();
    #else
    scl().outod();
    #endif
    sda().outod();
    sda().set_high();
    scl().set_high();
    delay_dur();
    sda().set_low();
    delay_dur();
    scl().set_low();
    delay_dur();

    constexpr auto header_err_transform = [](const HalResult res) -> HalResult{
        if(res == HalResult::WritePayloadAckTimeout) 
            return HalResult::SlaveAddrAckTimeout;
        return res;
    };

    const uint16_t addr = req.addr_without_rw();
    const bool is_read = req.is_read();


    return header_err_transform(
        write((addr << 1)| static_cast<uint16_t>(is_read))
    );
}

void SoftI2c::trail(){
    scl().set_low();
    sda().outod();
    sda().set_low();
    delay_dur();
    scl().set_high();
    delay_dur();
    sda().set_high();
    delay_dur();
}



HalResult SoftI2c::write(const uint32_t data){
    sda().outod();

    for(uint8_t mask = 0x80; mask; mask >>= 1){
        sda().write(BoolLevel::from(mask & data));
        delay_dur();
        scl().set_high();
        delay_dur();
        scl().set_low();
    }

    return wait_ack();
}

HalResult SoftI2c::read(uint8_t & data, const Ack ack){
    uint8_t ret = 0;

    sda().set_high();
    sda().inpu();
    delay_dur();

    for(uint8_t i = 0; i < 8; i++){
        scl().set_high();
        ret <<= 1; ret |= sda().read().to_bool();
        delay_dur();
        scl().set_low();
        delay_dur();
    }

    sda().write((ack == ACK) ? LOW : HIGH);
    sda().outod();
    scl().set_high();
    delay_dur();

    scl().set_low();
    sda().inpu();

    data = ret;
    return HalResult::Ok();
}

void SoftI2c::init(const Config & cfg){

    sda().set_high();
    sda().outod();
    scl().set_high();

    #ifdef SoftI2c_SCL_USE_PP_THAN_OD
    scl().outpp();
    #else
    scl().outod();
    #endif

    auto release_bus = [&]{
        set_baudrate(hal::NearestFreq{10'000});
        for(size_t i = 0; i < 4; i++){
            lend();
        }
    };

    release_bus();

    
    set_baudrate(cfg.baudrate);
}

HalResult SoftI2c::set_baudrate(const I2cBuadrate baudrate) {
    if(baudrate.is<hal::NearestFreq>()){
        const NearestFreq nearest_freq = baudrate.unwrap_as<hal::NearestFreq>();
        const uint32_t freq_hz = nearest_freq.count;
        if(freq_hz < 1000){
            delays_ = 0;
        }else{
            uint32_t b = freq_hz / 1000;
            delays_ = 400 / b;
        }
    }else{
        __builtin_unreachable();
    }
    return HalResult::Ok();
}

HalResult SoftI2c::reset(){
    return HalResult::Ok();
}

HalResult SoftI2c::unlock_bus(){
    return HalResult::Ok();
}
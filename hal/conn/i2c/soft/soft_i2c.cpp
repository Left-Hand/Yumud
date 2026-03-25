#include "soft_i2c.hpp"
#include "core/clock/monotonic_clock.hpp"
#include "core/debug/debug.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd;
using namespace ymd::hal;

// 对于部分i2c驱动能力弱的外设芯片(LT8960) 使用推挽输出的scl能避免由于人体触摸等问题导致的通信死锁问题
// 但对于另一部分芯片(如BMI160) 这可能会导致通信误码
// #define SOFT_I2C_SCL_USE_PP_THAN_OD


// #define SOFT_I2C_DISCARD_ACK
// #define SOFT_I2C_TEST_TIMEOUT (1000)


void SoftI2c::delay_dur(){
    if(delays_) clock::delay(Microseconds(delays_));
    else for(size_t i = 0; i < 3; i++) __nopn(5);
}

HalResult SoftI2c::wait_ack(){

    delay_dur();
    sda_pin().set_high();
    sda_pin().inpu();
    delay_dur();
    scl_pin().set_high();
    // TimeStamp delta;

    bool ovt = false;

    #ifndef SOFT_I2C_DISCARD_ACK
    const auto m = clock::micros();
    while(sda_pin().read() == HIGH){
        if(clock::micros() - m >= 
        #if SOFT_I2C_TEST_TIMEOUT
            SOFT_I2C_TEST_TIMEOUT
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
    scl_pin().set_low();
    // delay_dur();
    sda_pin().outod();
    
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
    #ifdef SOFT_I2C_SCL_USE_PP_THAN_OD
    scl_pin().outpp();
    #else
    scl_pin().outod();
    #endif
    sda_pin().outod();
    sda_pin().set_high();
    scl_pin().set_high();
    delay_dur();
    sda_pin().set_low();
    delay_dur();
    scl_pin().set_low();
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
    scl_pin().set_low();
    sda_pin().outod();
    sda_pin().set_low();
    delay_dur();
    scl_pin().set_high();
    delay_dur();
    sda_pin().set_high();
    delay_dur();
}



HalResult SoftI2c::write(const uint32_t data){
    sda_pin().outod();

    for(uint8_t mask = 0x80; mask; mask >>= 1){
        sda_pin().write(BoolLevel::from(mask & data));
        delay_dur();
        scl_pin().set_high();
        delay_dur();
        scl_pin().set_low();
    }

    return wait_ack();
}

HalResult SoftI2c::read(uint8_t & data, const Ack ack){
    uint8_t ret = 0;

    sda_pin().set_high();
    sda_pin().inpu();
    delay_dur();

    for(uint8_t i = 0; i < 8; i++){
        scl_pin().set_high();
        ret <<= 1; ret |= sda_pin().read().to_bool();
        delay_dur();
        scl_pin().set_low();
        delay_dur();
    }

    sda_pin().write((ack == ACK) ? LOW : HIGH);
    sda_pin().outod();
    scl_pin().set_high();
    delay_dur();

    scl_pin().set_low();
    sda_pin().inpu();

    data = ret;
    return HalResult::Ok();
}

void SoftI2c::init(const Config & cfg){

    sda_pin().set_high();
    sda_pin().outod();
    scl_pin().set_high();

    #ifdef SOFT_I2C_SCL_USE_PP_THAN_OD
    scl_pin().outpp();
    #else
    scl_pin().outod();
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
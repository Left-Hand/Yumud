#pragma once

#include "i2c.hpp"
#include "core/clock/clock.hpp"


namespace ymd::hal{
class I2cSw final: public I2cBase{
public:

    explicit I2cSw(
        Some<Gpio *> scl,
        Some<Gpio *> sda
    ):
        scl_(scl.deref()),
        sda_(sda.deref()){;}

    I2cSw(I2cSw && other) = default;
    void init(const Config & cfg);
    Gpio & scl() {return scl_;}
    Gpio & sda() {return sda_;}
    HalResult borrow(const I2cSlaveAddrWithRw req);
    HalResult reset();
    HalResult write(const uint32_t data) final;
    HalResult read(uint8_t & data, const Ack ack) final;

    HalResult set_baudrate(const uint32_t baudrate);
    void set_timeout(const Timeout timeout){timeout_ = timeout;}
    void discard_ack(const Enable en){discard_ack_ = en == EN;}
    void lend(){
        this->trail();
        owner_.lend();
    }

private:
    Gpio & scl_;
    Gpio & sda_;

    Timeout timeout_ = Timeout(10);
    uint16_t delays_ = 10;
    bool discard_ack_ = false;

    PeripheralOwnershipTracker owner_ = {};


    __no_inline void delay_dur();

    HalResult wait_ack();
    HalResult lead(const I2cSlaveAddrWithRw req) final;
    void trail();
    HalResult unlock_bus();

};

}
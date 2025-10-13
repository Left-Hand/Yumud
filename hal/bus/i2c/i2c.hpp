#pragma once

#include <chrono>

#include "hal/bus/bus_base.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/sumtype.hpp"
#include "i2c_utils.hpp"

namespace ymd::hal{
class Gpio;

enum class I2cError:uint8_t{

};

enum class I2cMasterEvent:uint8_t{
    // BUSY, MSL and SB flag */
    ModeSelect,
    // BUSY, MSL, ADDR, TXE and TRA flags */
    TransmitterModeSelected,
    // BUSY, MSL and ADDR flags */
    ReceiverModeSelected,
    // BUSY, MSL and ADD10 flags */
    ModeAddress10,
    // BUSY, MSL and RXNE flags */
    ByteReceived,
    // TRA, BUSY, MSL, TXE flags */
    ByteTransmitting,
    // TRA, BUSY, MSL, TXE and BTF flags */
    ByteTransmitted
};

enum class I2cSlaveEvent:uint8_t{
    // BUSY and ADDR flags */
    ReceiverAddressMatched,
    // TRA, BUSY, TXE and ADDR flags */
    TransmitterAddressMatched,
    // DUALF and BUSY flags */
    ReceiverSecondAddressMatched,
    // DUALF, TRA, BUSY and TXE flags */
    TransmitterSecondAddressMatched,
    // GENCALL and BUSY flags */
    GeneralCallAddressMatched,
    // BUSY and RXNE flags */
    ByteReceived,
    // STOPF flag */
    StopDetected,
    // TRA, BUSY, TXE and BTF flags */
    ByteTransmitted,
    // TRA, BUSY and TXE flags */
    ByteTransmitting,
    // AF flag */
    AckFailure
};

struct I2cEvent:public Sumtype<I2cMasterEvent, I2cSlaveEvent>{};

class I2c{
public:
    using Timeout = std::chrono::duration<uint16_t, std::micro>;

    I2c(I2c && other) = default;

    struct Config{
        uint32_t baudrate;
    };

    void set_timeout(const Timeout timeout){timeout_ = timeout;}
    void discard_ack(const Enable en){discard_ack_ = en == EN;}

    virtual hal::HalResult read(uint32_t & data, const Ack ack) = 0;
    virtual hal::HalResult write(const uint32_t data) = 0;
    virtual hal::HalResult unlock_bus() = 0;
    virtual hal::HalResult set_baudrate(const uint32_t baudrate) = 0;

    virtual HalResult lead(const I2cSlaveAddrWithRw req) = 0;
    virtual void trail() = 0;
    HalResult borrow(const I2cSlaveAddrWithRw req);

    void lend(){
        this->trail();
        owner_.lend();
    }

    struct Guard {
        I2c & i2c_;
        Guard(I2c & i2c):i2c_(i2c){}
        ~Guard(){i2c_.lend();}
    };

    auto create_guard(){return Guard(*this);}
protected:

    Timeout timeout_ = Timeout(10);
    bool discard_ack_ = false;

    PeripheralOwnershipTracker owner_ = {};

    I2c() = default;
};


}
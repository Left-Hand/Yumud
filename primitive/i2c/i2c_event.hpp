#pragma once


#include "i2c_addr.hpp"

namespace ymd::hal{
enum class [[nodiscard]] I2cMasterEvent:uint8_t{
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

enum class [[nodiscard]] I2cSlaveEvent:uint8_t{
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

}
#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/sumtype.hpp"
#include "core/stream/ostream.hpp"
#include "core/container/bitarray.hpp"
#include "core/container/inline_vector.hpp"

namespace ymd::modbus{

namespace primitive{

enum class [[nodiscard]] LibError:uint8_t{
    InvalidRequst,
    InvalidUnitId,
    InvalidTcpMbap,
    InvalidCrc,
    Transport,
    Timeout,
    InvalidResponse,
    InvalidArgument,
};

enum class [[nodiscard]] ModbusException:uint8_t{
    IllegalFunction = 1,      /**< Modbus exception 1 */
    IllegalDataAddress = 2,  /**< Modbus exception 2 */
    IllegalDataValue = 3,    /**< Modbus exception 3 */
    ServerDeviceFailure = 4, /**< Modbus exception 4 */
};

enum class [[nodiscard]] FunctionCode:uint8_t{
    None = 0,
    ReadCoils = 1,
    ReadDiscreteInputs = 2,
    ReadHoldingRegisters = 3,
    ReadInputRegisters = 4,
    WriteSingleCoil = 5,
    WriteSingleRegister = 6,
    WriteMultipleCoils = 15,
    WriteMultipleRegisters = 16,
    ReadFileRecord = 20,
    WriteFileRecord = 21,
    ReadWriteMultipleRegisters = 23,
    ReadDeviceIdentification = 43
};

struct [[nodiscard]] ModbusError:public Sumtype<LibError,ModbusException>{
    using enum LibError;
    using enum ModbusException;
    [[nodiscard]] bool is_exception() const{
        return is<ModbusException>();
    }

    [[nodiscard]] ModbusException unwrap_as_exception() const{
        return unwrap_as<ModbusException>();
    }

    friend OutputStream & operator<<(OutputStream & os, const LibError & err);
    friend OutputStream & operator<<(OutputStream & os, const ModbusException & err);
    friend OutputStream & operator<<(OutputStream & os, const ModbusError & err);
    friend OutputStream & operator<<(OutputStream & os, const FunctionCode & fc);
};

static constexpr Err<ModbusError> make_err(auto err){
    return Err<ModbusError>(err);
}

struct [[nodiscard]] Address{
    using Self = Address;

    static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    } 

    [[nodiscard]] constexpr uint8_t to_bits() const {
        return bits_;
    }

    [[nodiscard]] constexpr bool is_boardcast() const{
        return bits_ == 0;
    }

    [[nodiscard]] constexpr bool is_preserved() const {
        return bits_ >= 248;
    }
private:
    uint8_t bits_;
};  


template<typename T = void>
using IResult = Result<T, ModbusError>;


enum class [[nodiscard]] Transport:uint8_t{
    RTU = 1,
    TCP = 2,
};
}
}
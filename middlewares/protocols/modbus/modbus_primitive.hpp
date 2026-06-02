#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/sumtype.hpp"
#include "core/stream/ostream.hpp"
#include "core/container/bitarray.hpp"
#include "core/container/heapless_vector.hpp"

// https://blog.csdn.net/u011041241/article/details/109240555

namespace ymd::modbus{


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

enum class [[nodiscard]] ExceptionCode:uint8_t{
    IllegalFunction = 1,      /**< Modbus exception 1 */
    IllegalDataAddress = 2,  /**< Modbus exception 2 */
    IllegalDataValue = 3,    /**< Modbus exception 3 */
    ServerDeviceFailure = 4, /**< Modbus exception 4 */
    Acknowledge,
    ServerDeviceBusy,
    MemoryParityError,
    GatewayPathUnavailable,
    GatewayTargetDevice,
};


// 0x49~0x77	非法功能
// 0x78~0x7F	保留
// 0x80~0xFF	保留
struct [[nodiscard]] FunctionCode final{

    enum class [[nodiscard]] Kind:uint8_t{
        None = 0,
        ReadCoils = 1,
        ReadDiscreteInputs = 2,
        ReadHoldingRegisters = 3,
        ReadInputRegisters = 4,
        WriteSingleCoil = 5,
        WriteSingleHoldingRegister = 6,
        WriteMultipleCoils = 15,
        WriteMultipleRegisters = 16,
        ReportSlaveId = 17,
        ReadFileRecord = 20,
        WriteFileRecord = 21,
        MaskWriteRegister = 22,
        ReadWriteRegisters = 23,
        ResetSlave = 41,
        ReadDeviceIdentification = 43
    };

    using enum Kind;

    constexpr FunctionCode(const Kind kind): bits(static_cast<uint8_t>(kind)){;}
    constexpr FunctionCode(const _None_t): bits(0){;}

    uint8_t bits;

    constexpr bool is_none() const noexcept {return bits == 0u;}
    constexpr bool is_some() const noexcept {return bits != 0u;}

    constexpr Kind unwrap() const noexcept {
        if(is_none()) 
            __builtin_trap();
        return std::bit_cast<Kind>(bits);
    }

    friend OutputStream & operator<<(OutputStream & os, const Kind kind);

    friend OutputStream & operator<<(OutputStream & os, const FunctionCode & self){
        if(self.is_none()) return os << "None";
        return os << self.unwrap();
    }
};



struct [[nodiscard]] ModbusError:public Sumtype<LibError,ExceptionCode>{
    using enum LibError;
    using enum ExceptionCode;
    [[nodiscard]] bool is_exception() const noexcept {
        return this->is<ExceptionCode>();
    }

    [[nodiscard]] ExceptionCode unwrap_as_exception() const noexcept {
        return this->unwrap_as<ExceptionCode>();
    }

    friend OutputStream & operator<<(OutputStream & os, const LibError & err);
    friend OutputStream & operator<<(OutputStream & os, const ExceptionCode & err);
    friend OutputStream & operator<<(OutputStream & os, const ModbusError & err);

};

static constexpr Err<ModbusError> make_err(auto err){
    return Err<ModbusError>(err);
}

struct [[nodiscard]] NodeId final{
    using Self = NodeId;

    uint8_t count;

    [[nodiscard]] constexpr uint8_t to_u8() const noexcept {
        return count;
    }

    [[nodiscard]] constexpr bool is_boardcast() const noexcept {
        return count == 0;
    }

    [[nodiscard]] constexpr bool is_preserved() const noexcept {
        return count >= 248;
    }

};  


template<typename T = void>
using IResult = Result<T, ModbusError>;


enum class [[nodiscard]] Transport:uint8_t{
    RTU = 1,
    TCP = 2,
};


}
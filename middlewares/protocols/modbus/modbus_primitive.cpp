#include "modbus_primitive.hpp"

using namespace ymd;

namespace ymd::modbus::primitive{

static constexpr const char * liberr_to_str(const LibError err){
    switch(err){
        case LibError::InvalidRequst:
            return "InvalidRequst";
        case LibError::InvalidUnitId:
            return "InvalidUnitId";
        case LibError::InvalidTcpMbap:
            return "InvalidTcpMbap";
        case LibError::InvalidCrc:
            return "InvalidCrc";
        case LibError::Transport:
            return "Transport";
        case LibError::Timeout:
            return "Timeout";
        case LibError::InvalidResponse:
            return "InvalidResponse";
        case LibError::InvalidArgument:
            return "InvalidArgument";
    }
    return nullptr;
}

static constexpr const char * modbus_exception_to_str(const ExceptionCode err){
    switch (err) {
        case ExceptionCode::IllegalFunction:
            return "IllegalFunction";
        case ExceptionCode::IllegalDataAddress:
            return "IllegalDataAddress";
        case ExceptionCode::IllegalDataValue:
            return "IllegalDataValue";
        case ExceptionCode::ServerDeviceFailure:
            return "ServerDeviceFailure";
    }
    return nullptr;
}

__always_inline OutputStream & print_unkown_u8(OutputStream & os, uint8_t int_val){
    return os << "Unknown" << os.brackets<'('>() 
        << std::bit_cast<uint8_t>(int_val) << os.brackets<')'>();
}

OutputStream & operator<<(OutputStream & os, const LibError & err){
    if(const auto str = liberr_to_str(err); str != nullptr) [[likely]]
        return os << str;
    return print_unkown_u8(os, static_cast<uint8_t>(err));
}

OutputStream & operator<<(OutputStream & os, const ExceptionCode & err){
    if(const auto str = modbus_exception_to_str(err); str != nullptr) [[likely]]
        return os << str;
    return print_unkown_u8(os, static_cast<uint8_t>(err));
}

OutputStream & operator<<(OutputStream & os, const ModbusError & err){
    if(err.is_exception()) return os << err.unwrap_as_exception();
    return os << err.unwrap_as<LibError>();
}

OutputStream & operator<<(OutputStream & os, const FunctionCode & fc){
    if(const auto str = function_code_to_str(fc); str != nullptr) [[likely]]
        return os << str;
    return print_unkown_u8(os, static_cast<uint8_t>(fc));
}

}

[[nodiscard]] static constexpr uint16_t crc16(std::span<const uint8_t> bytes) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < bytes.size(); i++) {
        crc ^= (uint16_t) bytes[i];
        for (int j = 8; j != 0; j--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
                crc >>= 1;
        }
    }

    return (uint16_t) (crc << 8) | (uint16_t) (crc >> 8);
}


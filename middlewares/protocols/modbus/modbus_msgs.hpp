#pragma once

#include "modbus_primitive.hpp"

#include "modbus_serialize.hpp"

// 参考资料：
// https://blog.csdn.net/qq_21805743/article/details/120560226


namespace ymd::modbus{

// 0x49~0x77	非法功能
// 0x78~0x7F	保留
// 0x80~0xFF	保留
enum class FunctionCode : uint8_t {
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

namespace req_msg{

// REQ[1] 读取线圈
struct [[nodiscard]] ReadCoils final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadCoils;
    static constexpr size_t CONSTANT_LENGTH = 4;

    //基地址
    uint16_t base_addr;

    //响应数量
    uint16_t quantity; 

    static constexpr size_t context_length(){
        return CONSTANT_LENGTH;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;
        return serialize_u16x2(receiver, self.base_addr, self.quantity);
    }
};

// REQ[2] 读取离散输入
struct [[nodiscard]] ReadDiscreteInputs final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadDiscreteInputs;
    static constexpr size_t CONSTANT_LENGTH = 4;

    //基地址
    uint16_t base_addr;

    //数量
    uint16_t quantity; 


    static constexpr size_t context_length(){
        return CONSTANT_LENGTH;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;
        return serialize_u16x2(receiver, self.base_addr, self.quantity);
    }
};

// REQ[3] 读取保持寄存器
struct [[nodiscard]] ReadHoldingRegisters final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadHoldingRegisters;
    static constexpr size_t CONSTANT_LENGTH = 4;

    //基地址
    uint16_t base_addr;

    //数量
    uint16_t quantity; 

    static constexpr size_t context_length(){
        return CONSTANT_LENGTH;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;
        return serialize_u16x2(
            receiver, 
            self.base_addr, 
            self.quantity
        );
    }
};

// REQ[4] 读取输入寄存器
struct [[nodiscard]] ReadInputRegisters final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadInputRegisters;
    static constexpr size_t CONSTANT_LENGTH = 4;

    //基地址
    uint16_t base_addr;

    //数量(1至125(0x7D))
    uint16_t quantity; 

    static constexpr size_t context_length(){
        return CONSTANT_LENGTH;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;
        return serialize_u16x2(receiver, self.base_addr, self.quantity);
    }
};

// REQ[5] 写入单个线圈
struct [[nodiscard]] WriteSingleCoil final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleCoil;
    static constexpr size_t CONSTANT_LENGTH = 4;

    uint16_t coil_addr;

    //线圈是否开启
    Enable coil_enabled;

    static constexpr size_t context_length(){
        return CONSTANT_LENGTH;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;
        return serialize_u16x2(receiver, 
            self.coil_addr, 
            (self.coil_enabled == EN) ? 0xFF00 : 0x0000
        );
    }
};


// REQ[6] 写单个寄存器
struct [[nodiscard]] WriteSingleHoldingRegister final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleHoldingRegister;
    static constexpr size_t CONSTANT_LENGTH = 4;

    uint16_t reg_addr;
    uint16_t reg_value;

    static constexpr size_t context_length(){
        return CONSTANT_LENGTH;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;
        return serialize_u16x2(receiver, self.reg_addr, self.reg_value);
    }
};


// REQ[0x0f/15] 写入多个线圈
struct [[nodiscard]] WriteMultipleCoils final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleCoils;
    //length not constant

    uint16_t base_addr;
    std::span<const uint8_t> coils_values;

    constexpr size_t context_length() const noexcept {
        return 4 + coils_values.size();
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;

        {
            const uint16_t quantity = static_cast<uint16_t>(coils_values.size());

            const std::array<uint8_t, 4> buffer = {
                static_cast<uint8_t>(self.base_addr >> 8),
                static_cast<uint8_t>(self.base_addr & 0xFF),
                static_cast<uint8_t>(quantity >> 8),
                static_cast<uint8_t>(quantity & 0xFF)
            };

            if(const auto res = receiver.push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(const auto res = receiver.push_bytes(coils_values); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


// REQ[0x10/16] 写入多个寄存器
struct [[nodiscard]] WriteMultipleRegisters final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleRegisters;
    //length not constant

    uint16_t base_addr;
    std::span<const uint16_t> reg_values;

    constexpr size_t context_length() const noexcept {
        return 5 + reg_values.size() * 2;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;

        {
            const uint16_t quantity = static_cast<uint16_t>(reg_values.size());
            const uint8_t num_bytes = static_cast<uint8_t>(quantity * 2);

            const std::array<uint8_t, 5> buffer = {
                static_cast<uint8_t>(self.base_addr >> 8),
                static_cast<uint8_t>(self.base_addr & 0xFF),
                static_cast<uint8_t>(quantity >> 8),
                static_cast<uint8_t>(quantity & 0xFF),
                static_cast<uint8_t>(num_bytes)
            };

            if(const auto res = receiver.push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            for(size_t i = 0; i < reg_values.size(); i++){
                const std::array<uint8_t, 2> buffer = {
                    static_cast<uint8_t>(reg_values[i] >> 8),
                    static_cast<uint8_t>(reg_values[i] & 0xFF)
                };

                if(const auto res = receiver.push_bytes(std::span(buffer)); 
                    res.is_err()) return Err(res.unwrap_err());
            }
        }

        return Ok();
    }
};



// REQ[0x11/17] 报告从机Id
struct [[nodiscard]] ReportSlaveId final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReportSlaveId;
    static constexpr size_t CONSTANT_LENGTH = 0;

    //0长报文不需要序列化方法
};


// REQ[0x16/22] 掩码写寄存器
struct [[nodiscard]] MaskWriteRegister final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::MaskWriteRegister;
    static constexpr size_t CONSTANT_LENGTH = 6;

    uint16_t reg_addr;
    uint16_t and_mask;
    uint16_t or_mask;

    static constexpr size_t context_length(){
        return CONSTANT_LENGTH;
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto& self = *this;

        return serialize_u16_args(receiver, 
            self.reg_addr, 
            self.and_mask, 
            self.or_mask
        );
    }
};

// REQ[0x29/41] 重启指定从机
struct [[nodiscard]] ResetSlave final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ResetSlave;
    static constexpr size_t CONSTANT_LENGTH = 0;

    //0长报文不需要序列化方法
};



}

namespace resp_msg{


// RESP[3] 读保持寄存器
struct ReadHoldingRegisters{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadHoldingRegisters;
    std::span<const uint16_t> reg_values;
};


// RESP[6] 写单个寄存器
// 应答帧与发送帧完全一致，表明写入成功
struct WriteSingleHoldingRegister{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleHoldingRegister;
    uint16_t reg_addr;
    uint16_t reg_value;
};


}

}
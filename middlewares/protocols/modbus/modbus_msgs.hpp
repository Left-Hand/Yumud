#pragma once

#include "modbus_primitive.hpp"

#include "modbus_serialize.hpp"

// 参考资料：
// https://blog.csdn.net/qq_21805743/article/details/120560226


namespace ymd::modbus{

// 0x49~0x77	非法功能
// 0x78~0x7F	保留
// 0x80~0xFF	保留
enum class [[nodiscard]] FunctionCode : uint8_t {
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;

        std::array<uint16_t, 2> buf{
            self.base_addr,
            self.quantity
        };

        return serialize_u16_args(serializer, buf.data(), 2);
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;

        std::array<uint16_t, 2> buf{
            self.base_addr,
            self.quantity
        };

        return serialize_u16_args(serializer, buf.data(), 2);
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;

        std::array<uint16_t, 2> buf{
            self.base_addr,
            self.quantity
        };

        return serialize_u16_args(
            serializer, 
            buf.data(), 2
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;


        std::array<uint16_t, 2> buf{
            self.base_addr,
            self.quantity
        };

        return serialize_u16_args(
            serializer, buf.data(), 2);
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;

        std::array<uint16_t, 2> buf = {
            self.coil_addr,
            (self.coil_enabled == EN) ? uint16_t(0xFF00) : uint16_t(0x0000)
        };

        return serialize_u16_args(
            serializer, 
            buf.data(), 2
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;


        std::array<uint16_t, 2> buf{
            self.reg_addr,
            self.reg_value
        };

        return serialize_u16_args(serializer, buf.data(), 2);
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;

        {
            const uint16_t quantity = static_cast<uint16_t>(coils_values.size());

            const std::array<uint8_t, 4> buffer = {
                static_cast<uint8_t>(self.base_addr >> 8),
                static_cast<uint8_t>(self.base_addr & 0xFF),
                static_cast<uint8_t>(quantity >> 8),
                static_cast<uint8_t>(quantity & 0xFF)
            };

            if(const auto res = serializer.push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(const auto res = serializer.push_bytes(coils_values); 
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
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

            if(const auto res = serializer.push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            for(size_t i = 0; i < reg_values.size(); i++){
                const std::array<uint8_t, 2> buffer = {
                    static_cast<uint8_t>(reg_values[i] >> 8),
                    static_cast<uint8_t>(reg_values[i] & 0xFF)
                };

                if(const auto res = serializer.push_bytes(std::span(buffer)); 
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

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto& self = *this;


        std::array<uint16_t, 3> buf{
            self.reg_addr,
            self.and_mask,
            self.or_mask
        };

        return serialize_u16_args(serializer, buf.data(), 3);
    }
};


// REQ[0x17/23] 读写多个寄存器
struct [[nodiscard]] ReadWriteRegisters final{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadWriteRegisters;
    //length not constant

    uint16_t read_start_addr;      // 读起始地址高位/低位
    uint16_t read_quantity;        // 读数量高位/低位
    uint16_t write_start_addr;     // 写起始地址高位/低位
    uint16_t write_quantity;       // 写数量高位/低位
    std::span<const uint16_t> write_reg_values; // 写入的寄存器值

    constexpr size_t context_length() const noexcept {
        return 9 + write_reg_values.size() * 2;
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept {
        auto & self = *this;

        // 首先发送读参数（起始地址和数量）
        {
            const std::array<uint8_t, 8> read_params = {
                static_cast<uint8_t>(self.read_start_addr >> 8),
                static_cast<uint8_t>(self.read_start_addr & 0xFF),
                static_cast<uint8_t>(self.read_quantity >> 8),
                static_cast<uint8_t>(self.read_quantity & 0xFF),
                static_cast<uint8_t>(self.write_start_addr >> 8),
                static_cast<uint8_t>(self.write_start_addr & 0xFF),
                static_cast<uint8_t>(self.write_quantity >> 8),
                static_cast<uint8_t>(self.write_quantity & 0xFF)
            };

            if(const auto res = serializer.push_bytes(std::span(read_params)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        // 然后发送写字节数
        {
            const uint8_t write_byte_count = static_cast<uint8_t>(self.write_quantity * 2);
            const std::array<uint8_t, 1> byte_count = {write_byte_count};

            if(const auto res = serializer.push_bytes(std::span(byte_count)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        // 最后发送写入的寄存器数据
        {
            for(size_t i = 0; i < self.write_reg_values.size(); i++){
                const std::array<uint8_t, 2> buffer = {
                    static_cast<uint8_t>(self.write_reg_values[i] >> 8),
                    static_cast<uint8_t>(self.write_reg_values[i] & 0xFF)
                };

                if(const auto res = serializer.push_bytes(std::span(buffer)); 
                    res.is_err()) return Err(res.unwrap_err());
            }
        }

        return Ok();
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


// RESP[1] 读取线圈
struct ReadCoils{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadCoils;

    std::span<const uint8_t> coil_values; // 按字节打包的线圈状态
};


// RESP[2] 读取离散输入
struct ReadDiscreteInputs{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadDiscreteInputs;

    std::span<const uint8_t> discrete_input_values; // 按字节打包的离散输入状态
};


// RESP[3] 读保持寄存器
struct ReadHoldingRegisters{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadHoldingRegisters;

    std::span<const uint16_t> reg_values;
};


// RESP[4] 读输入寄存器
struct ReadInputRegisters{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadInputRegisters;

    std::span<const uint16_t> reg_values;
};


// RESP[5] 写单个线圈
// 应答帧与发送帧完全一致，表明写入成功
struct WriteSingleCoil{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleCoil;

    uint16_t coil_addr;
    uint16_t coil_value;  // 0xFF00 或 0x0000
};


// RESP[6] 写单个寄存器
// 应答帧与发送帧完全一致，表明写入成功
struct WriteSingleHoldingRegister{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleHoldingRegister;

    uint16_t reg_addr;
    uint16_t reg_value;
};


// RESP[0x0f/15] 写多个线圈
// 应答帧返回写入的起始地址和线圈数量
struct WriteMultipleCoils{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleCoils;

    uint16_t start_addr;
    uint16_t quantity;
};


// RESP[0x10/16] 写多个寄存器
// 应答帧返回写入的起始地址和寄存器数量
struct WriteMultipleRegisters{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleRegisters;

    uint16_t start_addr;
    uint16_t quantity;
};


// RESP[0x11/17] 报告从机Id
struct ReportSlaveId{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReportSlaveId;

    uint8_t byte_count;
    std::span<const uint8_t> slave_id_data;  // 包含运行状态、厂商ID、设备型号等信息
};


// RESP[0x16/22] 掩码写寄存器
// 应答帧与发送帧完全一致，表明写入成功
struct MaskWriteRegister{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::MaskWriteRegister;

    uint16_t reg_addr;
    uint16_t and_mask;
    uint16_t or_mask;
};


// RESP[0x17/23] 读写多个寄存器
struct ReadWriteRegisters{
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadWriteRegisters;

    std::span<const uint16_t> read_reg_values;  // 读取的寄存器值
};

}

}

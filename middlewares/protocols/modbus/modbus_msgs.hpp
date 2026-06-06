#pragma once

#include "modbus_primitive.hpp"
#include "modbus_serialize.hpp"
#include "core/utils/marco_utils.hpp"

// 参考资料：
// https://blog.csdn.net/qq_21805743/article/details/120560226




#define DEF_MODBUS_NOFIELDS\
    static constexpr size_t CONSTANT_LENGTH = 0;\
    [[nodiscard]] static consteval size_t context_length(){return 0;}\
    template<typename Serializer>\
    constexpr Result<void, typename Serializer::Error> \
    serialize(Serializer & srz) const noexcept {return Ok();}\



#define DEF_MODBUS_U16FIELDS_SERIALZE_FN(...)\
    constexpr void fill_bytes_unchecked(uint8_t * pbuf) const {\
        u16be_args_fill_bytes<PP_NARG(__VA_ARGS__)>(pbuf, {__VA_ARGS__});\
    }\
    template<typename Serializer>\
    constexpr Result<void, typename Serializer::Error> \
    serialize(Serializer & srz) const noexcept {\
        static constexpr size_t buf_len = CONSTANT_LENGTH;\
        if(const auto res = srz.compatible_with_length(buf_len);\
            res.is_err()) return Err(res.unwrap_err());\
        const auto allocated_ptr = srz.take_cursor_and_inc(buf_len);\
        fill_bytes_unchecked(allocated_ptr);\
        return Ok();\
    }\

#define DEF_MODBUS_DESERIALIZE_PATTERN\
    template<typename Deserializer>\
    [[nodiscard]] static constexpr Result<Self, typename Deserializer::Error> \
    deserialize_from(Deserializer & dsrz){\
        const size_t len = context_length();\
        if(const auto res = dsrz.compatible_with_length(len);\
            res.is_err()) return Err(res.unwrap_err());\
        return Ok(from_buf_unchecked(dsrz.take_cursor_and_inc(len)));\
    }\

#define DEF_MODBUS_U16FIELDS_DESERIALZE_FN(...)\
    constexpr void set_by_buf_unchecked(__restrict const uint8_t * buf){\
        bytes_fill_u16be_args(buf, __VA_ARGS__);\
    }\

#define DEF_MODBUS_SERDE_U16FIELDS(...)\
    static constexpr size_t CONSTANT_LENGTH = 2 * PP_NARG(__VA_ARGS__);\
    [[nodiscard]] static consteval size_t context_length(){return CONSTANT_LENGTH;}\
    static constexpr Self from_buf_unchecked(__restrict const uint8_t * buf){\
        Self self; self.set_by_buf_unchecked(buf); return self;}\
    DEF_MODBUS_U16FIELDS_SERIALZE_FN(__VA_ARGS__)\
    DEF_MODBUS_DESERIALIZE_PATTERN\
    DEF_MODBUS_U16FIELDS_DESERIALZE_FN(__VA_ARGS__)\

namespace ymd::modbus{

template<size_t N>
static constexpr void u16be_args_fill_bytes(__restrict uint8_t * buf, std::array<uint16_t, N> args){
    for(size_t i = 0; i < N; i++){
        buf[0] = uint8_t(args[i] >> 8);
        buf[1] = uint8_t(args[i]);
        buf += 2;
    }
}

template<typename ... Args>
static constexpr void bytes_fill_u16be_args(__restrict const uint8_t * buf, Args & ... args){
    size_t offset = 0;
    ((args = (static_cast<Args>(buf[offset]) << 8) | static_cast<Args>(buf[offset + 1]),
        offset += 2), ...);
}

struct [[nodiscard]] UncheckedBytesSpawner final{
    const uint8_t * buf;

    constexpr uint16_t spawn_u16be() {
        uint16_t ret = 0;
        ret |= (buf[0] << 8);
        ret |= buf[1];
        buf += 2;
        return ret;
    }

    constexpr uint8_t spawn_u8() {
        uint8_t ret = buf[0];
        buf += 1;
        return ret;
    }

    constexpr std::span<const uint8_t> spawn_bytes(const size_t n) {
        auto ret = std::span<const uint8_t>{buf, n};
        buf += n;
        return ret;
    }
};

namespace req_msgs{

// REQ[1] 读取线圈
struct [[nodiscard]] ReadCoils final{
    using Self = ReadCoils;

    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadCoils;


    //基地址
    uint16_t base_address;

    //响应数量
    uint16_t quantity; 


    DEF_MODBUS_SERDE_U16FIELDS(base_address, quantity)
};

// REQ[2] 读取离散输入
struct [[nodiscard]] ReadDiscreteInputs final{
    using Self = ReadDiscreteInputs;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadDiscreteInputs;

    //基地址
    uint16_t base_address;

    //数量
    uint16_t quantity; 


    DEF_MODBUS_SERDE_U16FIELDS(base_address, quantity)
};

// REQ[3] 读取保持寄存器
struct [[nodiscard]] ReadHoldingRegisters final{
    using Self = ReadHoldingRegisters;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadHoldingRegisters;

    //基地址
    uint16_t base_address;

    //数量
    uint16_t quantity; 

    DEF_MODBUS_SERDE_U16FIELDS(base_address, quantity)
};

// REQ[4] 读取输入寄存器
struct [[nodiscard]] ReadInputRegisters final{
    using Self = ReadInputRegisters;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadInputRegisters;

    //基地址
    uint16_t base_address;

    //数量(1至125(0x7D))
    uint16_t quantity; 

    DEF_MODBUS_SERDE_U16FIELDS(base_address, quantity)
};

// REQ[5] 写入单个线圈
struct [[nodiscard]] WriteSingleCoil final{
    using Self = WriteSingleCoil;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleCoil;
    static constexpr size_t CONSTANT_LENGTH = 4;

    uint16_t coil_addr;

    //线圈是否开启
    Enable coil_enabled;

    [[nodiscard]] static consteval size_t context_length(){
        return CONSTANT_LENGTH;
    }


    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        std::array<uint16_t, 2> buf = {
            self.coil_addr,
            (self.coil_enabled == EN) ? uint16_t(0xFF00) : uint16_t(0x0000)
        };

        return serialize_u16_args(
            srz, 
            buf.data(), 2
        );
    }

    constexpr void set_by_buf_unchecked(__restrict const uint8_t * buf){
        auto & self = *this;
        auto spn = UncheckedBytesSpawner{buf};
        self.coil_addr = spn.spawn_u16be();
        self.coil_enabled = spn.spawn_u8() ? EN : DISEN;
    }


    DEF_MODBUS_DESERIALIZE_PATTERN
};


// REQ[6] 写单个寄存器
struct [[nodiscard]] WriteSingleHoldingRegister final{
    using Self = WriteSingleHoldingRegister;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleHoldingRegister;

    uint16_t reg_address;
    uint16_t reg_value;

    DEF_MODBUS_SERDE_U16FIELDS(reg_address, reg_value)
};


// REQ[0x0f/15] 写入多个线圈
struct [[nodiscard]] WriteMultipleCoils final{
    using Self = WriteMultipleCoils;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleCoils;
    //length not constant

    uint16_t base_address;
    std::span<const uint8_t> coils_values;

    constexpr size_t context_length() const noexcept {
        return 4 + coils_values.size();
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        {
            const uint16_t quantity = static_cast<uint16_t>(coils_values.size());

            const std::array<uint8_t, 4> buffer = {
                static_cast<uint8_t>(self.base_address >> 8),
                static_cast<uint8_t>(self.base_address & 0xFF),
                static_cast<uint8_t>(quantity >> 8),
                static_cast<uint8_t>(quantity & 0xFF)
            };

            if(const auto res = srz.push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(const auto res = srz.push_bytes(coils_values); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }

    template<typename Deserializer>
    [[nodiscard]] static constexpr Result<Self, typename Deserializer::Error> 
    deserialize_from(Deserializer & dsrz){
        if(const auto res = dsrz.compatible_with_length(4);
            res.is_err()) return Err(res.unwrap_err());
        
        uint16_t base_address;
        uint16_t quantity;
        bytes_fill_u16be_args(dsrz.take_cursor_and_inc(4), base_address, quantity);

        if(const auto res = dsrz.compatible_with_length(quantity);
            res.is_err()) return Err(res.unwrap_err());
        
        return Ok(Self{
            .base_address = base_address,
            .coils_values = std::span(dsrz.take_cursor_and_inc(quantity), quantity)
        });
    }
};


// REQ[0x10/16] 写入多个寄存器
struct [[nodiscard]] WriteMultipleRegisters final{
    using Self = WriteMultipleRegisters;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleRegisters;
    //length not constant

    uint16_t base_address;
    std::span<const uint16_t> reg_values;

    constexpr size_t context_length() const noexcept {
        return 5 + (reg_values.size() * 2);
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        {
            const uint16_t quantity = static_cast<uint16_t>(reg_values.size());
            const uint8_t num_bytes = static_cast<uint8_t>(quantity * 2);

            const std::array<uint8_t, 5> buffer = {
                static_cast<uint8_t>(self.base_address >> 8),
                static_cast<uint8_t>(self.base_address & 0xFF),
                static_cast<uint8_t>(quantity >> 8),
                static_cast<uint8_t>(quantity & 0xFF),
                static_cast<uint8_t>(num_bytes)
            };

            if(const auto res = srz.push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        if(const auto res = serialize_u16_args(srz, reg_values.data(), reg_values.size());
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }


};



// REQ[0x11/17] 报告从机Id
struct [[nodiscard]] ReportSlaveId final{
    using Self = ReportSlaveId;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReportSlaveId;

    DEF_MODBUS_NOFIELDS
};


// REQ[0x16/22] 掩码写寄存器
struct [[nodiscard]] MaskWriteRegister final{
    using Self = MaskWriteRegister;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::MaskWriteRegister;

    uint16_t reg_address;
    uint16_t and_mask;
    uint16_t or_mask;

    DEF_MODBUS_SERDE_U16FIELDS(reg_address, and_mask, or_mask)
};


// REQ[0x17/23] 读写多个寄存器
struct [[nodiscard]] ReadWriteRegisters final{
    using Self = ReadWriteRegisters;
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
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        // 首先发送读参数（起始地址和数量）
        {
            const std::array<uint8_t, 9> buf = {
                static_cast<uint8_t>(self.read_start_addr >> 8),
                static_cast<uint8_t>(self.read_start_addr & 0xFF),
                static_cast<uint8_t>(self.read_quantity >> 8),
                static_cast<uint8_t>(self.read_quantity & 0xFF),
                static_cast<uint8_t>(self.write_start_addr >> 8),
                static_cast<uint8_t>(self.write_start_addr & 0xFF),
                static_cast<uint8_t>(self.write_quantity >> 8),
                static_cast<uint8_t>(self.write_quantity & 0xFF),
                static_cast<uint8_t>(self.write_quantity * 2)
            };

            if(const auto res = srz.push_bytes(std::span(buf)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        if(const auto res = serialize_u16_args(srz, write_reg_values.data(), write_reg_values.size());
            res.is_err()) return Err(res.unwrap_err());


        return Ok();
    }
};



// REQ[0x29/41] 重启指定从机
struct [[nodiscard]] ResetSlave final{
    using Self = ResetSlave;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ResetSlave;

    DEF_MODBUS_NOFIELDS
};


}

namespace resp_msgs{


// RESP[1] 读取线圈
struct [[nodiscard]] ReadCoils final{
    using Self = ReadCoils;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadCoils;

    std::span<const uint8_t> coil_values; // 按字节打包的线圈状态

    constexpr size_t context_length() const noexcept {
        return 1 + coil_values.size();  // 字节计数 + 数据字节数
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        // 写入字节计数
        const uint8_t byte_count = static_cast<uint8_t>(self.coil_values.size());
        if(const auto res = srz.push_bytes(std::span(&byte_count, 1)); 
            res.is_err()) return Err(res.unwrap_err());

        // 写入线圈值
        if(const auto res = srz.push_bytes(self.coil_values); 
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }
};


// RESP[2] 读取离散输入
struct [[nodiscard]] ReadDiscreteInputs final{
    using Self = ReadDiscreteInputs;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadDiscreteInputs;

    std::span<const uint8_t> discrete_input_values; // 按字节打包的离散输入状态

    constexpr size_t context_length() const noexcept {
        return 1 + discrete_input_values.size();  // 字节计数 + 数据字节数
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        // 写入字节计数
        const uint8_t byte_count = static_cast<uint8_t>(self.discrete_input_values.size());
        if(const auto res = srz.push_bytes(std::span(&byte_count, 1)); 
            res.is_err()) return Err(res.unwrap_err());

        // 写入离散输入值
        if(const auto res = srz.push_bytes(self.discrete_input_values); 
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }


    template<typename Deserializer>
    [[nodiscard]] static constexpr Result<Self, typename Deserializer::Error> 
    deserialize_from(Deserializer & dsrz){
        if(const auto res = dsrz.compatible_with_length(1);
            res.is_err()) return Err(res.unwrap_err());
        
        uint16_t num_byte = *dsrz.take_cursor_and_inc(1);

        if(const auto res = dsrz.compatible_with_length(num_byte);
            res.is_err()) return Err(res.unwrap_err());
        
        return Ok(Self{
            .discrete_input_values = std::span(dsrz.take_cursor_and_inc(num_byte), num_byte)
        });
    }
};


// RESP[3] 读保持寄存器
struct [[nodiscard]] ReadHoldingRegisters final{
    using Self = ReadHoldingRegisters;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadHoldingRegisters;

    std::span<const uint16_t> reg_values;

    constexpr size_t context_length() const noexcept {
        return 1 + reg_values.size() * 2;  // 字节计数 + 数据字节数
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        // 写入字节计数
        const uint8_t byte_count = static_cast<uint8_t>(self.reg_values.size() * 2);
        if(const auto res = srz.push_bytes(std::span(&byte_count, 1)); 
            res.is_err()) return Err(res.unwrap_err());

        // 写入寄存器值
        if(const auto res = serialize_u16_args(
            srz, self.reg_values.data(), self.reg_values.size()
        ); res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }
};


// RESP[4] 读输入寄存器
struct [[nodiscard]] ReadInputRegisters final{
    using Self = ReadInputRegisters;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadInputRegisters;

    std::span<const uint16_t> reg_values;

    constexpr size_t context_length() const noexcept {
        return 1 + reg_values.size() * 2;  // 字节计数 + 数据字节数
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        // 写入字节计数
        const uint8_t byte_count = static_cast<uint8_t>(self.reg_values.size() * 2);
        if(const auto res = srz.push_bytes(std::span(&byte_count, 1)); 
            res.is_err()) return Err(res.unwrap_err());

        // 写入寄存器值
        if(const auto res = serialize_u16_args(
            srz, self.reg_values.data(), self.reg_values.size()
        ); res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }
};


// RESP[5] 写单个线圈
// 应答帧与发送帧完全一致，表明写入成功
struct [[nodiscard]] WriteSingleCoil final{
    using Self = WriteSingleCoil;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleCoil;

    uint16_t coil_addr;
    uint16_t coil_value;  // 0xFF00 或 0x0000

    DEF_MODBUS_SERDE_U16FIELDS(coil_addr, coil_value)
};


// RESP[6] 写单个寄存器
// 应答帧与发送帧完全一致，表明写入成功
struct [[nodiscard]] WriteSingleHoldingRegister final{
    using Self = WriteSingleHoldingRegister;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteSingleHoldingRegister;

    uint16_t reg_address;
    uint16_t reg_value;

    DEF_MODBUS_SERDE_U16FIELDS(reg_address, reg_value)
};


// RESP[0x0f/15] 写多个线圈
// 应答帧返回写入的起始地址和线圈数量
struct [[nodiscard]] WriteMultipleCoils final{
    using Self = WriteMultipleCoils;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleCoils;

    uint16_t base_address;
    uint16_t quantity;

    DEF_MODBUS_SERDE_U16FIELDS(base_address, quantity)
};


// RESP[0x10/16] 写多个寄存器
// 应答帧返回写入的起始地址和寄存器数量
struct [[nodiscard]] WriteMultipleRegisters final{
    using Self = WriteMultipleRegisters;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::WriteMultipleRegisters;

    uint16_t base_address;
    uint16_t quantity;

    DEF_MODBUS_SERDE_U16FIELDS(base_address, quantity)
};


// RESP[0x11/17] 报告从机Id
struct [[nodiscard]] ReportSlaveId final{
    using Self = ReportSlaveId;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReportSlaveId;

    std::span<const uint8_t> slave_id_data;  // 包含运行状态、厂商ID、设备型号等信息

    constexpr size_t context_length() const noexcept {
        return slave_id_data.size();  // 字节计数 + 数据字节数
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;
        return srz.push_bytes(self.slave_id_data); 
    }


    template<typename Deserializer>
    [[nodiscard]] static constexpr Result<Self, typename Deserializer::Error> 
    deserialize_from(Deserializer & dsrz){
        if(const auto res = dsrz.compatible_with_length(1);
            res.is_err()) return Err(res.unwrap_err());
        
        uint16_t num_byte = *dsrz.take_cursor_and_inc(1);

        if(const auto res = dsrz.compatible_with_length(num_byte);
            res.is_err()) return Err(res.unwrap_err());
        
        return Ok(Self{
            .slave_id_data = std::span(dsrz.take_cursor_and_inc(num_byte), num_byte)
        });
    }

};


// RESP[0x16/22] 掩码写寄存器
// 应答帧与发送帧完全一致，表明写入成功
struct [[nodiscard]] MaskWriteRegister final{
    using Self = MaskWriteRegister;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::MaskWriteRegister;

    uint16_t reg_address;
    uint16_t and_mask;
    uint16_t or_mask;

    DEF_MODBUS_SERDE_U16FIELDS(reg_address, and_mask, or_mask)
};


// RESP[0x17/23] 读写多个寄存器
struct [[nodiscard]] ReadWriteRegisters final{
    using Self = ReadWriteRegisters;
    static constexpr FunctionCode FUNC_CODE = FunctionCode::ReadWriteRegisters;

    std::span<const uint16_t> read_reg_values;  // 读取的寄存器值

    constexpr size_t context_length() const noexcept {
        return 1 + read_reg_values.size() * 2;  // 字节计数 + 数据字节数
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const noexcept {
        auto & self = *this;

        // 写入字节计数
        const uint8_t byte_count = static_cast<uint8_t>(self.read_reg_values.size() * 2);
        if(const auto res = srz.push_bytes(std::span(&byte_count, 1)); 
            res.is_err()) return Err(res.unwrap_err());

        // 写入读取的寄存器值
        if(const auto res = serialize_u16_args(
            srz, self.read_reg_values.data(), self.read_reg_values.size()
        ); res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }
};

}

}

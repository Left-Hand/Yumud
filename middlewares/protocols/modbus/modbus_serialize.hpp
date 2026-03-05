#include "modbus_primitive.hpp"
#include "modbus_crc.hpp"


namespace ymd::modbus{

template<typename Receiver, size_t N>
static constexpr Result<void, typename Receiver::Error> serialize_u16_args(
    Receiver & receiver, 
    const std::array<uint16_t, N> && args
) {
    for(size_t i = 0; i < N; i++){
        const std::array<uint8_t, 2> buffer = {
            static_cast<uint8_t>(args[i] >> 8),
            static_cast<uint8_t>(args[i] & 0xFF)
        };

        if(const auto res = receiver.push_bytes(buffer); 
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

template<typename Receiver>
static constexpr Result<void, typename Receiver::Error> serialize_u16x2(
    Receiver & receiver, 
    const uint16_t arg1, const uint16_t arg2
){
    return serialize_u16_args<Receiver, 2>(receiver, {arg1, arg2});
}

template<typename Receiver, typename Request>
static constexpr Result<void, typename Receiver::Error> serialize_rtu_request(
    Receiver & receiver,
    const uint8_t node_id,
    const Request & request
){

    //header
    {
        const std::array<uint8_t, 2> buffer = {
            static_cast<uint8_t>(node_id),
            static_cast<uint8_t>(Request::FUNC_CODE),
        };

        if(const auto res = receiver.push_bytes(buffer); 
            res.is_err()) return Err(res.unwrap_err());
    }

    //context
    {
        if(const auto res = request.serialize_context(receiver); 
            res.is_err()) return Err(res.unwrap_err());
    }

    //tail crc

    {
        //crc字段为小端序
        const uint16_t crc =  Crc16ModbusAccumulator{}
            .push_bytes(receiver.collected_bytes())
            .finalize()
        ;

        const std::array<uint8_t, 2> buffer = {
            static_cast<uint8_t>(crc & 0xff),
            static_cast<uint8_t>(crc >> 8)
        };

        if(const auto res = receiver.push_bytes(buffer); 
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

template<typename Receiver, typename Request>
static constexpr Result<void, typename Receiver::Error> serialize_tcp_request(
    Receiver& receiver,
    const uint16_t transaction_id,  // 事务ID，客户端自增
    const uint8_t unit_id,          // 对应RTU的站地址
    const Request& request
) {
    //  预计算PDU长度 
    // PDU长度 = 功能码(1B) + 数据段长度(Request::context_length())
    const uint16_t pdu_data_len = request.context_length();
    const uint16_t pdu_total_len = 1 + pdu_data_len;
    // 长度字段不能超过 Modbus-TCP 限制（理论最大 253）

    if (pdu_total_len > 253) {
        return Err(Receiver::Error::kLengthExceed);
        // return Err(typename Receiver::Error{});
    }

    //  组装MBAP报文头 
    const std::array<uint8_t, 7> mbap_header = {
        // 事务标识符 - 大端序
        static_cast<uint8_t>((transaction_id >> 8) & 0xFF),
        static_cast<uint8_t>(transaction_id & 0xFF),
        // 协议标识符 - 固定0x0000
        0x00,
        0x00,
        // 长度字段 - 大端序（后续字节总长度）
        static_cast<uint8_t>((pdu_total_len >> 8) & 0xFF),
        static_cast<uint8_t>(pdu_total_len & 0xFF),
        // 单元标识符
        unit_id
    };

    // 写入MBAP头
    if (const auto res = receiver.push_bytes(mbap_header); res.is_err()) {
        return Err(res.unwrap_err());
    }

    //  写入功能码 
    const std::array<uint8_t, 1> func_code = {
        static_cast<uint8_t>(Request::FUNC_CODE)
    };

    if (const auto res = receiver.push_bytes(func_code); res.is_err()) {
        return Err(res.unwrap_err());
    }

    //  写入PDU数据段 
    if (const auto res = request.serialize_context(receiver); res.is_err()) {
        return Err(res.unwrap_err());
    }

    return Ok();
}
}
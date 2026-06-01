#include "modbus_primitive.hpp"
#include "modbus_crc.hpp"


namespace ymd::modbus{

template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> serialize_u16_args(
    Serializer & serializer, 
    __restrict const uint16_t * ptr, 
    const size_t len 
) {
    if(ptr == nullptr) return Ok();

    for(size_t i = 0; i < len; i++){
        const std::array<uint8_t, 2> buf = {
            static_cast<uint8_t>(ptr[i] >> 8),
            static_cast<uint8_t>(ptr[i] & 0xFF)
        };

        if(const auto res = serializer.push_bytes(buf); 
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}



template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> serialize_rtu_header(
    Serializer & serializer,
    const uint8_t node_id,
    const uint8_t func_code
){
    const std::array<uint8_t, 2> buf = {
        static_cast<uint8_t>(node_id),
        func_code
    };

    if(const auto res = serializer.push_bytes(buf); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_rtu_tailer(
    Serializer & serializer
) requires requires(Serializer& s) {
    { s.collected_bytes() } -> std::convertible_to<std::span<const uint8_t>>;
    { s.push_bytes(std::declval<std::span<const uint8_t>>()) } -> std::same_as<Result<void, typename Serializer::Error>>;
}{
    //crc字段为小端序
    const uint16_t checksum =  ChecksumBuilder::from_default()
        .push_bytes(serializer.collected_bytes())
        .finalize()
    ;

    const std::array<uint8_t, 2> buf = {
        static_cast<uint8_t>(checksum & 0xff),
        static_cast<uint8_t>(checksum >> 8)
    };

    if(const auto res = serializer.push_bytes(buf); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

template<typename Serializer, typename Msg>
static constexpr Result<void, typename Serializer::Error> 
serialize_msg_context(
    Serializer & serializer,
    const Msg & msg
){
    #if 1
    // context
    if(const auto res = msg.serialize_context(serializer); 
        res.is_err()) return Err(res.unwrap_err());
    #else

    // 有序列化方法/有常量长度且非0
    if constexpr(requires { msg.serialize_context(); }){
        // context
        if(const auto res = msg.serialize_context(serializer); 
            res.is_err()) return Err(res.unwrap_err());
    }

    if constexpr(requires { Msg::CONSTANT_LENGTH; } ){   
        if constexpr(Msg::CONSTANT_LENGTH != 0){
            // context
            if(const auto res = msg.serialize_context(serializer); 
                res.is_err()) return Err(res.unwrap_err());
        }
    }
    #endif
    return Ok();
}


template<typename Serializer, typename Msg>
static constexpr Result<void, typename Serializer::Error> 
serialize_rtu_msg(
    Serializer & serializer,
    const Msg & msg,
    const uint8_t node_id
){
    if(const auto res = serialize_rtu_header(
        serializer, node_id, uint8_t(Msg::FUNC_CODE)
    ); res.is_err()) return Err(res.unwrap_err());


    if(const auto res = serialize_msg_context(
        serializer, msg
    ); res.is_err()) return Err(res.unwrap_err());

    if(const auto res = serialize_rtu_tailer(serializer);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}




#if 0
struct [[nodiscard]] TcpHeader final{

    struct Parts{
        uint16_t transaction_id;
        uint8_t unit_id;
        uint8_t context_length;
        uint8_t func_code;
    };


    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> serialize(
        Serializer & serializer
    ) const {

    }
};
#endif



template<typename Serializer, typename Msg>
static constexpr Result<void, typename Serializer::Error> serialize_tcp_msg(
    Serializer& serializer,
    const Msg& msg,

    //  这是MODBUS服务器地址，通常为01，但如果网络上有多台服务器，则可能不同。
    const uint8_t unit_id,

    // 这是一个随机或递增的值，用于匹配请求和响应。在这里，我们使用0000作为示例，但在实际应用中，应该使用一个唯一的值。
    const uint16_t transaction_id
) {
    {
        //  预计算PDU长度 
        const uint16_t mbap_length = 2 + msg.context_length();
        const uint8_t func_code = uint8_t(Msg::FUNC_CODE);

        // 长度字段不能超过 Modbus-TCP 限制（理论最大 253）

        if (mbap_length > 253) {
            return Err(Serializer::make_length_exceed_error());
        }

        const std::array<uint8_t, 8> buf = {
            // 事务标识符 - 大端序
            static_cast<uint8_t>((transaction_id >> 8) & 0xFF),
            static_cast<uint8_t>(transaction_id & 0xFF),
            // 协议标识符 - 固定0x0000
            0x00,
            0x00,
            // 长度字段 - 大端序（后续字节总长度）
            static_cast<uint8_t>((mbap_length >> 8) & 0xFF),
            static_cast<uint8_t>(mbap_length & 0xFF),
            // 单元标识符
            unit_id,
            func_code
        };

        if (const auto res = serializer.push_bytes(buf); res.is_err()) {
            return Err(res.unwrap_err());
        }
    }

    if(const auto res = serialize_msg_context(
        serializer, msg
    ); res.is_err()) return Err(res.unwrap_err());


    return Ok();
}


}
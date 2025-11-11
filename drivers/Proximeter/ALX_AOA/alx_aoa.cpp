#include "alx_aoa_prelude.hpp"
#include "core/magic/enum_traits.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Self = AlxAoa_StreamParser;
using Error = Self::Error;

#define ALXAOA_DEBUG_EN 1

#if ALXAOA_DEBUG_EN == 1
#define ALXAOA_TODO(...) TODO()
#define ALXAOA_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define ALXAOA_PANIC(...) PANIC{__VA_ARGS__}
#define ALXAOA_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define ALXAOA_DEBUG(...)
#define ALXAOA_TODO(...) PANIC_NSRC()
#define ALXAOA_PANIC(...)  PANIC_NSRC()
#define ALXAOA_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif



[[nodiscard]] static constexpr uint8_t xor_bytes(
    const std::span<const uint8_t> bytes
){
    uint8_t ret = 0;
    for(const auto byte : bytes){
        ret ^= byte;
    }
    return ret;
}


template<typename T>
[[nodiscard]] static constexpr T be_bytes_to_int(
    const std::span<const uint8_t, sizeof(T)> bytes)
{
    using U = std::make_unsigned_t<T>;
    static_assert(sizeof(U) == sizeof(T));
    U ret = 0;
    
    // 替代方案：累积移位（可能更清晰）
    for(size_t i = 0; i < sizeof(U); i++) {
        ret = (ret << 8) | static_cast<U>(bytes[i]);
    }
    return std::bit_cast<T>(ret);
}

[[nodiscard]] static constexpr Result<Self::RequestCommand, Error> parse_command(
    const std::span<const uint8_t, 2> bytes
){
    const auto bits = be_bytes_to_int<uint16_t>(bytes);
    switch(bits){
        case static_cast<uint16_t>(Self::RequestCommand::Location):
            return Ok(Self::RequestCommand::Location);
        case static_cast<uint16_t>(Self::RequestCommand::HeartBeat):
            return Ok(Self::RequestCommand::HeartBeat);
        default:
            return Err(Error::InvalidCommand);
    }
}

[[nodiscard]] static constexpr Result<Self::DeviceId, Error> parse_device_id(
    const std::span<const uint8_t, 4> bytes
){
    const auto bits = be_bytes_to_int<uint32_t>(bytes);
    return Ok(Self::DeviceId::from_bits(bits));
}

[[nodiscard]] static constexpr Result<Self::TargetAngle, Error> parse_angle(
    const std::span<const uint8_t, 2> bytes
){
    const auto bits = be_bytes_to_int<int16_t>(bytes);
    return Ok(Self::TargetAngle::from_bits(bits));
}

[[nodiscard]] static constexpr Result<Self::TargetDistance, Error> parse_distance(
    const std::span<const uint8_t, 4> bytes
){
    const auto bits = be_bytes_to_int<uint32_t>(bytes);
    return Ok(Self::TargetDistance::from_bits(bits));
}

[[nodiscard]] static constexpr Result<Self::TargetStatus, Error> parse_tag_status(
    const std::span<const uint8_t, 2> bytes
){
    const auto bits = be_bytes_to_int<uint16_t>(bytes);
    if(bits != static_cast<uint16_t>(Self::TargetStatus::Normal)) 
        return Err(Error::InvalidTagStatus);
    return Ok(Self::TargetStatus{bits});
}

[[nodiscard]] static constexpr Result<uint16_t, Error> parse_batch_sn(
    const std::span<const uint8_t, 2> bytes
){
    const auto bits = be_bytes_to_int<uint16_t>(bytes);
    return Ok(bits);
}



static Result<Self::Location, Error> parse_location(BytesSpawner & spawner){
    // AnchorID 4 unsigned Integer 基站 ID 
    // TagID 4 unsigned Integer 标签 ID 
    // Distance 4 unsigned Integer 标签与基站间的距离，单位 cm 
    // Azimuth 2 signed Integer 标签与基站间的方位角，单位度 
    // Elevation 2 signed Integer 标签与基站间的仰角，单位度 
    // TagStatus 2 Byte 标签的状态 
    // BatchSn 2 Byte 测距序号 
    // Reserve 4 Byte 预留 
    // XorByte 1 Byte 该字节前所有字节的异或校验
    if(spawner.remaining().size() != 25){
        return Err(Error::InvalidLength);
    }

    const auto anchor_id = ({
        const auto res = parse_device_id(spawner.spawn<4>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const auto target_id = ({
        const auto res = parse_device_id(spawner.spawn<4>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const auto distance = ({
        const auto res = parse_distance(spawner.spawn<4>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const auto azimuth = ({
        const auto res = parse_angle(spawner.spawn<2>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const auto elevation = ({
        const auto res = parse_angle(spawner.spawn<2>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    [[maybe_unused]] const auto tag_status = ({
        const auto res = parse_tag_status(spawner.spawn<2>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    [[maybe_unused]] const auto batch_sn = ({
        const auto res = parse_batch_sn(spawner.spawn<2>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });


    const Self::Location msg = {
        .anchor_id = anchor_id,
        .target_id = target_id,
        .distance = distance,
        .azimuth = azimuth,
        .elevation = elevation
    };

    return Ok(msg);
}

#if 0
[[maybe_unused]]static void static_test(){
    constexpr uint8_t bytes[] = {
        0x00, 0x00, 0xAA, 0xA2, 0x00, 0x00, 0xAA, 0xA1, 0x00, 
        0x00, 0x00, 0x62, 0xFF, 0xD9, 0x00, 0x0D, 0x12, 0x34, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00
    };

    constexpr auto msg = [&]{
        auto spawner = BytesSpawner(std::span(bytes));
        return parse_location(spawner);
    }();

    static_assert(msg.is_ok());
}
#endif


void Self::push_byte(const uint8_t byte){
    auto fsm_update = [&](const auto state){ 
        // ALXAOA_DEBUG(state);
        byte_prog_ = state; 
    };
    switch(byte_prog_){
        case ByteProg::Header0:
            if(byte != 0xff){reset(); break;}
            fsm_update(ByteProg::Header1);
            break;
        case ByteProg::Header1:
            if(byte != 0xff){reset(); break;}
            fsm_update(ByteProg::Header2);
            break;
        case ByteProg::Header2:
            if(byte != 0xff){reset(); break;}
            fsm_update(ByteProg::Header3);
            break;
        case ByteProg::Header3:
            if(byte != 0xff){reset(); break;}
            fsm_update(ByteProg::WaitingLen0);
            break;
        case ByteProg::WaitingLen0: 
            if(byte != 0x00){reset(); break;}
            fsm_update(ByteProg::WaitingLen1);
            break;
        case ByteProg::WaitingLen1:{
            const auto len = static_cast<uint8_t>(byte);
            const bool is_valid_len = [&]{
                switch(len){
                    case 16:
                    case 37:
                        return true;
                    default:
                        return false;
                }
            }();
            
            if(not is_valid_len) {reset(); break;}
            leader_info_.len = len;
            fsm_update(ByteProg::Remaining);
            break;
        }
        case ByteProg::Remaining:
            payload_bytes_.push_back(byte);

            if(payload_bytes_.size() + LEADER_SIZE >= leader_info_.len){
                flush();
                reset();
            }
            break;
    }

}




Result<Self::Event, Self::Error>  Self::parse(){
    const size_t size = payload_bytes_.size();
    if(size < 10) ALXAOA_PANIC("size too small", size, leader_info_.len);


    const auto context_bytes = std::span(payload_bytes_.data(), size);

    // ALXAOA_DEBUG("header");
    // for(const auto byte : header_bytes) {ALXAOA_DEBUG(std::hex, std::showbase, byte); clock::delay(1ms);}
    // ALXAOA_DEBUG("context");
    // for(const auto byte : context_bytes) {ALXAOA_DEBUG(std::hex, std::showbase, byte); clock::delay(1ms);}



    BytesSpawner spawner(context_bytes);

    // SequenceID 2 unsigned Integer 消息流水号 
    // RequestCommand 2 unsigned Integer 命令码 
    // VersionID 2 unsigned Integer 协议版本，此版本固定 0x0100 

    [[maybe_unused]] const auto seq_id = be_bytes_to_int<uint16_t>(spawner.spawn<2>());
    const auto req_command = ({
        const auto res = parse_command(spawner.spawn<2>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    [[maybe_unused]] const auto protocol_version = be_bytes_to_int<uint16_t>(spawner.spawn<2>());

    //官方给的协议版本也不固定 不检测
    // if(protocol_version != 0x0100){
    //     ALXAOA_DEBUG("protocol_version", protocol_version);
    //     return Err(Error::InvalidProtocolVersion);
    // }


    switch(req_command){

        case RequestCommand::Location:{
            #if 1
            const auto header_xor = leader_info_.len; // 0xff ^ 0xff ^ 0xff ^ 0xff ^ 0x00 ^ len = len
            const auto context_xor = xor_bytes(std::span(context_bytes.begin(), std::prev(context_bytes.end())));
            const auto actual_xor = header_xor ^ context_xor;

            const auto expected_xor = *std::prev(context_bytes.end());
            
            if(actual_xor != expected_xor) {
                return Err(Error::InvalidXor);
            }
            
            const auto res = parse_location(spawner);
            if(res.is_err()) return Err(res.unwrap_err());
            return Ok(Event(res.unwrap()));
            // return Err(Error::InvalidProtocolVersion);
            break;
            #endif
        }
        case RequestCommand::HeartBeat:{
            #if 1
            // const auto res = parse_heartbeat(spawner);
            // if(res.is_err()) return Err(res.unwrap_err());
            // return Ok(Event(res.unwrap()));

            if(spawner.remaining().size() != 4) return Err(Error::InvalidLength);
            const auto anchor_id = ({
                const auto res = parse_device_id(spawner.spawn<4>());
                if(res.is_err()) return Err(res.unwrap_err());
                res.unwrap();
            });
            // AnchorID 4 unsigned Integer ID

            // Event ev = Event(parse_location(spawner).unwrap());
            // Event ev = Event(Self::HeartBeat{
            //     // .anchor_id = DeviceId::from_bits(0)
            //     .anchor_id = anchor_id
            // });
            Event ev = Event(std::monostate{});
            Self::HeartBeat msg{
                .anchor_id = anchor_id
            };

            ev = Event(msg);
            return Ok(ev);
            #endif
            break;
        }
        default:
            return Err(Error::InvalidRequest);
    }
    return Err(Error::InvalidRequest);
};



void Self::flush(){
    if(callback_ == nullptr) ALXAOA_PANIC("no callback");


    callback_(parse());

}


namespace ymd::drivers{

OutputStream & operator <<(OutputStream & os, const AlxAoa_Prelude::Error & error){
    DeriveDebugDispatcher<AlxAoa_Prelude::Error>::call(os, error);
    return os;
}
OutputStream & operator <<(OutputStream & os, const AlxAoa_StreamParser::ByteProg & self){
    DeriveDebugDispatcher<AlxAoa_StreamParser::ByteProg>::call(os, self);
    return os;
}
}
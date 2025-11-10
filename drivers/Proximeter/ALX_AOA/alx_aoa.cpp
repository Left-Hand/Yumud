#include "alx_aoa_prelude.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Self = ALXAOA_StreamParser;
using Error = Self::Error;

template<typename T>
[[nodiscard]] static constexpr T int_from_be_bytes(const std::span<const uint8_t, sizeof(T)> bytes){
    T ret = 0;
    for(size_t i = 0; i < sizeof(T); i++){
        ret |= static_cast<T>(bytes[i]) << (8 * (sizeof(T) - 1 - i));
    }
    return ret;
}

[[nodiscard]] static constexpr Result<Self::RequestCommand, Error> parse_command(const std::span<const uint8_t, 2> bytes){
    const auto bits = int_from_be_bytes<uint16_t>(bytes);
    switch(bits){
        case static_cast<uint16_t>(Self::RequestCommand::Location):
            return Ok(Self::RequestCommand::Location);
        case static_cast<uint16_t>(Self::RequestCommand::HeartBeat):
            return Ok(Self::RequestCommand::HeartBeat);
        default:
            return Err(Error::InvalidCommand);
    }
}

[[nodiscard]] static constexpr Result<Self::DeviceId, Error> parse_device_id(const std::span<const uint8_t, 4> bytes){
    const auto bits = int_from_be_bytes<uint32_t>(bytes);
    return Ok(Self::DeviceId{bits});
}

[[nodiscard]] static constexpr Result<Self::TargetAngle, Error> parse_angle(const std::span<const uint8_t, 2> bytes){
    const auto bits = int_from_be_bytes<int16_t>(bytes);
    return Ok(Self::TargetAngle(bits));
}

[[nodiscard]] static constexpr Result<Self::TargetDistance, Error> parse_distance(const std::span<const uint8_t, 4> bytes){
    const auto bits = int_from_be_bytes<uint32_t>(bytes);
    return Ok(Self::TargetDistance(bits));
}

[[nodiscard]] static constexpr Result<Self::TargetStatus, Error> parse_tag_status(const std::span<const uint8_t, 2> bytes){
    const auto bits = int_from_be_bytes<uint16_t>(bytes);
    if(bits != static_cast<uint16_t>(Self::TargetStatus::Normal)) 
        return Err(Error::InvalidTagStatus);
    return Ok(Self::TargetStatus{bits});
}

[[nodiscard]] static constexpr Result<uint16_t, Error> parse_batch_sn(const std::span<const uint8_t, 2> bytes){
    const auto bits = int_from_be_bytes<uint16_t>(bytes);
    return Ok(bits);
}

[[nodiscard]] static constexpr uint8_t xor_bytes(const std::span<const uint8_t> bytes){
    uint8_t ret = 0;
    for(const auto byte : bytes){
        ret ^= byte;
    }
    return ret;
}

static constexpr Result<Self::HeartBeat, Error> parse_heartbeat(BytesSpawner & spawner){
    const auto anchor_id = ({
        const auto res = parse_device_id(spawner.spawn<4>());
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    // AnchorID 4 unsigned Integer ID

    const Self::HeartBeat msg = {
        .anchor_id = anchor_id
    };

    return Ok(msg);
}

static constexpr Result<Self::Location, Error> parse_location(BytesSpawner & spawner){
    // AnchorID 4 unsigned Integer 基站 ID 
    // TagID 4 unsigned Integer 标签 ID 
    // Distance 4 unsigned Integer 标签与基站间的距离，单位 cm 
    // Azimuth 2 signed Integer 标签与基站间的方位角，单位度 
    // Elevation 2 signed Integer 标签与基站间的仰角，单位度 
    // TagStatus 2 Byte 标签的状态 
    // BatchSn 2 Byte 测距序号 
    // Reserve 4 Byte 预留 
    // XorByte 1 Byte 该字节前所有字节的异或校验
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

    if(spawner.remaining().size() != 4) __builtin_abort();

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
    switch(byte_prog_){
        case ByteProg::Header0:
            if(byte != 0xff){reset(); break;}
            byte_prog_ = ByteProg::Header1;
            break;
        case ByteProg::Header1:
            if(byte != 0xff){reset(); break;}
            byte_prog_ = ByteProg::Header2;
            break;
        case ByteProg::Header2:
            if(byte != 0xff){reset(); break;}
            byte_prog_ = ByteProg::Header3;
            break;
        case ByteProg::Header3:
            if(byte != 0xff){reset(); break;}
            byte_prog_ = ByteProg::WaitingLen0;
            break;
        case ByteProg::WaitingLen0: 
            if(byte != 0x00){reset(); break;}
            byte_prog_ = ByteProg::WaitingLen1;
            break;
        case ByteProg::WaitingLen1:


            leader_info_.len = static_cast<uint8_t>(byte);
            byte_prog_ = ByteProg::Remaining;
            break;
        case ByteProg::Remaining:
            payload_bytes_.push_back(byte);

            if(payload_bytes_.size() + LEADER_SIZE >= leader_info_.len){
                flush();
                reset();
            }
            break;
    }

}




void Self::flush(){
    if(callback_ == nullptr) __builtin_abort();



    auto parse_res = [&] -> Result<Event, Error> {
        const size_t size = payload_bytes_.size();
        if(size <= 10) __builtin_abort();

        const auto header_bytes = std::to_array<uint8_t>({
            0xff, 0xff, 0xff, 0xff, 0x00, static_cast<uint8_t>(size + 5)
        });

        const auto context_bytes = std::span(payload_bytes_.data(), size - 1);

        const auto expected_crc = xor_bytes((header_bytes)) ^ xor_bytes(context_bytes);

        const auto actual_crc = *payload_bytes_.end();
    
        if(actual_crc != expected_crc) return Err(Error::InvalidCrc);

        BytesSpawner spawner(context_bytes);

        // SequenceID 2 unsigned Integer 消息流水号 
        // RequestCommand 2 unsigned Integer 命令码 
        // VersionID 2 unsigned Integer 协议版本，此版本固定 0x0100 

        [[maybe_unused]] const auto seq_id = int_from_be_bytes<uint16_t>(spawner.spawn<2>());
        const auto req_command = ({
            const auto res = parse_command(spawner.spawn<2>());
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        const auto protocol_version = int_from_be_bytes<uint16_t>(spawner.spawn<2>());
        if(protocol_version != 0x0100) return Err(Error::InvalidProtocolVersion);


        switch(req_command){
            case RequestCommand::HeartBeat:{
                const auto res = parse_heartbeat(spawner);
                if(res.is_err()) return Err(res.unwrap_err());
                return Ok(Event(res.unwrap()));
                break;
            }
            case RequestCommand::Location:{
                const auto res = parse_location(spawner);
                if(res.is_err()) return Err(res.unwrap_err());
                return Ok(Event(res.unwrap()));
                break;
            }
            default:
                return Err(Error::InvalidRequest);
        }
    }();

    callback_(parse_res);

}
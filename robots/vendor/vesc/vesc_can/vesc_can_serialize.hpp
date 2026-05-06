#pragma once

#include "vesc_can_primitive.hpp"
#include "primitive/can/bxcan_frame.hpp"

namespace ymd::robots::vesc::can { 



static constexpr hal::CanExtId encode_can_id(
	const PacketId packet_id, 
	const uint8_t controller_id
) noexcept {
    uint32_t id_u29 = 0; 
    id_u29 |= static_cast<uint32_t>(packet_id) << 8;
	id_u29 |= static_cast<uint32_t>(controller_id) << 0;

    return hal::CanExtId::from_u29(id_u29);
}


template<typename T>
static constexpr hal::ClassicCanFrame make_can_frame(
    const uint8_t controller_id,
    const T & msg
){
    constexpr PacketId PACKET_ID = T::PACKET_ID; 
    constexpr size_t PAYLOAD_LENGTH = T::PAYLOAD_LENGTH;

    const auto can_id = encode_can_id(PACKET_ID, controller_id);
    alignas(4) std::array<uint8_t, 8> payload_buf;

    // fuck, memcpy can't be constexpr 

    if(std::is_constant_evaluated()){
        if constexpr(sizeof(msg) == 8){
            const uint64_t bits = std::bit_cast<uint64_t>(msg);
            payload_buf = std::bit_cast<std::array<uint8_t, 8>>(bits);
        }else if constexpr(sizeof(msg) == 4){
            const uint32_t bits = std::bit_cast<uint32_t>(msg);
            const auto temp_buf = std::bit_cast<std::array<uint8_t, 4>>(bits);
            std::copy(temp_buf.begin(), temp_buf.end(), payload_buf.begin());
        }else{
            __builtin_unreachable();
        }
    }else{
        (void)__builtin_memcpy(payload_buf.data(), &msg, PAYLOAD_LENGTH);
    }

    return hal::ClassicCanFrame::from_parts(
        can_id,
        hal::ClassicCanPayload::from_u8x8(payload_buf)
    );
}

}
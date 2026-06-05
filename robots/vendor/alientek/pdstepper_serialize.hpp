#pragma once

#include "pdstepper_primitive.hpp"
#include "pdstepper_checksum.hpp"
#include "core/utils/Result.hpp"

namespace ymd::robots::pdstepper{



template<typename Serializer>
Result<void, typename Serializer::Error> serialize_header(
    Serializer & srz,
    const uint8_t motor_id, 
    const Command command
){
    const uint8_t buf[] = {
        FRAME_HEAD_CHAR,
        motor_id,
        static_cast<uint8_t>(command)
    };

    return srz.push_bytes(buf);
}


template<typename Serializer>
Result<void, typename Serializer::Error> serialize_trailer(
    Serializer & srz
){
    const auto bytes = srz.collected_byte();
    const uint8_t checksum = ChecksumBuilder::from_default()
        .push_bytes(bytes)
        .finalize();

    const uint8_t buf[] = {
        checksum, FRAME_TAIL_CHAR
    };

    return srz.push_bytes(buf);
}

}
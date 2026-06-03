#pragma once

#include "pdstepper_primitive.hpp"
#include "pdstepper_checksum.hpp"
#include "core/utils/Result.hpp"

namespace ymd::robots::pdstepper{



template<typename Serialize>
Result<void, typename Serialize::Error> serialize_header(
    Serialize & srz,
    const uint8_t motor_id, 
    const FuncionCode fc
){
    const uint8_t buf[] = {
        FRAME_HEAD_TOEKN,
        motor_id,
        static_cast<uint8_t>(fc)
    };

    return srz.push_bytes(buf);
}


template<typename Serialize>
Result<void, typename Serialize::Error> serialize_trailer(
    Serialize & srz
){
    const auto bytes = srz.collected_byte();
    const uint8_t checksum = ChecksumBuilder::from_default().push_bytes(bytes).finalize();

    const uint8_t buf[] = {
        checksum, FRAME_TAIL_TOKEN
    };

    return srz.push_bytes(buf);
}

}
#pragma once

#include "pdstepper_primitive.hpp"
#include "pdstepper_checksum.hpp"
#include "core/utils/Result.hpp"

namespace ymd::robots::pdstepper{



template<typename Receiver>
Result<void, typename Receiver::Error> push_begin(
    Receiver & receiver,
    const uint8_t motor_id, 
    const FuncionCode fc
){
    const uint8_t buf[] = {
        FRAME_HEAD_TOEKN,
        motor_id,
        static_cast<uint8_t>(fc)
    };

    return receiver.push_bytes(buf);
}


template<typename Receiver>
Result<void, typename Receiver::Error> push_end(
    Receiver & receiver
){
    const auto bytes = receiver.collected_byte();
    const uint8_t checksum = ChecksumBuilder::from_default().push_bytes(bytes).finalize();

    const uint8_t buf[] = {
        checksum, FRAME_TAIL_TOKEN
    };

    return receiver.push_bytes(buf);
}

}
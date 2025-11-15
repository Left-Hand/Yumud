#pragma once

#include <cstdint>

namespace ymd::drivers{

enum class ComputeDistanceError:uint8_t{
    /// Reply times are too large to be multiplied
    ReplyTimesTooLarge,

    /// Round-trip times are too large to be multiplied
    RoundTripTimesTooLarge,

    /// The sum computed as part of the algorithm is too large
    SumTooLarge,

    /// The time of flight is so large, the distance calculation would overflow
    TimeOfFlightTooLarge,

    /// Round trip product is greater than round trip time due to low power
    // Not exactly sure what causes this but it's a potential problem and occurs when VCC is low
    RtGreaterThanRtt,
}
}
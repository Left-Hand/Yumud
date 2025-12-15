#pragma once

#include <cstdint>

// 参考来源

// https://github.com/jkelleyrtp/dw1000-rs/blob/master/dw1000/src/hl/ready.rs
// * BSD 3-Clause License
// * Copyright (c) Hanno Braun hanno@braun-embedded.com and contributors
// * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.

namespace ymd::drivers{

namespace dw1000{
/// The behaviour of the sync pin
enum SyncBehaviour:uint8_t{
    /// The sync pin does nothing
    None,
    /// The radio time will reset to 0 when the sync pin is high and the clock gives a rising edge
    TimeBaseReset,
    /// When receiving, instead of reading the internal timestamp, the time since the last sync
    /// is given back.
    ExternalSync,
    /// When receiving, instead of reading the internal timestamp, the time since the last sync
    /// is given back. Also resets the internal timebase back to 0.
    ExternalSyncWithReset,
};


#if 0
/// The time at which the transmission will start
enum SendTime:uint8_t{
    /// As fast as possible
    Now,
    /// After some time
    Delayed(Instant),
    /// After the sync pin is engaged. (Only works when sync setup is in ExternalSync mode)
    OnSync,
}
#endif

/// The polarity of the irq signal
enum class Polarity:uint8_t {
    /// The signal will be high when the interrupt is active
    ActiveHigh = 1,
    /// The signal will be low when the interrupt is active
    ActiveLow = 0,
};

}
}
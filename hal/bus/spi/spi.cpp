#include "spi.hpp"

using namespace ymd;
using namespace ymd::hal;

hal::BusError Spi::lead(const LockRequest req) {
    if(last_index.has_value()) return hal::BusError::OccuipedByOther;
    const auto index = req.id();
    if(!cs_port_.is_index_valid(index)) return hal::BusError::NoSelecter;
    cs_port_[index].clr();
    last_index = index;
    return hal::BusError::Ok();
}


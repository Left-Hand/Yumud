#include "spi.hpp"

using namespace ymd;
using namespace ymd::hal;

hal::HalResult Spi::lead(const LockRequest req) {
    if(last_index.has_value()) return hal::HalResult::OccuipedByOther;
    const auto index = req.id();
    if(!cs_port_.is_index_valid(index)) return hal::HalResult::NoSelecter;
    cs_port_[index].clr();
    last_index = index;
    return hal::HalResult::Ok();
}


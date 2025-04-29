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

std::optional<SpiSlaveIndex> Spi::attach_next_cs(hal::GpioIntf & io){
    for(size_t i = 0; i < cs_port_.size(); i++){
        if(cs_port_.is_index_empty(i)){
            io.outpp(HIGH);
            cs_port_.bind_pin(io, i);
            return SpiSlaveIndex{uint16_t(i)};
        }
    }
    return std::nullopt;
}
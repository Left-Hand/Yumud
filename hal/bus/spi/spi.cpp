#include "spi.hpp"

using namespace ymd;
using namespace ymd::hal;



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
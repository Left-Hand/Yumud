#include "spi.hpp"

using namespace ymd;
using namespace ymd::hal;



Option<SpiSlaveIndex> Spi::allocate_cs_gpio(Some<hal::GpioIntf *> p_io){
    auto & io = p_io.deref();
    for(size_t i = 0; i < cs_port_.size(); i++){
        if(cs_port_.is_index_empty(i)){
            io.outpp(HIGH);
            cs_port_.bind_pin(io, i);
            return Some(SpiSlaveIndex{uint16_t(i)});
        }
    }
    return None;
}
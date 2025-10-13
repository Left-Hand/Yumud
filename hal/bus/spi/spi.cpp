#include "spi.hpp"

using namespace ymd;
using namespace ymd::hal;



Option<SpiSlaveRank> Spi::allocate_cs_gpio(Some<hal::GpioIntf *> p_io){
    auto & io = p_io.deref();
    for(size_t i = 0; i < cs_port_.size(); i++){
        const auto nth = Nth(i);
        if(cs_port_.is_nth_available(nth)){
            io.outpp(HIGH);
            cs_port_.bind_pin(io, nth);
            return Some(SpiSlaveRank{
                static_cast<uint8_t>(nth.count())
            });
        }
    }
    return None;
}
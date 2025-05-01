#include "can_msg.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{

OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg){
    const auto guard = os.create_guard();
    
    os << os.brackets<'{'>();

    os.set_radix(16);
    os << uint32_t(msg.id());
        
    os << '<'
        << ((msg.is_std()) ? 'S' : 'E')
        << ((msg.is_remote()) ? 'R' : 'D') << std::noshowbase
        << '[' << std::dec << msg.size() << ']';
    os << "> ";
    
    os << std::hex << std::span<const uint8_t>{
        reinterpret_cast<const uint8_t *>(msg.begin()), 
        msg.size()
    };

    return os << os.brackets<'}'>();
}

}
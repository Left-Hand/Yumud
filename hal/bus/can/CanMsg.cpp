#include "CanMsg.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{

OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg){
    const auto guard = os.createGuard();
    
    os << os.brackets<'{'>();

    os.setRadix(16);
    os << uint32_t(msg.id());
        
    os << '<'
        << ((msg.is_std()) ? 'S' : 'E')
        << ((msg.is_remote()) ? 'R' : 'D') << std::noshowbase
        << '[' << std::dec << msg.size() << ']';
    os << "> ";
    
    os << std::hex << std::span<const std::byte>{
        reinterpret_cast<const std::byte *>(msg.begin()), 
        msg.size()
    };

    return os << os.brackets<'}'>();
}

}
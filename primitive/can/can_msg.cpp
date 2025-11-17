#include "can_msg.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{

OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg){
    const auto guard = os.create_guard();
    
    os << os.brackets<'{'>();

    os << '<'
        << ((msg.is_standard()) ? 'S' : 'E')
        << ((msg.is_remote()) ? 'R' : 'D')
        << "> ";

    os << "id=" << std::hex << std::showbase << msg.id_as_u32();

    if(not msg.is_remote()){
        os << " | buf[" << std::dec << msg.size() << "]=" 
            << std::hex << msg.payload_bytes();
    }

    return os << os.brackets<'}'>();
}

}
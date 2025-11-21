#include "bxcan_frame.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{

OutputStream & operator<<(OutputStream & os, const hal::CanClassicFrame & frame){
    const auto guard = os.create_guard();

    os << '<'
        << ((frame.is_standard()) ? 'S' : 'E')
        << ((frame.is_remote()) ? 'R' : 'D')
        << "> ";

    os << "id=" << std::hex << std::showbase << frame.id_u32();

    if(not frame.is_remote()){
        os << " | buf[" << std::dec << frame.length() << "]=" 
            << std::hex << frame.payload_bytes();
    }

    return os;
}

}
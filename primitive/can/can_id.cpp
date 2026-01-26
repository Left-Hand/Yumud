#include "can_id.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{

OutputStream & operator <<(OutputStream & os, const hal::CanStdId & self){
    auto guard = os.create_guard();
    return os << std::hex << self.to_u11();
} 

OutputStream & operator <<(OutputStream & os, const hal::CanExtId & self){
    auto guard = os.create_guard();
    return os << std::hex << self.to_u29();
} 



}
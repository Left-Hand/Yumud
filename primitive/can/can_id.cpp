#include "can_id.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{

OutputStream & operator <<(OutputStream & os, hal::CanStdId & id){
    return os << id.to_u11();
} 

OutputStream & operator <<(OutputStream & os, hal::CanExtId & id){
    return os << id.to_u29();
} 



}
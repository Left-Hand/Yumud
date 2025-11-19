#include "can_dlc.hpp"

namespace ymd::hal{

OutputStream & operator <<(OutputStream & os, hal::CanClassicDlc & self){
    return os << os.field("length")(self.length());
} 

OutputStream & operator <<(OutputStream & os, hal::CanFdDlc & self){
    return os << os.field("length")(self.length());
} 


}
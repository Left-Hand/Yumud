#include "can_dlc.hpp"

namespace ymd::hal{

OutputStream & operator <<(OutputStream & os, hal::BxCanDlc & self){
    return os << os.field("length")(self.length());
} 

OutputStream & operator <<(OutputStream & os, hal::FdCanDlc & self){
    return os << os.field("length")(self.length());
} 


}
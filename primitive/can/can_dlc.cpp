#include "can_dlc.hpp"

namespace ymd::hal{

OutputStream & operator <<(OutputStream & os, hal::CanClassicDlc & dlc){
    return os << dlc.length();
} 

OutputStream & operator <<(OutputStream & os, hal::CanFdDlc & dlc){
    return os << dlc.length();
} 


}
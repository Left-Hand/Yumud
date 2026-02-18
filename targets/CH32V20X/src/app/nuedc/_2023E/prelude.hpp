#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"

using namespace ymd;

namespace nuedc::_2023E{
struct YawPitch final{
    iq16 yaw;
    iq16 pitch;

    friend OutputStream & operator << (OutputStream & os, const YawPitch & self){
        return os << os.brackets<'('>() 
            << self.yaw << os.splitter() 
            << self.pitch << os.brackets<')'>();
    }
};

//描述了一组关节解
using GimbalSolution = YawPitch;
}

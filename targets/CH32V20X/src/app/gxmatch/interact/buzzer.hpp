#pragma once

namespace ymd{
class PwmChannelIntf;
}

namespace gxm{

class Buzzer{
protected:
    using Inst = ymd::PwmChannelIntf;
    Inst & instance_;

public:
    Buzzer(Inst & instance):instance_(instance){;}

    Buzzer & operator =(const bool en);
};

}
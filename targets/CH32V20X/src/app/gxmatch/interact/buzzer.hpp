#pragma once


namespace ymd::hal{
class PwmIntf;
}

namespace gxm{

class Buzzer{
protected:
    using Inst = ymd::hal::PwmIntf;
    Inst & instance_;

public:
    Buzzer(Inst & instance):instance_(instance){;}

    Buzzer & operator =(const bool en);
};

}
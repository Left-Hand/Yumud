#pragma once

namespace ymd{
class PwmIntf;
}

namespace gxm{

class Buzzer{
protected:
    using Inst = ymd::PwmIntf;
    Inst & instance_;

public:
    Buzzer(Inst & instance):instance_(instance){;}

    Buzzer & operator =(const bool en);
};

}
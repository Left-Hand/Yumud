#pragma once

namespace ymd{
class PwmChannel;
}

namespace gxm{

class Buzzer{
protected:
    using Inst = ymd::PwmChannel;
    Inst & instance_;

public:
    Buzzer(Inst & instance):instance_(instance){;}

    Buzzer & operator =(const bool en);
};

}
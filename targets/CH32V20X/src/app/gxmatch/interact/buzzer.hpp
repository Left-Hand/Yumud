#pragma once

namespace yumud{
class PwmChannel;
}

namespace gxm{

class Buzzer{
protected:
    using Inst = yumud::PwmChannel;
    Inst & instance_;

public:
    Buzzer(Inst & instance):instance_(instance){;}

    Buzzer & operator =(const bool en);
};

}
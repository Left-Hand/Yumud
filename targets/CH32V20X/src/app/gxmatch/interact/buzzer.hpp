#pragma once

class PwmChannel;

namespace gxm{

class Buzzer{
protected:
    PwmChannel & instance_;

public:
    Buzzer(PwmChannel & instance):instance_(instance){;}

    Buzzer & operator =(const bool en);
};

}
#pragma once

#include "../adc.hpp"

namespace ymd{

class Adc2:public AdcCompanion{
    Adc2():AdcCompanion(ADC2){;}
};


}
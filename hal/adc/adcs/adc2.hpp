#pragma once

#include "../adc.hpp"

namespace yumud{

class Adc2:public AdcCompanion{
    Adc2():AdcCompanion(ADC2){;}
};


}
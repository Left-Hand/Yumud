#ifndef __ADC_CHANNEL_HPP__

#define __ADC_CHANNEL_HPP__

#include "src/platform.h"

class AdcChannelBase{

};

class AdcChannel: public AdcChannelBase{

};

class RegularChannel: public AdcChannel{

};

class InjectedChannel: public AdcChannel{

};
#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "real.hpp"
#include "comm_inc.h"

class PwmChannelBase{
public:
    virtual void enable(const bool en = true) = 0;
    virtual void setDuty(const real_t & duty) = 0;
};

#endif
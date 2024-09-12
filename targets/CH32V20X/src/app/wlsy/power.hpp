#ifndef __POWER_HPP__

#define __POWER_HPP__

namespace WLSY{

class WattMonitor{
    virtual real_t getWatt() = 0;
};

}
#endif
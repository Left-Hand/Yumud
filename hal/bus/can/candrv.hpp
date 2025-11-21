#pragma once

#include "hal/bus/can/can.hpp"


namespace ymd::hal{
class CanDrv{
    public:
        using CanClassicFrame = hal::CanClassicFrame; 

        CanDrv(hal::Can & can):can_(can){;}
    
        Result<void, CanError> transmit(const CanClassicFrame & frame){
            return can_.write(frame);
        }
    
    private:
        hal::Can & can_; 
};

}
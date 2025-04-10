#pragma once

#include "hal/bus/can/can.hpp"


namespace ymd::hal{
class CanDrv{
    public:
        using CanMsg = hal::CanMsg; 

        CanDrv(hal::Can & can):can_(can){;}
    
        void transmit(const CanMsg & msg){
            can_.write(msg);
        }
    
    private:
        hal::Can & can_; 
};

}
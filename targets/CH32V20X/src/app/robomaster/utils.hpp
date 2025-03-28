#pragma once


#include "hal/bus/can/can.hpp"

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/PerUnit.hpp"

#include "core/math/real.hpp"

namespace ymd::rmst{


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
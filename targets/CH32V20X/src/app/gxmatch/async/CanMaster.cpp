#include "CanMaster.hpp"
#include "sys/debug/debug_inc.h"

using namespace gxm;
using namespace ymd;

void CanMaster::update(){
    auto & can = can1;
    if(can.available()){
        while(can.available()){
            auto && msg = can.read();

            bool accepted = false;
            for(Wrapper & proto : pool){
                if(proto.get().update(msg)){
                    accepted = true;
                    break;
                }
            }

            if(accepted){

            }else{ 
                DEBUG_PRINTLN("unaccepted msg", msg.id());
            }
        }
    }
}
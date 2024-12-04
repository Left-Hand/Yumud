#include "CanMaster.hpp"
#include "sys/debug/debug_inc.h"

using namespace gxm;
using namespace ymd;

void CanMaster::update(){
    if(can_.available()){
        while(can_.available()){
            auto && msg = can_.read();

            bool accepted = false;
            for(Wrapper & item : pool){
                if(item.get().update(msg)){
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
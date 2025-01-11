#include "CanMaster.hpp"
#include "sys/debug/debug_inc.h"

using namespace gxm;
using namespace ymd;
using namespace ymd::hal;

void CanMaster::update(){
    if(can_.available()){
        // DEBUG_PRINTLN(pool.size());
        // while(can_.available()){
        if(true){
            auto msg = can_.read();

            bool accepted = false;
            for(CanProtocolConcept * item : pool){
                if(item){
                    if(item->update(msg)){
                        accepted = true;
                        break;
                    }
                }
            }

            if(accepted){
                // DEBUG_PRINTLN(msg);
            }else{ 
                // HALT;
                // DEBUG_PRINTLN("unaccepted msg", msg.id());
            }
        }
    }
}
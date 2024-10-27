#include "grab_module.hpp"
#include "actuator/jointlr.hpp"
#include "actuator/zaxis.hpp"
#include "scara/nozzle.hpp"

#include "../common/inc.hpp"

namespace gxm{

void GrabModule::goHome(){
    scara.goHome();
}

void GrabModule::moveZ(const real_t pos){
    zaxis.setDistance(pos);
}

void GrabModule::moveXY(const Vector2 & pos){
    scara.moveXY(pos);
}


void GrabModule::moveTo(const Vector3 & pos){
    moveXY({pos.x, pos.y});
    moveZ(pos.z);
}

void GrabModule::pickUp(){
    scara.pickUp();
}

void GrabModule::putDown(){
    scara.putDown();
}


void GrabModule::take(){
    
}

void GrabModule::give(){
    
}


bool GrabModule::reached(){
    return scara.reached() and zaxis.reached();
}

// bool GrabModule::done(){
//     return zaxis.reached();
//     //  and joint_l.reached() and joint_r.reached();

// }

void GrabModule::begin(){
    
}

}
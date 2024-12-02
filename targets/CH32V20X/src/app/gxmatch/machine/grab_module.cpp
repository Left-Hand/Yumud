#include "grab_module.hpp"
#include "actuator/jointlr.hpp"
#include "actuator/zaxis.hpp"
#include "scara/nozzle.hpp"

#include "../common/inc.hpp"
#include "../autodrive/sequence/TrapezoidSolver_t.hpp"

using namespace gxm;
using namespace gxm::GrabActions;


class PressAction:public GrabAction{
public:
    PressAction(Inst & inst):
        GrabAction(200, [this](){
            inst_.press();
        }, inst){};
};


class ReleaseAction:public GrabAction{
public:
    ReleaseAction(Inst & inst):
        GrabAction(200, [this](){
            inst_.release();
        }, inst){};
};

class TestAction:public GrabAction{
public:
    TestAction(Inst & inst):
        GrabAction(200, [this](){
            DEBUG_PRINTLN(millis())
        }, inst){};
};

class MoveAction:public GrabAction{
protected:
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;

    Vector3 from_;
    real_t dist_;
    Vector3 norm_;
    TrapezoidSolver solver_;
    real_t dur_;
    // Vector3 to_;
public:
    MoveAction(Inst & inst, const Vector3 & from, const Vector3 & to):
        GrabAction(UINT_MAX, [this](){
            auto t = this->since();
            if(t > dur_){this->kill();}
            inst_.moveTo(from_ + norm_ * solver_.forward(this->dur_));
        }, inst),
        from_(from),
        dist_((to - from).length()),
        norm_((to - from) / dist_),

        solver_{
            inst.config().max_acc, 
            inst.config().max_spd,
            dist_
        },

        dur_{solver_.period()}
        
        {};
};

void GrabModule::goHome(){
    scara_.goHome();
}

void GrabModule::moveZ(const real_t pos){
    zaxis_.setDistance(pos);
}

void GrabModule::moveXY(const Vector2 & pos){
    scara_.moveXY(pos);
}


void GrabModule::moveTo(const Vector3 & pos){
    moveXY({pos.x, pos.y});
    moveZ(pos.z);
}


Vector3 GrabModule::getPos(){
    // return scara.
    TODO("no");
    return {0,0,0};
}
void GrabModule::press(){
    scara_.press();
}

void GrabModule::release(){
    scara_.release();
}


void GrabModule::take(){
    auto & self = *this;

    // self.addAction(new MoveAction{self, config_.})
    self << new TestAction{self};
}

void GrabModule::give(){
    // self.addAction(new MoveAction{self, config_.})
    // self.addAction(new )
}


bool GrabModule::reached(){
    return scara_.reached() and zaxis_.reached();
}

// bool GrabModule::done(){
//     return zaxis.reached();
//     //  and joint_l.reached() and joint_r.reached();

// }

void GrabModule::begin(){
    
}



Vector3 GrabModule::calculateTrayPos(TrayIndex index){
    auto vec2 = config_.tray_xy[size_t(index)];
    return {vec2.x, vec2.y, config_.tray_z};
}
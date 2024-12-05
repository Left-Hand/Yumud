#include "grab_actions.hpp"

using namespace gxm;
using namespace gxm::GrabActions;

void GrabModule::goHome(){
    scara_.goHome();
}

void GrabModule::rapid(const Vector3 & pos){
    if(config_.safe_aabb.has_point(pos) == false){
        DEBUG_PRINTLN("out of bound", pos);
        return;
    }

    scara_.moveXY(pos.xy());
    zaxis_.setDistance(pos.z + config_.z_bias);
}

void GrabModule::meta_press(){
    this->scara_.press();
}

void GrabModule::meta_release(){
    this->scara_.release();
}

void GrabModule::press(){
    auto & self = *this;
    self << new PressAction(self);
}

void GrabModule::release(){
    auto & self = *this;
    self << new ReleaseAction(self);
}

void GrabModule::move(const Vector3 & pos){
    auto & self = *this;
    self << new MoveAction(self, pos);
}


Vector3 GrabModule::getPos(){
    auto xy = scara_.getPos();
    auto z = zaxis_.getDistance();
    return {xy.x,xy.y,z};
}


void GrabModule::meta_air_take_air(){
    auto & self = *this;
    self  
        <<  new MoveXYAction(self, config_.catch_xy)
        <<  new MoveZAction(self, config_.catch_z)
        // << new DebugAction("press??")
        <<  new PressAction(self)
        <<  new MoveZAction(self, config_.free_z)
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::meta_air_give_air(const TrayIndex tray_index){
    auto & self = *this;
    self  
        <<  new MoveXYAction(self, calculateTrayPos(tray_index))
        <<  new MoveZAction(self, config_.tray_z)
        // << new DebugAction("press??")
        <<  new PressAction(self)
        <<  new MoveZAction(self, config_.free_z)
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::meta_to_air(){
    auto & self = *this;
    self  
        <<  new MoveZAction(self, config_.free_z)
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::meta_air_inspect(){
    auto & self = *this;
    self  
        <<  new MoveZAction(self, config_.inspect_xyz.z)
        <<  new MoveXYAction(self, config_.inspect_xyz.xy())
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::meta_take_place(const TrayIndex tray_index){
    auto & self = *this;
    self  
        <<  new PressAction(self)
        <<  new MoveZAction(self, config_.free_z)
        <<  new MoveXYAction(self, calculateTrayPos(tray_index))
        <<  new MoveZAction(self, config_.tray_z)
        <<  new ReleaseAction(self)
        <<  new StatusAction(self, TranportStatus::INNER)
    ;
}
void GrabModule::meta_give_place(){
    auto & self = *this;
    self  
        <<  new PressAction(self)
        <<  new MoveZAction(self, config_.free_z)
        <<  new MoveXYAction(self, config_.catch_xy)
        <<  new MoveZAction(self, config_.catch_z)
        // << new DebugAction("catch done" )
        <<  new ReleaseAction(self)
        <<  new StatusAction(self, TranportStatus::OUTER)
    ;
}


void GrabModule::inspect(){
    switch(status_){
        case TranportStatus::INNER:
            meta_to_air();
        default:
            meta_air_inspect();
    }
}

void GrabModule::take(const TrayIndex index){
    // switch(status_){
        // case TranportStatus::INNER:
            //如果当前在内部 需要先升空再取上
            meta_to_air();
            meta_air_take_air();
        // default:
            meta_take_place(index);
    // }
}



void GrabModule::give(const TrayIndex index){
    // switch(status_){
        // case TranportStatus::OUTER:
            //如果当前在外部 需要先升空再取上
            meta_to_air();
            meta_air_give_air(index);
        // default:
            meta_give_place();
    // }
}

void GrabModule::test(){
    auto & self = *this;

    // self.addAction(new MoveAction{self, config_.})
    // self << new TestAction{self};
    self << new TestAction{self};
    // self << MoveAction(self, Vector3(0, 0, 0), Vector3(1,1,1));
    // self << new MoveAction(self, Vector3(0, 0.2_r, 0.1_r), Vector3(0.1_r,0.2_r,0.1_r));
    // self << PressAction(self);
    // self << ReleaseAction(self);
}


bool GrabModule::reached(){
    return scara_.reached() and zaxis_.reached();
    // return getPos
}

bool GrabModule::caught(){
    return scara_.caught();
}

void GrabModule::init(){
    auto & self = *this;

    // self.rapid(Vector3(config_.inspect_xy);
    self << new RapidAction(self, config_.home_xyz);
    // self << new MoveZAction(self, config_.free_z);
    // self << new MoveXYAction(self, config_.catch_xy);
    self << new DelayAction(1000);
    self << new StatusAction(self, TranportStatus::AIR);
}


Vector2 GrabModule::calculateTrayPos(TrayIndex index){
    return {config_.tray_xy[size_t(index)]};
}
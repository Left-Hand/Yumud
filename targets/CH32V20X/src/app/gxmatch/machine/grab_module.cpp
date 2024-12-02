#include "grab_actions.hpp"

using namespace gxm;
using namespace gxm::GrabActions;

void GrabModule::goHome(){
    scara_.goHome();
}

void GrabModule::rapid(const Vector3 & pos){
    if(config_.safe_aabb.has_point(pos) == false){
        DEBUG_PRINTLN("out of bound", pos);
    }

    scara_.moveXY(pos.xy());
    zaxis_.setDistance(pos.z + config_.z_bias);
}

void GrabModule::press(){
    this->scara_.press();
}

void GrabModule::release(){
    this->scara_.release();
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


void GrabModule::air_take_air(){
    auto & self = *this;
    self  
        <<  new MoveXYAction(self, config_.catch_xy)
        <<  new MoveZAction(self, config_.catch_z)
        <<  new PressAction(self)
        <<  new MoveZAction(self, config_.free_z)
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::air_give_air(const TrayIndex tray_index){
    auto & self = *this;
    self  
        <<  new MoveXYAction(self, calculateTrayPos(tray_index))
        <<  new MoveZAction(self, config_.tray_z)
        <<  new PressAction(self)
        <<  new MoveZAction(self, config_.free_z)
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::to_air(){
    auto & self = *this;
    self  
        <<  new MoveZAction(self, config_.free_z)
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::air_inspect(){
    auto & self = *this;
    self  
        <<  new MoveZAction(self, config_.inspect_xyz.z)
        <<  new MoveXYAction(self, config_.inspect_xyz.xy())
        <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::take_place(const TrayIndex tray_index){
    auto & self = *this;
    self  
        <<  new MoveXYAction(self, calculateTrayPos(tray_index))
        <<  new MoveZAction(self, config_.tray_z)
        <<  new ReleaseAction(self)
        <<  new StatusAction(self, TranportStatus::INNER)
    ;
}
void GrabModule::give_place(){
    auto & self = *this;
    self  
        <<  new MoveXYAction(self, config_.catch_xy)
        <<  new MoveZAction(self, config_.catch_z)
        <<  new ReleaseAction(self)
        <<  new StatusAction(self, TranportStatus::OUTER)
    ;
}


void GrabModule::inspect(){
    switch(status_){
        case TranportStatus::INNER:
            to_air();
        default:
            air_inspect();
    }
}

void GrabModule::take(const TrayIndex index){
    switch(status_){
        case TranportStatus::INNER:
            //如果当前在内部 需要先升空再取上
            to_air();
            air_take_air();
        default:
            take_place(index);
    }
}



void GrabModule::give(const TrayIndex index){
    switch(status_){
        case TranportStatus::OUTER:
            //如果当前在外部 需要先升空再取上
            to_air();
            air_give_air(index);
        default:
            give_place();
    }
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
}

bool GrabModule::caught(){
    return scara_.caught();
}

void GrabModule::init(){
    auto & self = *this;

    // self.rapid(Vector3(config_.inspect_xy);
    self << RapidAction(self, config_.home_xyz);
    self << DelayAction(3000);
}


Vector2 GrabModule::calculateTrayPos(TrayIndex index){
    return {config_.tray_xy[size_t(index)]};
}
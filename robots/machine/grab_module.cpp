#include "grab_actions.hpp"

using namespace ymd;
using namespace ymd::robots;
using namespace ymd::robots::GrabActions;
#define DEBUG(...)     *this << new DebugAction(__VA_ARGS__);


void GrabModule::goHome(){
    scara_.goHome();
}

void GrabModule::rapid(const Vec3<iq16> & pos){
    if(config_.safe_aabb.has_point(pos) == false){
        DEBUG_PRINTLN("out of bound", pos);
        return;
    }

    scara_.moveXY(pos.xy());
    zaxis_.setDistance(pos.z + config_.z_bias);
}

void GrabModule::meta_idle(){
    scara_.idle();
    zaxis_.idle();
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

void GrabModule::move(const Vec3<iq16> & pos){
    auto & self = *this;
    self << new MoveAction(self, pos);
}

void GrabModule::move_xy(const Vec2<iq16> & pos){
    auto & self = *this;
    self << new MoveXYAction(self, pos);
}
    
void GrabModule::move_z(const real_t z){
    auto & self = *this;
    self << new MoveZAction(self, z);
}

Vec3<iq16> GrabModule::getPos(){
    auto xy = scara_.getPos();
    auto z = zaxis_.getDistance();
    return {xy.x,xy.y,z};
}


// void GrabModule::meta_air_take_air(const TrayIndex tray_index){
//     auto & self = *this;
//     self.move_xy(self.calculateCatchPos(tray_index));
//     self.move_z(config_.catch_z);
//     self.press();
//     self.move_z(config_.free_z);
//         // <<  new MoveXYAction(self, self.calculateCatchPos(tray_index))
//         // <<  new MoveZAction(self, config_.catch_z)
//         // <<  new PressAction(self)
//         // <<  new MoveZAction(self, config_.free_z)
//     ;
// }

// void GrabModule::meta_air_give_air(const TrayIndex tray_index){
//     auto & self = *this;
//     self.move_xy(calculateTrayPos(tray_index));  
//     self.move_z(config_.tray_z);
//     self.press();
//     self.move_z(config_.free_z);
// }

void GrabModule::meta_to_air(){
    auto & self = *this;
    self.move_z(config_.free_z);
        // <<  new MoveZAction(self, config_.free_z)
        // <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

void GrabModule::meta_air_inspect(){
    auto & self = *this;
    self.move_z(config_.inspect_xyz.z);
    self.move_xy(config_.inspect_xyz.xy());
        // <<  new MoveZAction(self, config_.inspect_xyz.z)
        // <<  new MoveXYAction(self, config_.inspect_xyz.xy())
        // <<  new StatusAction(self, TranportStatus::AIR)
    ;
}

// void GrabModule::meta_take_place(const TrayIndex tray_index){
//     auto & self = *this;
//     self.press();
//     self.move_z(config_.free_z);
//     DEBUG(calculateTrayPos(tray_index));
//     self.move_xy(calculateTrayPos(tray_index));
//     self.move_z(config_.tray_z);
//     self.release();
//         // <<  new PressAction(self)
//         // <<  new MoveZAction(self, config_.free_z)
//         // <<  new MoveXYAction(self, calculateTrayPos(tray_index))
//         // <<  new MoveZAction(self, config_.tray_z)
//         // <<  new ReleaseAction(self)
//     ;
// }
// void GrabModule::meta_give_place(const TrayIndex tray_index){
//     auto & self = *this;
//     self.press();
//     self.move_z(config_.free_z);
//     self.move_xy(calculateCatchPos(tray_index));  
//     // self.move_xy(config_.catch_xy[size_t(tray_index)]);  
//     // self << new DebugAction(config_.catch_xy[0]);
//     // self.move_xy(config_.catch_xy[0]);  
//     // self.move_z(config_.tray_z);
//     self.move_z(config_.catch_z);
//     self.release();

//     // self  
//     //     <<  new PressAction(self)
//     //     // <<  new MoveZAction(self, config_.free_z)
//     //     // << new DelayAction(1000)
//     //     // <<  new MoveXYAction(self, calculateCatchPos(TrayIndex::Center))
//     //     <<  new MoveXYAction(self, calculateCatchPos(tray_index))
//     //     // <<  new MoveXYAction(self, calculateCatchPos(TrayIndex::Right))
//     //     // << new DebugAction(calculateCatchPos(tray_index))
//     //     // << new DelayAction(10000)
//     //     // <<  new MoveZAction(self, config_.catch_z)
//     //     <<  new ReleaseAction(self)
//     // ;
// }


void GrabModule::inspect(){
    meta_to_air();
    meta_air_inspect();
}

void GrabModule::idle(){
    meta_to_air();
    this->move(config_.idle_xyz);
    this->release();
    *this << new IdleAction(*this);
}

// void GrabModule::take(const TrayIndex index){
//     meta_to_air();
//     meta_air_take_air(index);
//     meta_take_place(index);
//     meta_to_air();
// }



// void GrabModule::give(const TrayIndex index){
//     meta_to_air();
//     meta_air_give_air(index);
//     meta_give_place(index);
//     meta_to_air();
// }

void GrabModule::test(){
    auto & self = *this;

    // self.addAction(new MoveAction{self, config_.})
    // self << new TestAction{self};
    self << new TestAction{self};
    // self << MoveAction(self, Vec3<iq16>(0, 0, 0), Vec3<iq16>(1,1,1));
    // self << new MoveAction(self, Vec3<iq16>(0, 0.2_r, 0.1_r), Vec3<iq16>(0.1_r,0.2_r,0.1_r));
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

    self.move(config_.home_xyz);
    self.wait(1000);
    self.release();

}


// Vec2<iq16> GrabModule::calculateTrayPos(TrayIndex index) const{
//     // return {config_.tray_xy[size_t(index)].x, 0.144_r};
//     return {config_.tray_xy[size_t(index)]};
//     // switch(index){
//     //     case TrayIndex::Left :
//     //         return (Vec2<iq16>{-0.1_r, 0.2_r});  
//     //     case TrayIndex::Center :
//     //         return (Vec2<iq16>{0, 0.2_r});  
//     //     case TrayIndex::Right :
//     //         return (Vec2<iq16>{0.1_r, 0.2_r});  
//     // }
// }

// Vec2<iq16> GrabModule::calculateCatchPos(TrayIndex index) const{
//     return {config_.catch_xy[size_t(index)]};
//     // switch(index){
//     //     default:
//     //     case TrayIndex::Left :
//     //         return (Vec2<iq16>{-0.1_r, 0.24_r});  
//     //     case TrayIndex::Center :
//     //         return (Vec2<iq16>{0, 0.24_r});  
//     //     case TrayIndex::Right :
//     //         return (Vec2<iq16>{0.1_r, 0.24_r});  
//     // }
// }
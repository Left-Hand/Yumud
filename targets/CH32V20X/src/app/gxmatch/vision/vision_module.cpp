#include "vision_module.hpp"

namespace gxm{

void VisionModule::update(){
    
}

Vector2 VisionModule::getMatrialOffset(const MaterialColor color){
    return {0,0};
}

MaterialColor VisionModule::getCentralColor(){
    return MaterialColor::Blue;
}

bool VisionModule::confident(){
    return false;    
}

}
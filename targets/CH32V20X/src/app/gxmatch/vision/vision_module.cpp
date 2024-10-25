#include "vision_module.hpp"

namespace gxm{

void VisionModule::update(){
    
}

real_t VisionModule::getLanePadding(){
    return 0;
}

real_t VisionModule::getLaneRadian(){
    return 0;
}

Vector2 VisionModule::getBlockOffset(const BlockColor color){
    return {0,0};
}

BlockColor VisionModule::getCentreBlockColor(){
    return BlockColor::Blue;
}

bool VisionModule::confident(){
    return false;    
}

}
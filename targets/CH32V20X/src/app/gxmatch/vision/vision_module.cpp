#include "vision_module.hpp"

namespace gxm{

void VisionModule::update(){
    
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
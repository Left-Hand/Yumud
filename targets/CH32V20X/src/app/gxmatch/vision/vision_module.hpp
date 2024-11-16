#pragma once


#include "../common/inc.hpp"
namespace gxm{
    
class VisionModule{
public:
    void update();

    Vector2 getBlockOffset(const BlockColor color);
    BlockColor getCentreBlockColor();
    bool confident();
};


}
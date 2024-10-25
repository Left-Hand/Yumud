#pragma once


#include "../common/inc.hpp"
namespace gxm{
    
class VisionModule{
public:
    void update();

    real_t getLanePadding();
    real_t getLaneRadian();

    Vector2 getBlockOffset(const BlockColor color);
    BlockColor getCentreBlockColor();
    bool confident();
};


}
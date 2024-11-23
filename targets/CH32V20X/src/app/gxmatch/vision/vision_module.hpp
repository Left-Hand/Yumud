#pragma once


#include "../common/inc.hpp"
namespace gxm{
    
class VisionModule{
protected:

public:
    void update();

    Vector2 getMatrialOffset(const MaterialColor color);
    Vector2 getCentralOffset(){return getMatrialOffset(getCentralColor());}
    MaterialColor getCentralColor();
    bool confident();
};


}
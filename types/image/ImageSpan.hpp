#pragma once

#include "types/rgb.h"


namespace ymd{

struct ImageSpan{
    void * data;
    uint16_t w;
    uint16_t h;
    RgbType type;
};

}
#pragma once

#include "types/rgb.h"


namespace ymd{

struct ImageSpan{
    void * data;
    uint16_t w;
    uint16_t h;
    RgbType type;
};


template<typename ColorType>
class ImageView:
    public ImageReadableIntf<ColorType>, 
    public ImageWritableIntf<ColorType>{
public:
protected:
    using m_Image = ImageWR<ColorType>;
    m_Image & instance;
    Rect2u window;
public:
    ImageView(m_Image & _instance):instance(_instance){}
    ImageView(m_Image & _instance, const Rect2u & _window):
        instance(_instance), window(_window){;}

    ImageView(ImageView & other, const Rect2u & _window):
        instance(other.instance), 
        window(Rect2u(other.window.position + _window.position, other.window.size)
        .intersection(Vec2u(), other.instance.get_size())){;}
    Rect2u rect() const {return window;}
};

}
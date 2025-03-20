#pragma once

#include "drivers/Display/DisplayerPhy.hpp"
#include "types/image/PackedImage.hpp"

namespace ymd::drivers{

class SSD13XX:public Displayer<Binary>{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
protected:
    DisplayerPhy & interface;
    SSD13XX(DisplayerPhy & _interface):Displayer(size_), interface(_interface){;}

    void setarea_unsafe(const Rect2i & area) {
        setpos_unsafe(area.position);
    }

    void putpixel_unsafe(const Vector2i & pos, const Binary color){
        auto & frame = fetch_frame();
        frame.putpixel_unsafe(pos, color);
    }

    void setpos_unsafe(const Vector2i & pos) ;

    virtual Vector2i get_offset() const = 0;

    virtual void preinitByCmds() = 0;

    virtual void setFlushPos(const Vector2i & pos){
        auto [x, y] = pos + get_offset();
        interface.write_command(0xb0 | size_t(y / 8));
        interface.write_command(((x & 0xf0 )>>4) |0x10);
        interface.write_command((x & 0x0f));
    }


    void setOffset(){
        interface.write_command(0xD3); 
        interface.write_command(get_offset().y);
    }
public:
    scexpr uint8_t default_id = 0x78;

    void init();

    void update();

    void enable(const bool en = true){
        if(en){
            interface.write_command(0x8D);
            interface.write_command(0x14);
            interface.write_command(0xAF);
        }else{
            interface.write_command(0x8D);
            interface.write_command(0x10);
            interface.write_command(0xAE);
        }
    }

    void turnDisplay(const bool i){
        interface.write_command(0xC8 - 8*uint8_t(i));//正常显示
        interface.write_command(0xA1 - uint8_t(i));
    }

    void enable_flip_y(const bool flip = true){interface.write_command(0xA0 | flip);}
    void enable_flip_x(const bool flip = true){interface.write_command(0xC0 | (flip << 3));}
    void enable_inversion(const bool inv = true){interface.write_command(0xA7 - inv);}  

    virtual VerticalBinaryImage & fetch_frame() = 0;

};

class SSD13XX_72X40:public SSD13XX{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
protected:
    scexpr Vector2i phy_size = Vector2i(72, 40);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds();

    Vector2i get_offset() const {
        return {28, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetch_frame() {return frame_instance;};
    SSD13XX_72X40(DisplayerPhy & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_128X64:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(128, 64);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);


    void preinitByCmds() ;
    Vector2i get_offset() const {
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() {return frame_instance;};
    SSD13XX_128X64(DisplayerPhy & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};



class SSD13XX_128X32:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(128, 32);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() ;
    void setFlushPos(const Vector2i & pos) {
        auto [x, y] = pos + get_offset();
        interface.write_command(0xb0 | (y / 8));
        interface.write_command(((x & 0xf0 )>>4) |0x10);
        interface.write_command((x & 0x0f));
    }

    Vector2i get_offset() const {
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() {return frame_instance;};
    SSD13XX_128X32(DisplayerPhy & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_88X48:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(88, 48);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() ;

    Vector2i get_offset() const {
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() {return frame_instance;};
    SSD13XX_88X48(DisplayerPhy & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_64X48:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(64, 48);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() ;

    Vector2i get_offset() const {
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() {return frame_instance;};
    SSD13XX_64X48(DisplayerPhy & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_128X80:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(128, 80);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() ;

    Vector2i get_offset() const {
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() {return frame_instance;};
    SSD13XX_128X80(DisplayerPhy & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};

};

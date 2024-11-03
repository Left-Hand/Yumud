#pragma once

#include "../../DisplayerInterface.hpp"
#include "types/image/packed_image.hpp"

namespace yumud::drivers{

class SSD13XX:public Displayer<Binary>{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
protected:
    DisplayerInterface & interface;
    SSD13XX(DisplayerInterface & _interface):Displayer(size), interface(_interface){;}

    void setarea_unsafe(const Rect2i & area) override{
        setpos_unsafe(area.position);
    }

    void putpixel_unsafe(const Vector2i & pos, const Binary & color){
        auto & frame = fetchFrame();
        frame.putpixel_unsafe(pos, color);
    }

    void setpos_unsafe(const Vector2i & pos) override{
        auto & frame = fetchFrame();
        frame.setpos(pos);
    }

    virtual Vector2i getOffset() const = 0;

    virtual void preinitByCmds() = 0;

    virtual void setFlushPos(const Vector2i & pos){
        auto [x, y] = pos + getOffset();
        interface.writeCommand(0xb0 | (y / 8));
        interface.writeCommand(((x & 0xf0 )>>4) |0x10);
        interface.writeCommand((x & 0x0f));
    }


    void setOffset(){
        interface.writeCommand(0xD3); 
        interface.writeCommand(getOffset().y);
    }
public:
    scexpr uint8_t default_id = 0x78;

    void init();

    void update();

    void enable(const bool en = true){
        if(en){
            interface.writeCommand(0x8D);
            interface.writeCommand(0x14);
            interface.writeCommand(0xAF);
        }else{
            interface.writeCommand(0x8D);
            interface.writeCommand(0x10);
            interface.writeCommand(0xAE);
        }
    }

    void turnDisplay(const bool i){
        interface.writeCommand(0xC8 - 8*i);//正常显示
        interface.writeCommand(0xA1 - i);
    }

    void enableFlipY(const bool flip = true){interface.writeCommand(0xA0 | flip);}
    void enableFlipX(const bool flip = true){interface.writeCommand(0xC0 | (flip << 3));}
    void enableInversion(const bool inv = true){interface.writeCommand(0xA7 - inv);}  

    virtual VerticalBinaryImage & fetchFrame() = 0;

};

class SSD13XX_72X40:public SSD13XX{
public:
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
protected:
    scexpr Vector2i phy_size = Vector2i(72, 40);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() override;

    Vector2i getOffset() const override{
        return {28, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() override{return frame_instance;};
    SSD13XX_72X40(DisplayerInterface & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_128X64:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(128, 64);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);


    void preinitByCmds() override;
    Vector2i getOffset() const override{
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() override{return frame_instance;};
    SSD13XX_128X64(DisplayerInterface & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};



class SSD13XX_128X32:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(128, 32);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() override;
    void setFlushPos(const Vector2i & pos) override{
        auto [x, y] = pos + getOffset();
        interface.writeCommand(0xb0 | (y / 8));
        interface.writeCommand(((x & 0xf0 )>>4) |0x10);
        interface.writeCommand((x & 0x0f));
    }

    Vector2i getOffset() const override{
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() override{return frame_instance;};
    SSD13XX_128X32(DisplayerInterface & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_88X48:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(88, 48);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() override;

    Vector2i getOffset() const override{
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() override{return frame_instance;};
    SSD13XX_88X48(DisplayerInterface & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_64X48:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(64, 48);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() override;

    Vector2i getOffset() const override{
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() override{return frame_instance;};
    SSD13XX_64X48(DisplayerInterface & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};


class SSD13XX_128X80:public SSD13XX{
protected:
    scexpr Vector2i phy_size = Vector2i(128, 80);
    VerticalBinaryImage frame_instance = VerticalBinaryImage(phy_size);

    void preinitByCmds() override;

    Vector2i getOffset() const override{
        return {2, 0};
    }

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() override{return frame_instance;};
    SSD13XX_128X80(DisplayerInterface & _interface):ImageBasics(phy_size), SSD13XX(_interface){;}
};

};

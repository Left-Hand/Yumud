#ifndef __SSD1306_HPP__
#define __SSD1306_HPP__

#include "../../DisplayerInterface.hpp"
#include "types/image/packedImage.hpp"

// class OldeDisplayer : public D
class SSD13XX:public Displayer<Binary>{
protected:
    DisplayerInterface & interface;
    uint16_t width = 72;
    uint16_t height = 40;
    uint16_t x_offset = 0;

    void setarea_unsafe(const Rect2i & area){
        setpos_unsafe(area.position);
    }

    virtual void preinitByCmds() = 0;


    SSD13XX(DisplayerInterface & _interface):Displayer(size), interface(_interface){;}

    virtual void setFlushPos(const Vector2i & pos) = 0;

    void putsegv8(const Vector2i & pos, const uint8_t & mask, const Binary & color) override{
        fetchFrame().putsegv8(pos, mask, color);
    }
public:
    static constexpr uint8_t default_id = 0x78;

    void init();
    void flush(const Binary & color);

    void update();

    void setOffsetX(const uint8_t & offset){x_offset = offset;}
    void setOffsetY(const uint8_t & offset){
        interface.writeCommand(0xD3); 
        interface.writeCommand(offset);
    }
    void enable(const bool & en = true){
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

    void turnDisplay(const bool & i){
        interface.writeCommand(0xC8 - 8*i);//正常显示
        interface.writeCommand(0xA1 - i);
    }

    void enableFlipY(const bool & flip = true){interface.writeCommand(0xA0 | flip);}
    void enableFlipX(const bool & flip = true){interface.writeCommand(0xC0 | (flip << 3));}
    void enableInversion(const bool & inv = true){interface.writeCommand(0xA7 - inv);}  

    virtual VerticalBinaryImage & fetchFrame() = 0;

};

class SSD13XX_72X40:public SSD13XX{
protected:
    static constexpr Vector2i phy_size = Vector2i(72, 40);
    PackedBinary frame_buf[phy_size.x*phy_size.y / 8];
    VerticalBinaryImage frame_instance = VerticalBinaryImage(frame_buf, phy_size);



    void putpixel_unsafe(const Vector2i & pos, const Binary & color){
        auto & frame = fetchFrame();
        frame.putpixel_unsafe(pos, color);
    }

    void setpos_unsafe(const Vector2i & pos) override{
        auto & frame = fetchFrame();
        frame.setpos_unsafe(pos);
    }

    void setFlushPos(const Vector2i & pos) override{
        auto [x, y] = pos;
        x+=28;
        interface.writeCommand(0xb0 + (y / 8));
        interface.writeCommand(((x & 0xf0 )>>4) |0x10);
        interface.writeCommand((x & 0x0f));
    }
    void preinitByCmds() override;

    friend class VerticalBinaryImage;
public:

    VerticalBinaryImage & fetchFrame() override{return frame_instance;};
    SSD13XX_72X40(DisplayerInterface & _interface):ImageBasics<Binary>(phy_size), SSD13XX(_interface){;}
};


#endif
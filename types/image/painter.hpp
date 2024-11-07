#pragma once

#include "painter_concept.hpp"
#include "sys/string/gbk.hpp"


namespace yumud{

template<typename ColorType>
class Painter:public PainterConcept{
protected:
    using Vector2 = Image<ColorType>::Vector2;
    using Vector2i = Image<ColorType>::Vector2i;

    ImageWritable<ColorType> * src_image = nullptr;


    void drawtexture_unsafe(const Rect2i & rect,const ColorType * color_ptr){
        src_image -> puttexture_unsafe(rect, color_ptr);
    }

    void drawStr(const Vector2i & pos, const char * str_ptr, const size_t str_len) override{
        GBKIterator iterator(str_ptr);

        for(int x = pos.x; x < src_image->size.x;){
            if(iterator){
                auto chr = iterator.next();
                drawChar(Vector2i(x, pos.y), chr);
                auto font = chr > 0x80 ? chfont : enfont;
                if(font){
                    x += font->getSize().x + padding;
                }
            }else{
                break;
            }
        }
    }
public:
    Painter():PainterConcept(){;}


    Rect2i getClipWindow() override {
        return src_image->get_view();
    }

    void bindImage(ImageWritable<ColorType> & _source){
        src_image = &_source;
    }

    void setFontScale(const uint8_t scale){
        chfont->setScale(scale);
        enfont->setScale(scale);
    }

    void drawTextureRect(const Rect2i & rect,const ColorType * color_ptr){
        if(!src_image->getDisplayArea().contains(rect)) return;
        drawtexture_unsafe(rect, color_ptr);
    }

    template<typename w_ColorType>
    void drawImage(ImageWithData<w_ColorType, w_ColorType> & image, const Vector2i & pos = Vector2i(0,0)){
        if(!src_image->get_view().contains(image.get_view()) || image.data == nullptr) return;
        auto rect = Rect2i(pos, image.get_size());
        src_image->setarea_unsafe(rect);
        uint32_t i = 0;
        w_ColorType * ptr = image.data.get();
        for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++)
            for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++, i++)
                src_image->putpixel_unsafe(Vector2i(x,y), ptr[i]);
    }

    void drawFilledRect(const Rect2i & rect) override {
        Rect2i rect_area = src_image->get_view().intersection(rect);
        if(!rect_area) return;
        
        //FIXME use rect_area rather than rect will cause crash 
        src_image -> putrect_unsafe(rect, m_color);
    }

    void drawPixel(const Vector2i & pos) override {
        src_image -> putpixel(pos, m_color);
    }

    void drawLine(const Vector2i & from, const Vector2i & to) override{
        if(!src_image->has_point(from)){
            // ASSERT_WITH_HALT(false, "start point lost: ", from);
            return;
        }else if(!src_image->has_point(to)){
            // ASSERT_WITH_HALT(false, "end point lost: ", to);
            return;
        }

        auto [x0, y0] = from;
        auto [x1, y1] = to;

        if(y0 == y1) return drawHriLine(from, x1 - x0);
        if(x0 == x1) return drawVerLine(from, y1 - y0);
        bool steep = false;

        if (ABS(x1 - x0) < ABS(y1 - y0)) {
            SWAP(x0, y0);
            SWAP(x1, y1);
            steep = true;
        }

        if (x0 > x1) {
            SWAP(x0, x1);
            SWAP(y0, y1);
        }

        int dx = x1 - x0;
        int dy = y1 - y0;
        int deltaY = ABS(dy << 1);
        int middle = dx;
        int y = y0;
        for (int x = x0; x <= x1; ++x) {
            if (steep) {
                drawPixel({y,x});
            }
            else {
                drawPixel({x,y});
            }
            deltaY += ABS(dy << 1);
            if (deltaY >= middle) {
                y += (y1 > y0 ? 1 : -1);
                middle += ABS(dx << 1);
            }
        }
    }
    
    void drawChar(const Vector2i & pos,const wchar_t chr) override {
        const Font * font = chr > 0x80 ? chfont : enfont;
        if(font == nullptr) return;
        
        Rect2i image_area = Rect2i(Vector2i{}, src_image->size);
        const Vector2i font_size = font->getSize();
        Rect2i char_area = Rect2i(pos, font_size).intersection(image_area);

        if(!char_area) return;
        
        for(int i = char_area.position.x; i < char_area.position.x + char_area.size.x ; i++){
            uint8_t mask = 0;
            for(int j = 0; j < font_size.y; j++){
 
                if(j % 8 == 0) mask = 0;

                Vector2i offs = Vector2i(i - char_area.position.x ,j);
                if(font->getpixel(chr, offs)){
                    mask |= (0x01 << (j % 8));
                }

                if(j % 8 == 7 || j == font_size.y - 1){
                    src_image->putseg_v8_unsafe(Vector2i(i, (j & (~(8 - 1))) + pos.y), mask, m_color);
                    mask = 0;
                }
            }
        }
    }



};

}
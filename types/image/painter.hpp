#pragma once

#include "PainterConcept.hpp"
#include "core/string/encoding/gbk.hpp"


namespace ymd{

template<typename ColorType>
class Painter:public PainterConcept{
protected:

    ImageWritable<ColorType> * src_image = nullptr;
    
    using Error = typename PainterConcept::Error;
    template<typename T = void>
    using IResult = PainterConcept::IResult<T>;

    void drawtexture_unsafe(const Rect2u & rect,const ColorType * color_ptr){
        src_image -> puttexture_unsafe(rect, color_ptr);
    }

    __no_inline Result<void, Error> draw_str(const Vector2u & pos, const char * str_ptr, const size_t str_len) override{
        GBKIterator iterator(str_ptr);

        for(size_t x = pos.x;;){
            if(x >= src_image->size().x){
                // FIXME: in tsubst_copy, at cp/pt.cc:17004
                // return Err(Error::StringLengthTooLong);
                return Ok();
            }

            if(iterator){
                auto chr = iterator.next();
                draw_char(Vector2u(x, pos.y), chr);
                auto font = chr > 0x80 ? chfont : enfont;
                if(font){
                    x += font->getSize().x + padding;
                }else{
                    //FIXME: in tsubst_copy, at cp/pt.cc:17004
                    // return Err(Error::NoFontFounded);
                    return Ok();
                }
            }else{
                break;
            }
        }

        return Ok();
    }
public:
    Painter():PainterConcept(){;}


    Rect2u get_clip_window() override {
        return src_image->rect();
    }

    void bind_image(ImageWritable<ColorType> & _source){
        src_image = &_source;
    }

    void set_font_scale(const uint8_t scale){
        chfont->setScale(scale);
        enfont->setScale(scale);
    }

    void draw_texture_rect(const Rect2u & rect,const ColorType * color_ptr){
        if(!src_image->getDisplayArea().contains(rect)) return;
        drawtexture_unsafe(rect, color_ptr);
    }

    template<typename w_ColorType>
    void draw_image(ImageWithData<w_ColorType, w_ColorType> & image, const Vector2u & pos = Vector2u(0,0)){
        if(!src_image->get_view().contains(image.get_view()) || image.data == nullptr) return;
        auto rect = Rect2u(pos, image.size());
        src_image->setarea_unsafe(rect);
        uint32_t i = 0;
        w_ColorType * ptr = image.data.get();
        for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++)
            for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++, i++)
                src_image->putpixel_unsafe(Vector2u(x,y), ptr[i]);
    }

    [[nodiscard]]
    IResult<> draw_filled_rect(const Rect2u & rect) override {
        Rect2u ins = src_image->rect().intersection(rect);
        if(ins.get_area() == 0) return Ok();
        
        //FIXME use ins rather than rect will cause crash 
        src_image -> putrect_unsafe(rect, m_color);

        return Ok();
    }

    void draw_pixel(const Vector2u & pos) override {
        // src_image -> putpixel(pos, Binary(Binary::WHITE));
        src_image -> putpixel(pos, m_color);
    }

    [[nodiscard]]
    IResult<> draw_line(const Vector2u & from, const Vector2u & to) override{
        // if(!src_image->size().has_point(from)){
        //     return Err(Error(Error::StartPointOutOfBound));
        // }
        
        // if(!src_image->size().has_point(to)){
        //     return Err(Error(Error::EndPointOutOfBound));
        // }

        auto [x0, y0] = from;
        auto [x1, y1] = to;

        if(y0 == y1) return draw_hri_line(from, x1 - x0);
        if(x0 == x1) return draw_ver_line(from, y1 - y0);
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
        for (int x = x0; x <= int(x1); ++x) {
            if (steep) {
                draw_pixel({size_t(y),size_t(x)});
            } else {
                draw_pixel({size_t(x),size_t(y)});
            }
            deltaY += ABS(dy << 1);
            if (deltaY >= middle) {
                y += (y1 > y0 ? 1 : -1);
                middle += ABS(dx << 1);
            }
        }

        return Ok();
    }
    
    [[nodiscard]]
    IResult<> draw_char(const Vector2u & pos,const wchar_t chr) override {
        const Font * font = chr > 0x80 ? chfont : enfont;
        // if(font == nullptr){
        //     return Err(Error::NoChineseFontFounded);
        // } 
        
        if(font == nullptr) return Ok();

        Rect2u image_area = Rect2u{Vector2u{0,0}, src_image->size()};
        const Vector2u font_size = font->getSize();
        Rect2u char_rect = Rect2u(pos, font_size).intersection(image_area);

        if(char_rect.get_area() == 0) return Ok();
        
        for(size_t i = char_rect.position.x; i < char_rect.position.x + char_rect.size.x ; i++){
            uint8_t mask = 0;
            for(size_t j = 0; j < font_size.y; j++){
 
                if(j % 8 == 0) mask = 0;

                Vector2u offs = Vector2u(i - char_rect.position.x ,j);
                if(font->getpixel(chr, offs)){
                // if(true){
                    mask |= (0x01 << (j % 8));
                }

                if(j % 8 == 7 || j == font_size.y - 1){
                    src_image->putseg_v8_unsafe(Vector2u(i, (j & (~(8 - 1))) + pos.y), mask, m_color);
                    mask = 0;
                }
            }
        }

        return Ok();
    }



};

}
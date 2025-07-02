#pragma once

#include "painter_base.hpp"
#include "core/string/encoding/gbk.hpp"
#include "core/utils/Option.hpp"


namespace ymd{

template<typename ColorType>
class Painter:public PainterBase{
public:
    using Error = typename PainterBase::Error;
    
    template<typename T = void>
    using IResult = PainterBase::IResult<T>;

    Painter():PainterBase(){;}


    Option<Rect2u> get_expose_rect(){
        return may_src_image_.map([](const Image<ColorType> & image){
            return image.size().to_rect();
        });
    }

    void set_font_scale(const uint8_t scale){
        if(may_chfont_.is_some())
            may_chfont_.unwrap().set_scale(scale);
        if(may_enfont_.is_some())
            may_enfont_.unwrap().set_scale(scale);
    }

    #if 0
    void draw_texture_rect(const Rect2u & rect,const ColorType * color_ptr){
        if(!may_src_image->size().to_rect().contains(rect)) return;
        drawtexture_unchecked(rect, color_ptr);
    }

    template<typename w_ColorType>
    void draw_image(Image<w_ColorType> & image, const Vector2u & pos = Vector2u(0,0)){
        if(!may_src_image->get_view().contains(image.get_view()) || image.data == nullptr) return;
        auto rect = Rect2u(pos, image.size());
        may_src_image->setarea_unchecked(rect);
        uint32_t i = 0;
        w_ColorType * ptr = image.data.get();
        for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++)
            for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++, i++)
                may_src_image->putpixel_unchecked(Vector2u(x,y), ptr[i]);
    }

    [[nodiscard]]
    IResult<> draw_filled_rect(const Rect2u & rect){
        const auto area = may_src_image->size().to_rect()
            .intersection(rect)
            .map([](const Rect2u & _rect){return _rect.get_area();})
            .unwrap_or(0);
        if(area == 0) return Ok();
        
        //FIXME use ins rather than rect will cause crash 
        src_image -> putrect_unchecked(rect, m_color);

        return Ok();
    }


    void draw_pixel(const Vector2u & pos){
        // src_image -> putpixel(pos, Binary(Binary::WHITE));
        src_image -> putpixel(pos, m_color);
    }

    [[nodiscard]]
    IResult<> draw_line(const Vector2u & from, const Vector2u & to){
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
    IResult<> draw_char(const Vector2u & pos,const wchar_t chr){
        const Font * font = chr > 0x80 ? chfont : enfont;
        // if(font == nullptr){
        //     return Err(Error::NoChineseFontFounded);
        // } 
        
        if(font == nullptr) return Ok();

        const Rect2u image_area = Rect2u{Vector2u{0,0}, src_image->size()};
        const Vector2u font_size = font->getSize();
        const auto char_rect_opt = Rect2u(pos, font_size)
            .intersection(image_area);
        if(char_rect_opt.is_none()) return Ok();
        const auto char_rect = char_rect_opt.unwrap();

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
                    src_image->putseg_v8_unchecked(Vector2u(i, (j & (~(8 - 1))) + pos.y), mask, m_color);
                    mask = 0;
                }
            }
        }

        return Ok();
    }
    #else
    void draw_texture_rect(const Rect2u & rect,const ColorType * color_ptr){
        TODO();
    }

    template<typename w_ColorType>
    void draw_image(Image<w_ColorType> & image, const Vector2u & pos = Vector2u(0,0)){
        TODO();
    }

    [[nodiscard]]
    IResult<> draw_filled_rect(const Rect2u & rect){
        TODO();
        return Ok();
    }


    void draw_pixel(const Vector2u & pos){
        return;
    }

    [[nodiscard]]
    IResult<> draw_line(const Vector2u & from, const Vector2u & to){
        TODO();
        return Ok();
    }
    
    [[nodiscard]]
    IResult<> draw_char(const Vector2u & pos,const wchar_t chr){
        // const auto err = Error(Error::Unfinished);
        // return Err(err);
        TODO();
        return Ok();
    }
    #endif
private:

    Option<Image<ColorType> &> may_src_image_ = None;
    


    void drawtexture_unchecked(const Rect2u & rect,const ColorType * color_ptr){
        may_src_image_ -> puttexture_unchecked(rect, color_ptr);
    }

    __no_inline IResult<> draw_str(
        const Vector2u & pos, 
        const StringView str)
    {
        GBKIterator iterator(str);

        if(may_src_image_.is_none())
            return Err(Error::ImageNotSet);
        if(may_chfont_.is_none())
            return Err(Error::NoChineseFontFounded);
        if(may_enfont_.is_none())
            return Err(Error::NoEnglishFontFounded);

        auto & src_image = may_src_image_.unwrap();
        auto & chfont = may_chfont_.unwrap();
        auto & enfont = may_enfont_.unwrap();

        for(size_t x = pos.x;;){
            if(x >= src_image.size().x){
                // FIXME: in tsubst_copy, at cp/pt.cc:17004
                // return Err(Error::StringLengthTooLong);
                return Ok();
            }

            if(iterator){
                const auto chr = iterator.next();
                if(const auto res = draw_char(Vector2u(x, pos.y), chr);
                    res.is_err()) return res;
                const auto & font = chr > 0x80 ? chfont : enfont;
                x += font.get_size().x + padding_;
            }else{
                break;
            }
        }

        return Ok();
    }
};

}
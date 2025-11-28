#pragma once

#include "painter_base.hpp"
#include "core/string/encoding/gbk.hpp"
#include "core/string/encoding/ascii.hpp"
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
        if(may_src_image_.is_none()) return None;
        return Some(Rect2u::from_size(may_src_image_.unwrap().size()));
    }

    void set_font_scale(const uint8_t scale){
        if(may_chfont_.is_some())
            may_chfont_.unwrap().set_scale(scale);
        if(may_enfont_.is_some())
            may_enfont_.unwrap().set_scale(scale);
    }

    #if 0
    void draw_texture_rect(const Rect2u & rect,const ColorType * color_ptr){
        if(!may_src_imagesize().to_rect().contains(rect)) return;
        drawtexture_unchecked(rect, color_ptr);
    }

    template<typename w_ColorType>
    void draw_image(Image<w_ColorType> & image, const Vec2u & pos = Vec2u(0,0)){
        if(!may_src_imageget_view().contains(image.get_view()) || image.data == nullptr) return;
        auto rect = Rect2u(pos, image.size());
        may_src_imagesetarea_unchecked(rect);
        uint32_t i = 0;
        w_ColorType * ptr = image.data.get();
        for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++)
            for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++, i++)
                may_src_imageputpixel_unchecked(Vec2u(x,y), ptr[i]);
    }

    [[nodiscard]]
    IResult<> draw_char(const Vec2u & pos,const wchar_t chr){
        const Font * font = chr > 0x80 ? chfont : enfont;
        // if(font == nullptr){
        //     return Err(Error::NoChineseFontFounded);
        // } 
        
        if(font == nullptr) return Ok();

        const Rect2u image_area = Rect2u{Vec2u{0,0}, src_image->size()};
        const Vec2u font_size = font->getSize();
        const auto char_rect_opt = Rect2u(pos, font_size)
            .intersection(image_area);
        if(char_rect_opt.is_none()) return Ok();
        const auto char_rect = char_rect_opt.unwrap();

        for(size_t i = char_rect.position.x; i < char_rect.position.x + char_rect.size.x ; i++){
            uint8_t mask = 0;
            for(size_t j = 0; j < font_size.y; j++){
 
                if(j % 8 == 0) mask = 0;

                Vec2u offs = Vec2u(i - char_rect.position.x ,j);
                if(font->getpixel(chr, offs)){
                // if(true){
                    mask |= (0x01 << (j % 8));
                }

                if(j % 8 == 7 || j == font_size.y - 1){
                    src_image->putseg_v8_unchecked(Vec2u(i, (j & (~(8 - 1))) + pos.y), mask, );
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
    void draw_image(Image<w_ColorType> & image, const Vec2u & pos = Vec2u(0,0)){
        TODO();
    }

    [[nodiscard]]
    IResult<> draw_filled_rect(const Rect2u & rect){
        if(may_src_image_.is_none()) 
            return Err(Error(Error::Kind::ImageNotSet));
        auto & src_image = may_src_image_.unwrap();
        const Rect2u region = ({
            const auto may_region = Rect2u::from_size(src_image.size())
                .intersection(rect);
            if(may_region.is_none()) return Ok();
            may_region.unwrap();
        });
        
        if(region.area() == 0) return Ok();
        
        for(size_t y = region.y(); y < region.y() + region.h(); y++){
            for(size_t x = region.x(); x < region.x() + region.w(); ++x){
                src_image.putpixel_unchecked({x,y}, color_cast<ColorType>(color_));
            }
        }

        return Ok();
    }

    void putpixel_unchecked(const Vec2<uint16_t> pos){
        auto & src_image = may_src_image_.unwrap();
        if(not src_image.size().has_point(pos)) return;
        src_image.putpixel_unchecked(pos, color_cast<ColorType>(color_));
    }

    [[nodiscard]]
    IResult<> draw_line(const Vec2u & from, const Vec2u & to){
        if(may_src_image_.is_none()) 
            return Err(Error(Error::Kind::ImageNotSet));

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
                putpixel_unchecked({uint16_t(y),uint16_t(x)});
            } else {
                putpixel_unchecked({uint16_t(x),uint16_t(y)});
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
    IResult<> draw_wchar(const Vec2u & pos, const wchar_t chr){
        if(may_src_image_.is_none())
            return Err(Error(Error::Kind::ImageNotSet));
        if(may_enfont_.is_none())
            return Err(Error(Error::Kind::NoEnglishFontFounded));
        auto & src_image = may_src_image_.unwrap();
        auto & enfont = may_enfont_.unwrap();

        const auto enfont_size = enfont.get_size();
        if(not src_image.size().has_point(pos + enfont_size))
            return Ok();

        for(size_t x = 0; x < enfont_size.x; x++){
            for(size_t y = 0; y < enfont_size.y; y++){
                if(enfont.get_pixel(chr, {uint8_t(x),uint8_t(y)})){
                    src_image.putpixel_unchecked(
                        pos + Vec2u{x, y}, 
                        color_cast<ColorType>(color_)
                    );
                }
            }
        }

        return Ok();
    }

    __no_inline IResult<> draw_ascii_str(
        const Vec2u & pos, 
        const StringView str)
    {
        AsciiIterator iter(str);
        if(may_src_image_.is_none())
            return Err(Error(Error::Kind::ImageNotSet));
        if(may_enfont_.is_none())
            return Err(Error(Error::Kind::NoEnglishFontFounded));

        auto & src_image = may_src_image_.unwrap();
        auto & enfont = may_enfont_.unwrap();

        for(size_t x = pos.x;;){
            if(x >= src_image.size().x){
                return Err(Error(Error::Kind::StringLengthTooLong));
            }

            if(not iter.has_next()) break;

            const auto chr = iter.next();
            if(const auto res = draw_wchar(Vec2u(x, pos.y), chr);
                res.is_err()) return res;
            x += (enfont.get_size().x + padding_);
        }

        return Ok();
    }

    __no_inline IResult<> draw_gbk_str(
        const Vec2u & pos, 
        const StringView str)
    {
        GbkIterator iter(str);

        if(may_src_image_.is_none())
            return Err(Error(Error::Kind::ImageNotSet));
        if(may_chfont_.is_none())
            return Err(Error(Error::Kind::NoChineseFontFounded));
        if(may_enfont_.is_none())
            return Err(Error(Error::Kind::NoEnglishFontFounded));

        auto & src_image = may_src_image_.unwrap();
        auto & chfont = may_chfont_.unwrap();
        auto & enfont = may_enfont_.unwrap();

        for(size_t x = pos.x;;){
            if(x >= src_image.size().x){
                return Err(Error(Error::Kind::StringLengthTooLong));
            }

            if(not iter.has_next()) break;

            const auto chr = iter.next();
            if(const auto res = draw_wchar(Vec2u(x, pos.y), chr);
                res.is_err()) return res;
            const auto & font = chr > 0x80 ? chfont : enfont;
            x += font.get_size().x + padding_;

        }

        return Ok();
    }
    #endif

    void set_src_image(Some<Image<ColorType> *> src_image){
        may_src_image_ =  std::move(src_image);
    }
private:

    Option<Image<ColorType> &> may_src_image_ = None;
    
    void drawtexture_unchecked(const Rect2u & rect,const ColorType * color_ptr){
        may_src_image_.unwrap().puttexture_unchecked(rect, color_ptr);
    }


};

}
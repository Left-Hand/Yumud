#pragma once

#include "types/vectors/vector2.hpp"
#include "font_res.hpp"

namespace ymd{

class Font{
protected:

    Vec2<uint8_t> size;
    uint8_t scale;
    public:
    constexpr Font(Vec2<uint8_t> _size, uint8_t _scale = 1):size(_size), scale(_scale){;}
    
    virtual bool get_pixel(const wchar_t chr, const Vec2<uint8_t> offset) const = 0;
    void set_scale(const uint8_t _scale){scale = _scale;}
    Vec2<uint8_t> get_size() const { return size * scale; }
};




class Font8x5:public Font{
    bool get_pixel(const wchar_t chr, const Vec2<uint8_t> offset) const override{
        if (!size.has_point(offset)) return false;
        return font_res::enfont_8x5[MAX(chr - ' ', 0)][offset.x + 1] & (1 << offset.y);
    }
public:
    constexpr Font8x5():Font( Vec2<uint8_t>(5,8)){;}
};



class Font7x7:public Font{
protected:
    using font_item_t = font_res::chfont_7x7_item_t;
    const font_item_t * find_font_item(uint16_t code) const{
        size_t left = 0;
        const auto & res = font_res::chfont_7x7;
        size_t right = std::size(res) - 1;
        
        while (left <= right) {
            size_t mid = left + (right - left) / 2;
            
            if (res[mid].code == code) {
                return &res[mid];
            } else if (res[mid].code < code) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return nullptr;
    }
public:
    constexpr Font7x7():Font(Vec2<uint8_t>{7,7}){;}
	bool get_pixel(const wchar_t chr, const Vec2<uint8_t> offset) const override{
        // if(!size.has_point(offset)) return false;
        if(offset.y > 6) return false;
		static wchar_t last_chr = 0;
		static const font_item_t * last_font_ptr = nullptr;
		static uint8_t buf[7];

		if(chr != last_chr){
            last_font_ptr = find_font_item(chr);
            if(last_font_ptr){
                for(uint8_t i = 0; i < 7; i++){
                    buf[i] = last_font_ptr->data[i];
                }
            }else{
                for(uint8_t i = 0; i < 7; i++){
                    buf[i] = 0;
                }
            }
            last_chr = chr;
		}

		if(last_font_ptr){
			return buf[offset.y] & (0x80 >> offset.x);
		}else{
			return false;
		}
	}
};

class Font16x8:public Font{
public:
    static constexpr auto & RES = font_res::enfont_16x8;
    constexpr Font16x8():Font(Vec2<uint8_t>(8,16)){;}
    bool get_pixel(const wchar_t chr, const Vec2<uint8_t> offset) const override{
        if (!size.has_point(offset)) return false;

        return RES[MAX(chr - ' ', 0)][offset.y] & (1 << offset.x);
    }
};

}

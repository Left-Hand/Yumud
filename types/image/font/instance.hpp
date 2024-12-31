#pragma once

#include "font.hpp"

#include "font8x5.hpp"
#include "font16x8.hpp"
#include "chfont7x7.hpp"


namespace ymd{
class Font8x5:public Font{
    bool _getpixel(const wchar_t chr, const Vector2i & offset) const override{
        if (!size.has_point(offset)) return false;
        return font8x5_enc[MAX(chr - ' ', 0)][offset.x + 1] & (1 << offset.y);
    }
public:
    constexpr Font8x5():Font( Vector2i(5,8)){;}
};

extern Font8x5 font8x5;



class Font7x7:public Font{
protected:
    const font_item_t * find_font_item(uint16_t code) const{
        size_t left = 0;
        size_t right = ARRSIZE(font_items) - 1;
        
        while (left <= right) {
            size_t mid = left + (right - left) / 2;
            
            if (font_items[mid].code == code) {
                return &font_items[mid];
            } else if (font_items[mid].code < code) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return nullptr;
    }
public:
    constexpr Font7x7():Font(Vector2i{7,7}){;}
	bool _getpixel(const wchar_t chr, const Vector2i & offset) const override{
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

extern Font7x7 font7x7;


class Font16x8:public Font{
public:
    constexpr Font16x8():Font(Vector2i(8,16)){;}
    bool _getpixel(const wchar_t chr, const Vector2i & offset) const override{
        if (!size.has_point(offset)) return false;
        return font16x8_enc[MAX(chr - ' ', 0)][offset.y] & (1 << offset.x);
    }
};

extern Font16x8 font16x8;
}
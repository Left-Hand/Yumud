#ifndef __FONT_HPP__

#define __FONT_HPP__

#include "../../vector2/vector2_t.hpp"
#include "font8x5.h"
#include "font16x8.h"
#include "chfont7x7.h"

class Font{
protected:
    Vector2i size;
    uint8_t scale;
    virtual bool _getpixel(const wchar_t & chr, const Vector2i & offset) const = 0;
public:
    Font(Vector2i _size, uint8_t _scale = 1):size(_size), scale(_scale){;}
    bool getpixel(const wchar_t & chr, const Vector2i & offset) const{
        return _getpixel(chr, {offset.x / scale, offset.y / scale});
    }

    void setScale(const uint8_t & _scale){scale = _scale;}
    Vector2i getSize() const { return size * scale; }
};


class Font8x5:public Font{
    bool _getpixel(const wchar_t & chr, const Vector2i & offset) const override{
        if (!size.has_point(offset)) return false;
        return font8x5_enc[MAX(chr - ' ', 0)][offset.x + 1] & (1 << offset.y);
    }
public:
    Font8x5():Font( Vector2i(5,8)){;}
};

extern Font8x5 font8x5;


class GBKIterator {
private:
	const char* gbkString;
	int currentIndex;
	
public:
	GBKIterator(const char* gbkString) : gbkString(gbkString), currentIndex(0) {}
	
	bool hasNext() {
		return gbkString[currentIndex] != '\0';
	}
	
	int next() {
		if (!hasNext()) {
			return -1; // 如果已经到达字符串末尾，则返回-1
		}
		
		unsigned char ch = gbkString[currentIndex];
		int unicodeValue;
		
		if (ch < 0x80) {
			unicodeValue = ch;
		} else {
			unicodeValue = ((gbkString[currentIndex] & 0xFF) << 8) | (gbkString[currentIndex + 1] & 0xFF);
			++currentIndex;
		}
		
		++currentIndex;
		return unicodeValue;
	}
};

class Font7x7:public Font{
protected:
    const font_item_t * find_font_item(uint16_t code)const {
        size_t left = 0;
        size_t right = ARRSIZE(font_items) - 1;

        // if(code < 256){
        //     if(code >= 0x61) return &font_items[code - (0x61 - 44)];
        //     else if(code >= 0x30) return &font_items[code - (0x30 - 6)];
        //     else {
        //         uint8_t index = 0;
        //         switch(code){
        //         case 0x20:
        //             index = 0;
        //             break;
        //         case 0x21:
        //             index = 1;
        //             break;
        //         case 0x24:
        //             index = 2;
        //             break;
        //         case 0x26:
        //             index = 3;
        //             break;
        //         case 0x2c:
        //             index = 4;
        //             break;
        //         case 0x2E:
        //             index = 5;
        //             break;
        //         }
        //         return &font_items[index];
        //     }
        // }else{
        //     right = ARRSIZE(font_items) - 1;
        // }
        
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
	Font7x7():Font(Vector2i{7,7}){;}
	bool _getpixel(const wchar_t & chr, const Vector2i & offset) const override{
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

// class Font16x8:public Font{
// public:
//     Font16x8():Font((void **)font16x8_enc, Vector2i(8,16)){;}
//     bool getpixel(const wchar_t & chr, const Vector2i & offset) const override{
//         if (!size.has_point(offset)) return false;
//         return font16x8_enc[MAX(chr - ' ', 0)][offset.y] & (1 << offset.x);
//     }
// };

// extern Font16x8 font16x8;


#endif
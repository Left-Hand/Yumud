#pragma once

#include "algebra/vectors/vec2.hpp"
#include "font_res.hpp"

namespace ymd{

struct [[nodiscard]] Font{
protected:

    Vec2<uint8_t> size;
    uint8_t scale;
    public:
    constexpr Font(Vec2<uint8_t> _size, uint8_t _scale = 1):size(_size), scale(_scale){;}
    
    virtual bool get_pixel(const wchar_t token, const Vec2<uint8_t> offset) = 0;
    void set_scale(const uint8_t _scale){scale = _scale;}
    Vec2<uint8_t> get_size() const { return size * scale; }
};




struct [[nodiscard]] Font8x5:public Font{
    bool get_pixel(const wchar_t token, const Vec2<uint8_t> offset) const {
        if (!size.has_point(offset)) return false;
        return font_res::enfont_8x5[MAX(token - ' ', 0)][offset.x + 1] & (1 << offset.y);
    }
public:
    constexpr Font8x5():Font( Vec2<uint8_t>(5,8)){;}
};



struct [[nodiscard]] Font7x7 final:public Font{
    using font_item_t = font_res::chfont_7x7_item_t;

public:
    constexpr Font7x7():Font(Vec2<uint8_t>{7,7}){;}
	bool get_pixel(const wchar_t token, const Vec2<uint8_t> offset){
        // if(!size.has_point(offset)) return false;
        if(offset.y > 6) return false;

		if(token != last_token_){
            last_token_ = token;
            p_last_font_item_ = find_font_item(token);

            if(p_last_font_item_){
                for(uint8_t i = 0; i < 7; i++){
                    buf[i] = p_last_font_item_->data[i];
                }
            }else{
                for(uint8_t i = 0; i < 7; i++){
                    buf[i] = 0;
                }
            }

		}

		if(p_last_font_item_){
			return buf[offset.y] & (0x80 >> offset.x);
		}else{
			return false;
		}
	}

private:
    wchar_t last_token_ = 0;
    const font_item_t * p_last_font_item_ = nullptr;
    uint8_t buf[7];


    const font_item_t * find_font_item(wchar_t code) const{
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
};

struct [[nodiscard]] Font16x8:public Font{
public:
    static constexpr auto & RES = font_res::enfont_16x8;
    constexpr Font16x8():Font(Vec2<uint8_t>(8,16)){;}
    bool get_pixel(const wchar_t token, const Vec2<uint8_t> offset) const {
        if (!size.has_point(offset)) return false;

        return RES[MAX(token - ' ', 0)][offset.y] & (1 << offset.x);
    }
};


struct MonoFont7x7 final{
    using font_item_t = font_res::chfont_7x7_item_t;

    constexpr MonoFont7x7(){;}
	uint32_t get_row_pixels(const wchar_t token, const uint8_t row_nth) {

		if(token != last_token_){
            last_token_ = token;
            p_last_font_item_ = find_font_item(token);

            if(p_last_font_item_){
                for(uint8_t i = 0; i < 7; i++){
                    buf[i] = p_last_font_item_->data[i];
                }
            }
		}

		if(p_last_font_item_){
			return buf[row_nth];
		}else{
			return 0u;
		}
	}

    constexpr Vec2u16 size() const {
        return Vec2u16{7,7};
    }
private:
    wchar_t last_token_ = 0;
    const font_item_t * p_last_font_item_ = nullptr;
    uint8_t buf[7] = {0};


    const font_item_t * find_font_item(wchar_t code) const{
        size_t left = 0;
        const auto & res = font_res::chfont_7x7;
        size_t right = std::size(res) - 1;
        
        while (left <= right) {
            // size_t mid = left + ((right - left) >> 1);
            size_t mid = ((right + left) >> 1);
            
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
};

struct MonoFont8x5 final{
    constexpr MonoFont8x5() = default;
	static uint32_t get_row_pixels(const wchar_t token, const uint8_t row_nth) {
        auto & row_data = font_res::enfont_8x5[MAX(token - ' ', 0)];
        uint32_t row_mask = 0;

        for(uint8_t x = 0; x < 5; x++){
            row_mask |= uint32_t(bool(uint8_t(row_data[x]) & uint8_t(1 << row_nth))) << (x);
        }

        return row_mask;
	}


    static constexpr Vec2u16 size() {
        return Vec2u16{5,8};
    }
public:
};

struct MonoFont16x8 final{
    constexpr MonoFont16x8() = default;
	uint32_t get_row_pixels(const wchar_t token, const uint8_t row_nth) {
        auto & col_data = font_res::enfont_16x8[MAX(token - ' ', 0)];
        uint32_t row_mask = 0;

        row_mask = col_data[row_nth];

        return row_mask;
	}


    constexpr Vec2u16 size() const {
        return Vec2u16{8,16};
    }
public:
};
}

#pragma once

namespace etk{

struct EtkDataFrame{

};



struct EtkToken:public Reg8{
    using Reg8::operator=;

    uint8_t right:1;
    uint8_t left:1;
    uint8_t down:1;
    uint8_t up:1;
    uint8_t :4;

    operator Vector2i() const {
        return Vector2i{
            right - left,
            up - down
        };
    }
};



}
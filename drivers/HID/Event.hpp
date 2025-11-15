#pragma once

#include "core/utils/Option.hpp"
#include "types/vectors/vector2.hpp"



namespace ymd::drivers{

struct KeyPlacement{
    constexpr Option<uint8_t> row() const {
        if(may_row_ > 0) return Some(may_row_);
        else return None;
    }
    constexpr Option<uint8_t> col() const {
        if(may_col_ > 0) return Some(may_col_);
        else return None;
    }

    constexpr explicit KeyPlacement(
        Option<uint8_t> row, 
        Option<uint8_t> col
    ):
        may_row_(row.is_some() ? row.unwrap() : -1),
        may_col_(col.is_some() ? col.unwrap() : -1)
    {;}
private:
    int8_t may_row_;
    int8_t may_col_;
};

struct KeyEvent{
public:
    enum class Type{
        Press,
        Release,
        Hold,
        DoublePress
    };

    KeyPlacement placement;
    Type type;
};



// class CharKeyEvent{
// public:
//     constexpr Option<char> key() const {
//         return key_;
//     }

//     constexpr CharKeyEvent(Option<char> key):
//         key_(key)
//     {;}
// private:

//     Option<char> key_;
// };

// class MouseEvent{
// public:
//     constexpr Option<Vec2i> pos() const {
//         return pos_;
//     }

//     constexpr MouseEvent(Option<Vec2i> pos):
//         pos_(pos)
//     {;}
// private:

//     Option<Vec2i> pos_;
// };

}

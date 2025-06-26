#pragma once

#include "core/utils/Option.hpp"
#include "types/vectors/vector2/Vector2.hpp"



namespace ymd::drivers{

struct KeyPlacement{
    constexpr Option<uint8_t> row() const {
        if(row_ > 0) return Some(row_);
        else return None;
    }
    constexpr Option<uint8_t> col() const {
        if(row_ > 0) return Some(row_);
        else return None;
    }

    constexpr KeyPlacement(
        Option<uint8_t> row, 
        Option<uint8_t> col
    ):
        row_(row.is_some() ? row.unwrap() : -1),
        col_(col.is_some() ? col.unwrap() : -1)
    {;}
private:
    int8_t row_;
    int8_t col_;
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
//     constexpr Option<Vector2i> pos() const {
//         return pos_;
//     }

//     constexpr MouseEvent(Option<Vector2i> pos):
//         pos_(pos)
//     {;}
// private:

//     Option<Vector2i> pos_;
// };

}

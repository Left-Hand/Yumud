#include "painter.hpp"

// template<typename ColorType>
// void Painter<ColorType>::drawHriLine(const Rangei & x_range, const int y){
//     if(!x_range ||!src_image->get_window().get_y_range().has(y)) return;

//     // src_image -> putrect_unsafe(Rect2i(x_range, Rangei(y, y+1)), m_color);
// }

// template<typename ColorType>
// void Painter<ColorType>::drawHriLine(const Vector2i & pos,const int l){
//     Rangei x_range = src_image->get_window().get_x_range().intersection(Rangei(pos.x, pos.x + ABS(l)));
//     drawHriLine(x_range, pos.y);
// }

// template<typename ColorType>
// void Painter<ColorType>::drawVerLine(const Vector2i & pos,const int l){
//     Rangei y_range = src_image->get_window().get_y_range().intersection(Rangei(pos.y, pos.y + ABS(l)));
//     if(!y_range ||!src_image->get_window().get_x_range().has(pos.x)) return;

//     // DEBUG_PRINTLN(Rect2i(Rangei(pos.x,pos.x+1), y_range));
//     // src_image -> putrect_unsafe(Rect2i(Rangei(pos.x,pos.x+1), y_range), m_color);
// }

// template<typename ColorType>
// void Painter<ColorType>::drawVerLine(const Rangei & y_range, const int x){
//     if(!y_range ||!src_image -> get_window().get_x_range().has(x)) return;
//     src_image -> putrect_unsafe(Rect2i(Rangei(x,x+1), y_range), m_color);
// }
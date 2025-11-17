#include "painter.hpp"

using namespace ymd;


// void Painter::drawLine(const Vec2u & from, const Vec2u & to){
//     if(!src_image->size().has_point(from)){
//         // ASSERT_WITH_HALT(false, "start point lost: ", from);
//         return;
//     }else if(!src_image->size().has_point(to)){
//         // ASSERT_WITH_HALT(false, "end point lost: ", to);
//         return;
//     }

//     auto [x0, y0] = from;
//     auto [x1, y1] = to;

//     if(y0 == y1) return drawHriLine(from, x1 - x0);
//     if(x0 == x1) return drawVerLine(from, y1 - y0);
//     bool steep = false;

//     if (ABS(x1 - x0) < ABS(y1 - y0)) {
//         SWAP(x0, y0);
//         SWAP(x1, y1);
//         steep = true;
//     }

//     if (x0 > x1) {
//         SWAP(x0, x1);
//         SWAP(y0, y1);
//     }

//     int dx = x1 - x0;
//     int dy = y1 - y0;
//     int deltaY = ABS(dy << 1);
//     int middle = dx;
//     int y = y0;
//     for (int x = x0; x <= x1; ++x) {
//         if (steep) {
//             drawPixel({y,x});
//         }
//         else {
//             drawPixel({x,y});
//         }
//         deltaY += ABS(dy << 1);
//         if (deltaY >= middle) {
//             y += (y1 > y0 ? 1 : -1);
//             middle += ABS(dx << 1);
//         }
//     }
// }
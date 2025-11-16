#pragma once


#include "apriltag.hpp"
#include "dec_16h5.hpp"

namespace ymd::nvcv2{
class Apriltag16H5Recognizer final :public ApriltagRecognizerIntf{
protected:
    Apriltag16H5Decoder decoder_;
public:
    //4 * 4 = 16
    static constexpr uint APRILTAG_SIDE_COUNTS = 4;


    void update(
        const Image<Gray> & src, 
        const Rect2u roi
    ) override {
        const auto vertexs = find_vertex(src, Gray::from_white(), roi);

        uint16_t code = 0;
        for(uint j = 0; j < APRILTAG_SIDE_COUNTS; j++){
            for(uint i = 0; i < APRILTAG_SIDE_COUNTS; i++){
                uint16_t mask = (0x8000) >> (j * 4 + i);
                Gray val = get_vertex_val(vertexs, {i,j}, src);
                if(val.as_u8() > 173) code |= mask;
            }
        }

        // decoder_.update(code);
    }


    static constexpr Gray get_vertex_val(
        const Vertexs & _vertexs, 
        const Vec2<iq16> & _grid_pos, 
        const Image<Gray> & gs
    ){
        // TODO();
        while(true);
        return Gray::from_black();
        // return gs.bilinear_interpol(get_vertex_grid(_vertexs, _grid_pos));
    };

    static constexpr Vec2<iq16> get_vertex(const Vertexs & __vertexs, const Vec2<iq16> & __grid_pos){
        const Vec2<iq16> grid_scale = (__grid_pos + Vec2<iq16>{1,1}) / (APRILTAG_SIDE_COUNTS + 2);

        const Vec2<iq16> upper_x = __vertexs[0].lerp(__vertexs[1], grid_scale.x);
        const Vec2<iq16> lower_x = __vertexs[3].lerp(__vertexs[2], grid_scale.x);

        return upper_x.lerp(lower_x, grid_scale.y);
    };


    static constexpr Vec2<iq16> get_vertex_grid(const Vertexs & __vertexs, const Vec2<iq16> & __grid_pos){
        // return get_vertex(__vertexs, __grid_pos + Vec2<iq16>{0.5, 0.5});
        return Vec2<iq16>(0,0);
    };


    static constexpr Vertexs find_vertex(
        const Image<Gray> & __map, 
        const Gray & match, 
        const Rect2u & roi
    ){
        const auto x_range = roi.x_range();
        const auto y_range = roi.y_range();

        const auto center = roi.center();
        Vertexs ret{
            center, center, center, center
        };


        for(auto y = y_range.start; y < y_range.stop; ++y){
            for(auto x = x_range.start; x < x_range.stop; ++x){
                const auto color = __map[{x,y}];
                if(color != match) continue;

                if(static_cast<iq16>(-(x)) + (+(y)) < (-(ret[0].x)) + (-(ret[0].y))) ret[0] = Vec2u(x,y);
                if(static_cast<iq16>(-(x)) + (-(y)) < (-(ret[1].x)) + (+(ret[1].y))) ret[1] = Vec2u(x,y);
                if(static_cast<iq16>(+(x)) + (+(y)) < (+(ret[2].x)) + (-(ret[2].y))) ret[2] = Vec2u(x,y);
                if(static_cast<iq16>(+(x)) + (-(y)) < (+(ret[3].x)) + (+(ret[3].y))) ret[3] = Vec2u(x,y);
            }
        }

        #undef COMP

        return ret;
    };
};

}
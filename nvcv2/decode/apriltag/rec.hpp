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
        const Image<Grayscale> src, 
        const Rect2u roi
    ) override {
        const auto vertexs = find_vertex(src, Grayscale(255), roi);

        uint16_t code = 0;
        for(uint j = 0; j < APRILTAG_SIDE_COUNTS; j++){
            for(uint i = 0; i < APRILTAG_SIDE_COUNTS; i++){
                uint16_t mask = (0x8000) >> (j * 4 + i);
                Grayscale val = get_vertex_val(vertexs, {i,j}, src);
                if((uint8_t)val > 173) code |= mask;
            }
        }

        // decoder_.update(code);
    }


    static constexpr Grayscale get_vertex_val(
        const Vertexs & _vertexs, 
        const Vector2<q16> & _grid_pos, 
        const Image<Grayscale> & gs
    ){
        // TODO();
        while(true);
        return Grayscale(0);
        // return gs.bilinear_interpol(get_vertex_grid(_vertexs, _grid_pos));
    };

    static constexpr Vector2<q16> get_vertex(const Vertexs & __vertexs, const Vector2<q16> & __grid_pos){
        const Vector2<q16> grid_scale = (__grid_pos + Vector2<q16>{1,1}) / (APRILTAG_SIDE_COUNTS + 2);

        const Vector2<q16> upper_x = __vertexs[0].lerp(__vertexs[1], grid_scale.x);
        const Vector2<q16> lower_x = __vertexs[3].lerp(__vertexs[2], grid_scale.x);

        return upper_x.lerp(lower_x, grid_scale.y);
    };


    static constexpr Vector2<q16> get_vertex_grid(const Vertexs & __vertexs, const Vector2<q16> & __grid_pos){
        // return get_vertex(__vertexs, __grid_pos + Vector2<q16>{0.5, 0.5});
        return Vector2<q16>(0,0);
    };


    static constexpr Vertexs find_vertex(
        const Image<Grayscale> & __map, 
        const Grayscale & match, 
        const Rect2u & roi
    ){
        const auto x_range = roi.get_x_range();
        const auto y_range = roi.get_y_range();

        Vertexs ret;
        auto center = roi.get_center();

        for(auto & item : ret){
            item = center;
        }

        #define COMP(s1, s2, i)\
        if((0 s1*x) + (0 s2*y) < (0 s1*ret[i].x) + (0 s2*ret[i].y))\
        ret[i] = Vector2u(x,y);\

        for(auto y = y_range.start; y < y_range.stop; ++y){
            for(auto x = x_range.start; x < x_range.stop; ++x){
                const auto color = __map[{x,y}];
                if(color != match) continue;

                COMP(-1, -1, 0)
                COMP(+1, -1, 1)
                COMP(+1, +1, 2)
                COMP(-1, +1, 3)
            }
        }

        #undef COMP

        return ret;
    };
};

}
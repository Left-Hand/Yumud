#pragma once


#include "../apriltag.hpp"
#include "dec16h5.hpp"


class Apriltag16H5Recognizer{
protected:
    Apriltag16H5Decoder decoder;
public:

    void update(const Image<Grayscale> src, const Rect2i roi) override {
        scexpr uint apriltag_s = 4;

        auto get_vertex_val = [&](const Vertexs & _vertexs, const Vector2 & _grid_pos, const Image<Grayscale> & gs) -> Grayscale{

            auto get_vertex = [&](const Vertexs & __vertexs, const Vector2 & __grid_pos) -> Vector2 {
                Vector2 grid_scale = (__grid_pos + Vector2{1,1}) / (apriltag_s + 2);

                Vector2 upper_x = __vertexs[0].lerp(__vertexs[1], grid_scale.x);
                Vector2 lower_x = __vertexs[3].lerp(__vertexs[2], grid_scale.x);

                return upper_x.lerp(lower_x, grid_scale.y);
            };


            auto get_vertex_grid = [&](const Vertexs & __vertexs, const Vector2 & __grid_pos) -> Vector2{
                return get_vertex(__vertexs, __grid_pos + Vector2{0.5, 0.5});
            };

            return gs.bilinear_interpol(get_vertex_grid(_vertexs, _grid_pos));
        };

        auto find_vertex = [](const Image<Grayscale> & __map, const Grayscale & match, const Rect2i & roi) -> Vertexs{
            auto x_range = roi.get_x_range();
            auto y_range = roi.get_y_range();

            Vertexs ret;
            auto center = roi.get_center();
    
            for(auto & item : ret){
                item = center;
            }

            #define COMP(s1, s2, i)\
            if((0 s1*x) + (0 s2*y) < (0 s1*ret[i].x) + (0 s2*ret[i].y))\
            ret[i] = Vector2i(x,y);\

            for(auto y = y_range.from; y < y_range.to; ++y){
                for(auto x = x_range.from; x < x_range.to; ++x){
                    auto color = __map[{x,y}];
                    if(color != match) continue;

                    COMP(-1, -1, 0)
                    COMP(+1, -1, 1)
                    COMP(+1, +1, 2)
                    COMP(-1, +1, 3)
                }
            }

            return ret;
        };

        auto vertexs = find_vertex(src, Grayscale(255), roi);

        uint16_t code = 0;
        for(uint j = 0; j < apriltag_s; j++){
            for(uint i = 0; i < apriltag_s; i++){
                uint16_t mask = (0x8000) >> (j * 4 + i);
                Grayscale val = get_vertex_val(vertexs, {i,j}, img);
                if((uint8_t)val > 173) code |= mask;
            }
        }


        decoder.update(code);
    }
}
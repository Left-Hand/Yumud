#pragma once

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"

#include "nvcv2/shape/shape.hpp"

#include "types/ray2/Ray2.hpp"

using namespace ymd;

namespace ymd::smc::sim{

// template<typename T>
// struct Element{
//     constexpr bool is_covered(const Vector2_t<real_t> pos) const {
//         return T::is_covered(static_cast<const T &>(*this));
//     }
// };

struct AnnularSector{
    real_t x;
    real_t y;

    real_t inner_radius;
    real_t outer_radius;
    
    real_t start_rad;
    real_t stop_rad;
    constexpr bool is_covered(const Vector2_t<real_t> pos) const {
        return s_is_covered(*this, pos);
    }
private:
    static constexpr bool s_is_covered(const AnnularSector & self, const Vector2_t<real_t> pos) {
        const auto x_err = pos.x - self.x;
        const auto y_err = pos.y - self.y;

        const auto abs_x_err = ABS(x_err);
        const auto abs_y_err = ABS(y_err);

        constexpr real_t INV_SQRT2 = 0.7071067811865475244008443621048490_r;
        //快速检测
        // if((MIN(abs_x_err, abs_y_err) > self.outer_radius)) return false;
        if((abs_x_err > self.outer_radius) and (abs_y_err > self.outer_radius)) return false;

        // if((MAX(abs_x_err, abs_y_err) < self.inner_radius)) return false;
        if((abs_x_err < self.inner_radius * INV_SQRT2) and (abs_y_err < self.inner_radius * INV_SQRT2)) return false;

        const auto err_pos = Vector2_t(x_err, y_err);
        const auto len_squ = square(x_err) + square(y_err);

        // if(not IN_RANGE(len_squ, square(self.inner_radius), square(self.outer_radius))) return false;
        if(len_squ > square(self.outer_radius)) return false;
        if(len_squ < square(self.inner_radius)) return false;

        // const auto wrapped_stop = (self.stop_rad < self.start_rad) ?
        //     self.stop_rad + real_t(TAU) : self.stop_rad;
        const auto wrapped_stop = self.stop_rad;

        if(err_pos.is_clockwise_to(Vector2_t<real_t>::
                from_idenity_rotation(self.start_rad))) return false;
        if(err_pos.is_count_clockwise_to(Vector2_t<real_t>::
                from_idenity_rotation(wrapped_stop))) return false;

        return true;
    }
};

struct RectBlob{
    real_t x;
    real_t y;
    
    real_t width;
    real_t height;

    constexpr bool is_covered(const Vector2_t<real_t> pos) const {
        return s_is_covered(*this, pos);
    }
private:
    static constexpr bool s_is_covered(const RectBlob & self, const Vector2_t<real_t> pos){
        
        return 
            IN_RANGE(pos.x, self.x - self.width * 0.5_r, self.x + self.width * 0.5_r)
            and IN_RANGE(pos.y, self.y - self.height * 0.5_r, self.y + self.height * 0.5_r);
    }
};


namespace static_test{
static constexpr auto blob = RectBlob{
    .x = 0,
    .y = 0,
    .width = 1,
    .height = 1
};

static constexpr auto arc = AnnularSector{
    .x = 0,
    .y = 0,

    .inner_radius = 1,
    .outer_radius = 2,
    
    .start_rad = 0,
    .stop_rad = 1.5_r
};

static_assert(blob.is_covered({1,1}) == false);
static_assert(blob.is_covered({0,0}) == true);



// static_assert(arc.is_covered({1,1}) == true);
static_assert(arc.is_covered({0,0}) == false);
static_assert(arc.is_covered({-0.02_r,1.1_r}) == false);
// static_assert(arc.is_covered({1.3_r,0.1_r}) == true);  // 0°方向
// static_assert(arc.is_covered({-1.9_r,0}) == false); // 180°方向（超出角度范围）

// static_assert(Vector2_t<real_t>(1,1).is_clockwise_to(Vector2_t<real_t>(1,0)));

}

static constexpr real_t PIXELS_PER_METER = 100;
static constexpr real_t METERS_PER_PIXEL = 1 / PIXELS_PER_METER;
static constexpr Vector2u CAMERA_SIZE = {188, 120};
static constexpr Vector2u HALF_CAMERA_SIZE = CAMERA_SIZE / 2;

static constexpr Vector2 transform_camera_to_scene(
    const Vector2u pixel, const Ray2_t<real_t> viewpoint) {
    const Vector2i pixel_offset = {
        int(pixel.x) - int(HALF_CAMERA_SIZE.x), 
        int(HALF_CAMERA_SIZE.y) - int(pixel.y)};

    const Vector2 camera_offset = Vector2(pixel_offset) * METERS_PER_PIXEL;
    const auto rot = viewpoint.rad - real_t(PI/2);
    return viewpoint.org + camera_offset.rotated(rot);
}

namespace static_test{
    constexpr auto viewpoint = Ray2_t<real_t>{Vector2(0, 0), real_t(PI/2)};
    constexpr auto pos1 = transform_camera_to_scene(HALF_CAMERA_SIZE, viewpoint);
    // constexpr auto pos2 = transform_camera_to_scene(HALF_CAMERA_SIZE, viewpoint);
    static_assert(float(pos1.x) == 0);
    static_assert(float(pos1.y) == 0);
}

namespace details{
    PRO_DEF_MEM_DISPATCH(MemIsCovered, is_covered);

    struct ElementFacade : pro::facade_builder
        ::support_copy<pro::constraint_level::none>
        ::add_convention<MemIsCovered, bool(const Vector2_t<real_t>) const>
        ::build {};
}

using ElementProxy = pro::proxy<details::ElementFacade>;

using Elements = std::vector<ElementProxy>;

class Scene final{
public:
    Scene() = default;
    Scene(const Scene &) = delete;
    Scene(Scene &&) = default;

    template<typename T>
    void add_element(T && element){
        elements_.emplace_back(
            pro::make_proxy<details::ElementFacade>(
                std::forward<T>(element))
        );
    }

    Image<Grayscale> render(const Ray2_t<real_t> viewpoint) const {
        Image<Grayscale> ret{CAMERA_SIZE};

        for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
            for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
                const auto pos = transform_camera_to_scene({x,y}, viewpoint);

                // DEBUG_PRINTLN(x,y,pos);
                const bool covered = this->is_covered(pos);
                ret.set_pixel({x,y}, covered ? Grayscale{255} : Grayscale{0});
            }
        }

        return ret;
    }
private:
    Elements elements_;

    const bool is_covered(const Vector2_t<real_t> pos) const{
        for(const auto & element : elements_){
            if(element->is_covered(pos)){
                return true;
            }
        }
        return false;
    }
};



}
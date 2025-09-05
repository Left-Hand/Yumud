#pragma once

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter/painter.hpp"
#include "types/shapes/box_rect.hpp"
#include "types/shapes/annular_sector.hpp"
#include "types/shapes/rotated_rect.hpp"

#include "nvcv2/shape/shape.hpp"

#include "types/regions/ray2.hpp"
#include "types/regions/line2.hpp"

#include "types/gesture/pose2.hpp"

// static constexpr Vec2u CAMERA_SIZE = {94/2, 60/2};
// static constexpr Vec2u CAMERA_SIZE = {94, 60};
static constexpr Vec2u CAMERA_SIZE = {94 * 3 / 2, 60 * 3 / 2};
// static constexpr Vec2u CAMERA_SIZE = {188, 120};
// static constexpr Vec2u CAMERA_SIZE = {120, 80};
// static constexpr Vec2u CAMERA_SIZE = {120, 80};
static constexpr Vec2u HALF_CAMERA_SIZE = CAMERA_SIZE / 2;
static constexpr uint8_t WHITE_COLOR = 0x9f;


using namespace ymd;

template<typename T>
using cache_of_t = CacheOf<T, bool>;




using BoundingBox = Rect2<q16>;

namespace ymd{
struct RotatedZebraRect{
    q16 width;
    q16 height;
    Angle<q16> orientation;


    template<size_t I>
    requires ((0 <= I) and (I < 4))
    constexpr Vec2<q16> get_corner() const {
        switch(I){
            case 0: return {-width / 2, height / 2};
            case 1: return {width / 2, height / 2};
            case 2: return {-width / 2, -height / 2};
            case 3: return {width / 2, -height / 2};
            default: __builtin_unreachable();
        }
    }
};

template<>
struct alignas(4) CacheOf<RotatedZebraRect, bool>{
    using Self = CacheOf<RotatedZebraRect, bool>;

    q16 half_width;
    q16 half_height;
    q16 s;
    q16 c;

    static constexpr Self from(const RotatedZebraRect & obj){
        const auto [s,c] = obj.orientation.sincos();
        return Self{
            .half_width = obj.width / 2,
            .half_height = obj.height / 2,
            .s = s,
            .c = c
        };
    }

    __fast_inline constexpr uint8_t color_from_point(const Vec2<q16> offset) const {
        return s_color_from_point(*this, offset);
    }
private:
    __fast_inline static constexpr uint8_t s_color_from_point(
        const Self & self, const Vec2<q16> offset){
        // -s * p.x + c * p.y;
        // -c * p.x - s * p.y;
        const auto x_offset = - self.c * offset.x - self.s * offset.y;
        
        return 
            ((abs(-self.s * offset.x + self.c * offset.y)
                <= self.half_height) and
            (abs(x_offset) 
                <= self.half_width))
                
            // ? (static_cast<uint8_t>(CLAMP(sinpu(x_offset * 15) * 3 + 1, 0, 2) * WHITE_COLOR / 2)) : 0
            ? (WHITE_COLOR) : 0
        ;
    }
};

template<>
struct BoundingBoxOf<RotatedZebraRect>{


    static constexpr BoundingBox bounding_box(const RotatedZebraRect & obj){
        const auto rot = Vec2<q16>::from_angle(obj.orientation);
        const std::array<Vec2<q16>, 4> points = {
            obj.get_corner<0>().improduct(rot),
            obj.get_corner<1>().improduct(rot),
            obj.get_corner<2>().improduct(rot),
            obj.get_corner<3>().improduct(rot)
        };

        return BoundingBox::from_minimal_bounding_box(std::span(points));
    }

};




struct SpotLight final{
    q16 radius = 1;

};

template<>
struct CacheOf<SpotLight, bool>{
    using Self = CacheOf<SpotLight, bool>;

    q16 squ_radius;

    static constexpr auto from(const SpotLight & obj) {
        return CacheOf{obj.radius * obj.radius};
    }

    __fast_inline constexpr uint8_t color_from_point(const Vec2<q16> offset) const {
        return s_color_from_point(*this, offset);
    }
private:
    __fast_inline static constexpr uint8_t s_color_from_point(
        const Self & self, 
        const Vec2<q16> offset
    ){
        const auto len_squ = offset.length_squared();
        // const auto temp = MAX(9 * len_squ, 1);
        // return uint8_t(130 / temp);
        const auto temp = CLAMP(2 - 3 * len_squ,0, 1);
        return uint8_t(130 * temp);
    }
};

template<>
struct BoundingBoxOf<SpotLight>{
    static constexpr auto bounding_box(const SpotLight & obj){
        return Rect2u16{
            Vec2u16{static_cast<uint16_t>(-obj.radius), 
                static_cast<uint16_t>(-obj.radius)}, 
            Vec2u16{
                static_cast<uint16_t>(2 * obj.radius), 
                static_cast<uint16_t>(2 * obj.radius)}
        };
    }
};


}

namespace ymd::smc::sim{

struct Placement{
    Vec2<q16> position;
};


template<typename T>
struct ElementWithPlacement{
    using Element = T;
    using Cache = cache_of_t<T>;

    Placement placement;
    BoundingBox bounding_box;
    Cache cache;
};

template<typename T>
constexpr ElementWithPlacement<T> operator | (const T & element, const Placement& placement){
    return ElementWithPlacement<T>{
        .placement = placement,
        .bounding_box = BoundingBoxOf<T>::bounding_box(element),
        .cache = CacheOf<T, bool>::from(element),
    };
}




//将相机像素转换为地面坐标
static constexpr Vec2<q16> project_pixel_to_ground(
    const Vec2u pixel, 
    const Pose2<q16> pose, 
    const q16 zoom
) {
    const Vec2i pixel_offset = {
        int(pixel.x) - int(HALF_CAMERA_SIZE.x), 
        int(HALF_CAMERA_SIZE.y) - int(pixel.y)};

    const Vec2<q16> camera_offset = Vec2<q16>(pixel_offset) * zoom;
    const auto rot = pose.orientation - 90_deg;
    return pose.position + camera_offset.rotated(rot);
}


static constexpr Vec2u project_ground_to_pixel(
    const Vec2<q16>& ground_pos,
    const Pose2<q16> pose,
    const q16 zoom)
{
    // 1. Remove pose position offset
    const Vec2<q16> relative_pos = ground_pos - pose.position;
    
    // 2. Calculate inverse orientation (original orientation was pose.orientation - PI/2)
    const auto [s, c] = (-(pose.orientation - 90_deg)).sincos();
    
    // 3. Apply inverse orientation matrix (transpose of original orientation matrix)
    const Vec2<q16> unrotated = {
        c * relative_pos.x - s * relative_pos.y,
        s * relative_pos.x + c * relative_pos.y
    };
    
    // 4. Remove scaling and convert to pixel space
    const Vec2<q16> pixel_offset = unrotated / zoom;
    
    // 5. Convert to camera coordinates and clamp to pixel grid
    return Vec2u{
        static_cast<uint>(round(pixel_offset.x + HALF_CAMERA_SIZE.x)),
        static_cast<uint>(round(HALF_CAMERA_SIZE.y - pixel_offset.y))
    };
}

namespace details{
    PRO_DEF_MEM_DISPATCH(MemIsCovered, color_from_point);

    struct ElementFacade : pro::facade_builder
        ::support_copy<pro::constraint_level::none>
        ::add_convention<MemIsCovered, bool(const Vec2<q16>) const>
        ::build {};
}




class SceneIntf{
public:
    SceneIntf() = default;
    SceneIntf(const SceneIntf &) = delete;
    SceneIntf(SceneIntf &&) = default;
    virtual ~SceneIntf() = default;
    virtual Image<Gray> render(const Pose2<q16> pose, const q16 zoom) const = 0;
};

// class DynamicScene final:public SceneIntf{
// public:
//     DynamicScene(){;}

//     template<typename T>
//     void add_element(T && element){
//         elements_.emplace_back(
//             pro::make_proxy<details::ElementFacade>(
//                 std::forward<T>(element))
//         );
//     }

//     Image<Gray> render(const Pose2<q16> pose) const {
//         Image<Gray> ret{CAMERA_SIZE};

//         const auto org = project_pixel_to_ground({0,0}, pose);
//         const auto y_step = project_pixel_to_ground({0,1}, pose) - org;
//         const auto x_step = project_pixel_to_ground({1,0}, pose) - org;
        
//         auto offset = org;
//         for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
//             const auto beg = offset;
//             for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
//                 const bool covered = this->color_from_point(offset);
//                 ret.set_pixel({x,y}, covered ? Gray{255} : Gray{0});
//                 offset += x_step;
//             }
//             offset = beg;
//             offset += y_step;
//         }

//         return ret;
//     }
// private:
//     std::vector<pro::proxy<details::ElementFacade>> elements_;

//     bool color_from_point(const Vec2<q16> offset) const{
//         for(const auto & element : elements_){
//             if(element->color_from_point(offset)){
//                 return true;
//             }
//         }
//         return false;
//     }
// };


template<typename ...Objects>
class StaticScene final:public SceneIntf{ 
public:
    using Container = std::tuple<Objects...>;

    ~StaticScene() = default;
    constexpr StaticScene(Objects&&... objects):
        objects_(std::make_tuple(std::forward<Objects>(objects)...)){}


    Image<Gray> render(const Pose2<q16> pose, const q16 zoom) const {
        // static constexpr auto EXTENDED_BOUND_LENGTH = 1.3_r;
        const auto pbuf = std::make_shared<uint8_t[]>(CAMERA_SIZE.x * CAMERA_SIZE.y);
        const auto org =    project_pixel_to_ground({0,0}, pose, zoom);
        const auto y_step = project_pixel_to_ground({0,1}, pose, zoom) - org;
        const auto x_step = project_pixel_to_ground({1,0}, pose, zoom) - org;

        const auto ground_region = Rect2<q16>::from_minimal_bounding_box({
            org, 
            project_pixel_to_ground({CAMERA_SIZE.x,0},    pose, zoom),
            project_pixel_to_ground({0,CAMERA_SIZE.y},    pose, zoom),
            project_pixel_to_ground(CAMERA_SIZE,          pose, zoom)
        });

            
        bool dirty = false;
        auto apply_render = [&](const bool rough_judge_passed, auto && fn){
            if(!rough_judge_passed) return;
            dirty = true;
            using Fn = std::decay_t<decltype(fn)>;
            auto local_pos = org;
            for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
                const auto beg = local_pos;
                auto * ptr = &pbuf[y * CAMERA_SIZE.x];
                for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
                    const uint8_t color = std::forward<Fn>(fn)(local_pos);
                    *ptr = MIN(*ptr + color, 255);
                    local_pos += x_step;
                    ptr = std::next(ptr);
                }
                local_pos = beg;
                local_pos += y_step;
            }
        };

        std::apply([&](const auto&... object){
            (apply_render(
                ground_region.intersects(object.bounding_box.shift(object.placement.position)),
                // true,
                [&](const Vec2<q16> local_pos) { 
                    return object.cache.color_from_point(local_pos - object.placement.position); 
                }), ...);
        }, objects_);

        if(!dirty) std::fill_n(pbuf.get(), CAMERA_SIZE.x * CAMERA_SIZE.y, 0);
        return Image<Gray>(std::move(
            std::reinterpret_pointer_cast<Gray[]>(pbuf)), CAMERA_SIZE);
    }
private:
    std::tuple<Objects...> objects_;  // Expanded parameter pack
};


template<typename... Objects>
static constexpr auto make_static_scene(Objects && ... objects){
    return StaticScene<Objects...>(std::forward<Objects>(objects)...);
}

}
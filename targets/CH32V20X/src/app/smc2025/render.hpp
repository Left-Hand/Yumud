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

// static constexpr Vector2u CAMERA_SIZE = {94/2, 60/2};
// static constexpr Vector2u CAMERA_SIZE = {94, 60};
static constexpr Vector2u CAMERA_SIZE = {94 * 3 / 2, 60 * 3 / 2};
// static constexpr Vector2u CAMERA_SIZE = {188, 120};
// static constexpr Vector2u CAMERA_SIZE = {120, 80};
// static constexpr Vector2u CAMERA_SIZE = {120, 80};
static constexpr Vector2u HALF_CAMERA_SIZE = CAMERA_SIZE / 2;
static constexpr uint8_t WHITE_COLOR = 0x9f;


using namespace ymd;


template<typename T>
struct cache_of{
    using type = typename T::Cache;
};

template<typename T>
using cache_of_t = cache_of<T>::type;


namespace ymd::smc::sim{

using BoundingBox = Rect2<q16>;

struct Placement{
    Vector2<q16> position;
};


template<typename T>
struct ElementWithPlacement{
    using Element = T;
    using Cache = cache_of_t<T>;

    Placement placement;
    BoundingBox bounding_box;
    Cache cache;
    // T element;
};

template<typename T>
constexpr ElementWithPlacement<T> operator | (const T & element, const Placement& placement){
    return ElementWithPlacement<T>{
        .placement = placement,
        .bounding_box = element.to_bounding_box(),
        .cache = element.to_cache(),
        // .element = element,
    };
}




struct SpotLight final{
    q16 radius = 1;

    struct Cache{
        q16 squ_radius;

        __fast_inline constexpr uint8_t color_from_point(const Vector2<q16> offset) const {
            return s_color_from_point(*this, offset);
        }
    private:
        __fast_inline static constexpr uint8_t s_color_from_point(const Cache & self, const Vector2<q16> offset){
            const auto len_squ = offset.length_squared();
            // const auto temp = MAX(9 * len_squ, 1);
            // return uint8_t(130 / temp);
            const auto temp = CLAMP(2 - 3 * len_squ,0, 1);
            return uint8_t(130 * temp);
        }
    };

    constexpr auto to_cache() const {
        return Cache{
            .squ_radius = square(radius)
        };
    }

    constexpr auto to_bounding_box() const {
        return BoundingBox{-radius, -radius, 2 * radius, 2 * radius};
    }
};



struct RotatedZebraRect{
    q16 width;
    q16 height;
    q16 rotation;

    struct alignas(4) Cache{
        q16 half_width;
        q16 half_height;
        q16 s;
        q16 c;

        __fast_inline constexpr uint8_t color_from_point(const Vector2<q16> offset) const {
            return s_color_from_point(*this, offset);
        }
    private:
        __fast_inline static constexpr uint8_t s_color_from_point(
            const Cache & self, const Vector2<q16> offset){
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

    constexpr auto to_cache() const {
        const auto [s,c] = sincos(rotation);
        return Cache{
            .half_width = width / 2,
            .half_height = height / 2,
            .s = s,
            .c = c
        };
    }

    constexpr BoundingBox to_bounding_box() const {
        const auto rot = Vector2<q16>::from_idenity_rotation(rotation);
        const std::array<Vector2<q16>, 4> points = {
            get_raw_point<0>().improduct(rot),
            get_raw_point<1>().improduct(rot),
            get_raw_point<2>().improduct(rot),
            get_raw_point<3>().improduct(rot)
        };

        return BoundingBox::from_minimal_bounding_box(std::span(points));
    }

    template<size_t I>
    requires ((0 <= I) and (I < 4))
    constexpr Vector2<q16> get_raw_point() const {
        switch(I){
            case 0: return {-width / 2, height / 2};
            case 1: return {width / 2, height / 2};
            case 2: return {-width / 2, -height / 2};
            case 3: return {width / 2, -height / 2};
            default: __builtin_unreachable();
        }
    }
};


//将相机像素转换为地面坐标
static constexpr Vector2<q16> project_pixel_to_ground(
    const Vector2u pixel, 
    const Pose2<q16> pose, 
    const q16 zoom
) {
    const Vector2i pixel_offset = {
        int(pixel.x) - int(HALF_CAMERA_SIZE.x), 
        int(HALF_CAMERA_SIZE.y) - int(pixel.y)};

    const Vector2<q16> camera_offset = Vector2<q16>(pixel_offset) * zoom;
    const auto rot = pose.orientation - q16(PI/2);
    return pose.position + camera_offset.rotated(rot);
}


static constexpr Vector2u project_ground_to_pixel(
    const Vector2<q16>& ground_pos,
    const Pose2<q16> pose,
    const q16 zoom)
{
    // 1. Remove pose position offset
    const Vector2<q16> relative_pos = ground_pos - pose.position;
    
    // 2. Calculate inverse rotation (original rotation was pose.orientation - PI/2)
    const auto [s, c] = sincos(-(pose.orientation - q16(PI/2)));
    
    // 3. Apply inverse rotation matrix (transpose of original rotation matrix)
    const Vector2<q16> unrotated = {
        c * relative_pos.x - s * relative_pos.y,
        s * relative_pos.x + c * relative_pos.y
    };
    
    // 4. Remove scaling and convert to pixel space
    const Vector2<q16> pixel_offset = unrotated / zoom;
    
    // 5. Convert to camera coordinates and clamp to pixel grid
    return Vector2u{
        static_cast<uint>(round(pixel_offset.x + HALF_CAMERA_SIZE.x)),
        static_cast<uint>(round(HALF_CAMERA_SIZE.y - pixel_offset.y))
    };
}

namespace details{
    PRO_DEF_MEM_DISPATCH(MemIsCovered, color_from_point);

    struct ElementFacade : pro::facade_builder
        ::support_copy<pro::constraint_level::none>
        ::add_convention<MemIsCovered, bool(const Vector2<q16>) const>
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

//     bool color_from_point(const Vector2<q16> offset) const{
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
                [&](const Vector2<q16> local_pos) { 
                    return object.cache.color_from_point(local_pos - object.placement.position); 
                }), ...);
        }, objects_);

        if(!dirty) std::memset(pbuf.get(), 0, CAMERA_SIZE.x * CAMERA_SIZE.y);
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
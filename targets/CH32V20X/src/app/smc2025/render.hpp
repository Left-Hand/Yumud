#pragma once

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"

#include "nvcv2/shape/shape.hpp"

#include "types/regions/ray2/Ray2.hpp"
#include "types/regions/line2/Line2.hpp"

// static constexpr Vector2u CAMERA_SIZE = {94/2, 60/2};
// static constexpr Vector2u CAMERA_SIZE = {94, 60};
static constexpr Vector2u CAMERA_SIZE = {94 * 3 / 2, 60 * 3 / 2};
// static constexpr Vector2u CAMERA_SIZE = {188, 120};
// static constexpr Vector2u CAMERA_SIZE = {120, 80};
// static constexpr Vector2u CAMERA_SIZE = {120, 80};
static constexpr Vector2u HALF_CAMERA_SIZE = CAMERA_SIZE / 2;
static constexpr uint8_t WHITE_COLOR = 0x9f;


using namespace ymd;

consteval q16 operator"" _deg(long double x){
    return q16(x * PI / 180);
}
consteval q16 operator"" _deg(uint64_t x){
    return q16(x * PI / 180);
}



namespace ymd::smc::sim{

template<typename T>
struct Pose2_t{
    Vector2<T> pos;
    T rad;

    template<typename U = T>
    constexpr Pose2_t(const Pose2_t<U> & other):
        pos(other.pos), rad(other.rad){;}

    template<typename U = T>
    constexpr Pose2_t(const Vector2<U> & _pos, const U _rad):
        pos(_pos), rad(_rad){;}

    [[nodiscard]] constexpr Pose2_t<T> forward_move(const T length) const {
        const auto [s, c] = sincos(rad);
        const auto delta = Vector2<T>{c, s} * length;
        return {
            pos + delta,
            rad
        };
    }

    [[nodiscard]] constexpr Pose2_t<T> side_move(const T length) const {
        const auto [s, c] = sincos(rad + q16(PI/2));
        const auto delta = Vector2<T>{c, s} * length;
        return {
            pos + delta,
            rad
        };
    }

    [[nodiscard]] constexpr Pose2_t<T> revolve_by_radius_and_rotation(
            const T radius, const T rot) const {

        const auto ar = Vector2<T>::from_idenity_rotation(rad).rotated(
            rot > 0 ? T(PI/2) : T(-PI/2)
        ) * radius;

        const auto org = pos + ar;
        const auto delta = (-ar).rotated(rot);
        return {org + delta, rad + rot};
    }
};

template<typename T>
Pose2_t(const Vector2<T> & _pos, const T _rad) -> Pose2_t<T>;

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Pose2_t<T> & obj){
    return os << os.brackets<'('>() 
        << obj.pos << os.splitter() << 
        obj.rad << os.brackets<')'>();
}

using BoundingBox = Rect2<q16>;

struct Placement{
    Vector2<q16> pos;

    // q16 rotation = 0;
    // q16 zoom = 1;

    // Vector2q<16> apply_transform(Vector2q<16> p) const {
    // return (p * zoom).rotated(rotation) + pos;
    // }
};

template<typename T>
struct cache_of{
    using type = typename T::Cache;
};

template<typename T>
using cache_of_t = cache_of<T>::type;

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


struct AnnularSector{

    q16 inner_radius;
    q16 outer_radius;
    
    q16 start_rad;
    q16 stop_rad;

    constexpr bool does_contains_rad(const q16 rad) const {
        return IN_RANGE(rad, start_rad, stop_rad);
    }

    constexpr Rect2<q16> get_bounding_box() const {
        const bool x_reached_left = does_contains_rad(q16(PI));
        const bool x_reached_right = does_contains_rad(q16(0));
        const bool y_reached_top = does_contains_rad(q16(PI/2));
        const bool y_reached_bottom = does_contains_rad(q16(-PI/2));

        const auto p1 = Vector2<q16>::from_idenity_rotation(start_rad) * outer_radius;
        const auto p2 = Vector2<q16>::from_idenity_rotation(stop_rad) * outer_radius;

        const auto x_min = x_reached_left ? (-outer_radius) : MIN(p1.x, p2.x);
        const auto x_max = x_reached_right ? (outer_radius) : MAX(p1.x, p2.x);
        const auto y_min = y_reached_top ? (-outer_radius) : MIN(p1.y, p2.y);
        const auto y_max = y_reached_bottom ? (outer_radius) : MAX(p1.y, p2.y);

        return Rect2<q16>(x_min, x_max, y_min, y_max);
    }

    struct alignas(4) Cache{
        q16 squ_inner_radius;
        q16 squ_outer_radius;
        Vector2<q16> start_norm_vec;
        Vector2<q16> stop_norm_vec;
        bool is_close;


        __fast_inline constexpr uint8_t color_from_point(const Vector2<q16> offset) const {
            return s_color_from_point(*this, offset);
        }
    private:
        static constexpr uint8_t s_color_from_point(const Cache & self, const Vector2<q16> offset){
            const auto len_squ = offset.length_squared();

            if (len_squ < self.squ_inner_radius || 
                len_squ > self.squ_outer_radius) {
                return false;
            }

            return (self.is_close 
                ? (offset.is_count_clockwise_to(self.start_norm_vec) && 
                offset.is_clockwise_to(self.stop_norm_vec))
                : (offset.is_count_clockwise_to(self.start_norm_vec) || 
                offset.is_clockwise_to(self.stop_norm_vec))) ? WHITE_COLOR : 0;
        } 
    };

    constexpr auto to_cache() const {
        const auto v1 = Vector2<q16>::from_idenity_rotation(start_rad);
        const auto v2 = Vector2<q16>::from_idenity_rotation(stop_rad);
        return Cache{
            .squ_inner_radius = square(inner_radius),
            .squ_outer_radius = square(outer_radius),
            .start_norm_vec = v1,
            .stop_norm_vec = v2,
            .is_close = v2.is_count_clockwise_to(v1)
        };
    }

    constexpr auto to_bounding_box() const {
        const auto v1 = Vector2<q16>::from_idenity_rotation(start_rad);
        const auto v2 = Vector2<q16>::from_idenity_rotation(stop_rad);
        const bool is_close = v2.is_count_clockwise_to(v1);
        Rect2<real_t> bb = Rect2<real_t>::from_minimal_bounding_box({
            v1 * inner_radius,
            v1 * outer_radius,
            v2 * inner_radius,
            v2 * outer_radius
        });

        MergeHelper::merge_if_has_radian(bb, {1,0}, v1, v2, is_close);
        MergeHelper::merge_if_has_radian(bb, {0,1}, v1, v2, is_close);
        MergeHelper::merge_if_has_radian(bb, {-1,0}, v1, v2, is_close);
        MergeHelper::merge_if_has_radian(bb, {0,-1}, v1, v2, is_close);

        return bb;
    }

    __fast_inline constexpr bool has_radian(
        const real_t radian)
    const {
        return has_radian(Vector2<q16>::from_idenity_rotation(radian));
    }

    __fast_inline constexpr bool has_radian(
        const Vector2<real_t> offset)
    const {
        const auto v1 = Vector2<q16>::from_idenity_rotation(start_rad);
        const auto v2 = Vector2<q16>::from_idenity_rotation(stop_rad);
        return MergeHelper::has_radian(
            offset,
            v1, v2, v2.is_count_clockwise_to(v1)
        );
    }
private:

    struct MergeHelper{
        static constexpr bool has_radian(
            const real_t radian,
            const Vector2<real_t> start_norm_vec,
            const Vector2<real_t> stop_norm_vec,
            const bool is_close
        ){
            return has_radian(
                Vector2<q16>::from_idenity_rotation(radian), 
                start_norm_vec, stop_norm_vec, 
                is_close
            );
        }

        static constexpr bool has_radian(
            const Vector2<real_t> offset,
            const Vector2<real_t> start_norm_vec,
            const Vector2<real_t> stop_norm_vec,
            const bool is_close
        ){
            const auto b1 = offset.is_count_clockwise_to(start_norm_vec);
            const auto b2 = offset.is_clockwise_to(stop_norm_vec);
            if(is_close) return b1 and b2;
            else return b1 or b2;
        }

        static constexpr void merge_if_has_radian(
            BoundingBox & box,
            const Vector2<real_t> offset,
            const Vector2<real_t> start_norm_vec,
            const Vector2<real_t> stop_norm_vec,
            const bool is_close
        ){
            if(has_radian(offset, start_norm_vec, stop_norm_vec, is_close)){
                box = box.merge(offset);
            }
        }
    };

};

struct RectBlob{
    q16 width;
    q16 height;

    struct Cache{
        q16 half_width;
        q16 half_height;

        __fast_inline constexpr uint8_t color_from_point(const Vector2<q16> offset) const {
            return s_color_from_point(*this, offset);
        }
    private:
        __fast_inline static constexpr uint8_t s_color_from_point(const Cache & self, const Vector2<q16> offset){
            return 
                ((abs(offset.x) - (self.half_width) <= 0)
                and (abs(offset.y) - (self.half_height) <= 0)) ? WHITE_COLOR : 0;
            ;
        }
    };

    constexpr auto to_cache() const {
        return Cache{
            .half_width = width / 2,
            .half_height = height / 2,
        };
    }

    constexpr auto to_bounding_box() const {
        return BoundingBox{-width/2,-height/2, width, height};
    }
};


struct Aurora{
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

struct RotatedRect{
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
            return 
                ((abs(-self.s * offset.x + self.c * offset.y)
                    <= self.half_height) and
                (abs(-self.c * offset.x - self.s * offset.y) 
                    <= self.half_width))
                    
                ? WHITE_COLOR : 0
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
static constexpr Vector2q<16> project_pixel_to_ground(
    const Vector2u pixel, 
    const Pose2_t<q16> viewpoint, 
    const q16 zoom
) {
    const Vector2i pixel_offset = {
        int(pixel.x) - int(HALF_CAMERA_SIZE.x), 
        int(HALF_CAMERA_SIZE.y) - int(pixel.y)};

    const Vector2q<16> camera_offset = Vector2q<16>(pixel_offset) * zoom;
    const auto rot = viewpoint.rad - q16(PI/2);
    return viewpoint.pos + camera_offset.rotated(rot);
}


static constexpr Vector2u project_ground_to_pixel(
    const Vector2q<16>& ground_pos,
    const Pose2_t<q16> viewpoint,
    const q16 zoom)
{
    // 1. Remove viewpoint position offset
    const Vector2q<16> relative_pos = ground_pos - viewpoint.pos;
    
    // 2. Calculate inverse rotation (original rotation was viewpoint.rad - PI/2)
    const auto [s, c] = sincos(-(viewpoint.rad - q16(PI/2)));
    
    // 3. Apply inverse rotation matrix (transpose of original rotation matrix)
    const Vector2q<16> unrotated = {
        c * relative_pos.x - s * relative_pos.y,
        s * relative_pos.x + c * relative_pos.y
    };
    
    // 4. Remove scaling and convert to pixel space
    const Vector2q<16> pixel_offset = unrotated / zoom;
    
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
    virtual Image<Grayscale> render(const Pose2_t<q16> viewpoint, const q16 zoom) const = 0;
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

//     Image<Grayscale> render(const Pose2_t<q16> viewpoint) const {
//         Image<Grayscale> ret{CAMERA_SIZE};

//         const auto org = project_pixel_to_ground({0,0}, viewpoint);
//         const auto y_step = project_pixel_to_ground({0,1}, viewpoint) - org;
//         const auto x_step = project_pixel_to_ground({1,0}, viewpoint) - org;
        
//         auto offset = org;
//         for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
//             const auto beg = offset;
//             for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
//                 const bool covered = this->color_from_point(offset);
//                 ret.set_pixel({x,y}, covered ? Grayscale{255} : Grayscale{0});
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


    Image<Grayscale> render(const Pose2_t<q16> viewpoint, const q16 zoom) const {
        // static constexpr auto EXTENDED_BOUND_LENGTH = 1.3_r;
        const auto pbuf = std::make_shared<uint8_t[]>(CAMERA_SIZE.x * CAMERA_SIZE.y);
        const auto org =    project_pixel_to_ground({0,0}, viewpoint, zoom);
        const auto y_step = project_pixel_to_ground({0,1}, viewpoint, zoom) - org;
        const auto x_step = project_pixel_to_ground({1,0}, viewpoint, zoom) - org;

        const auto ground_region = Rect2<q16>::from_minimal_bounding_box({
            org, 
            project_pixel_to_ground({CAMERA_SIZE.x,0},    viewpoint, zoom),
            project_pixel_to_ground({0,CAMERA_SIZE.y},    viewpoint, zoom),
            project_pixel_to_ground(CAMERA_SIZE,          viewpoint, zoom)
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
                ground_region.intersects(object.bounding_box.shift(object.placement.pos)),
                // true,
                [&](const Vector2<q16> local_pos) { 
                    return object.cache.color_from_point(local_pos - object.placement.pos); 
                }), ...);
        }, objects_);

        if(!dirty) std::memset(pbuf.get(), 0, CAMERA_SIZE.x * CAMERA_SIZE.y);
        return Image<Grayscale>(std::move(
            std::reinterpret_pointer_cast<Grayscale[]>(pbuf)), CAMERA_SIZE);
    }
private:
    std::tuple<Objects...> objects_;  // Expanded parameter pack
};


template<typename... Objects>
static constexpr auto make_static_scene(Objects && ... objects){
    return StaticScene<Objects...>(std::forward<Objects>(objects)...);
}

}
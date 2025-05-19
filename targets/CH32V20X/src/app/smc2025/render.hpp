#pragma once

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"

#include "nvcv2/shape/shape.hpp"

#include "types/ray2/Ray2.hpp"
#include "types/line2/Line2.hpp"

using namespace ymd;

consteval real_t operator"" _deg(long double x){
    return real_t(x * PI / 180);
}
consteval real_t operator"" _deg(uint64_t x){
    return real_t(x * PI / 180);
}



namespace ymd::smc::sim{

template<typename T>
struct Gest2_t{
    Vector2_t<T> pos;
    T rad;

    [[nodiscard]] constexpr Gest2_t<T> forward_move(const T length) const {
        const auto [s, c] = sincos(rad);
        const auto delta = Vector2_t<T>{c, s} * length;
        return {
            .pos = pos + delta,
            .rad = rad
        };
    }

    [[nodiscard]] constexpr Gest2_t<T> side_move(const T length) const {
        const auto [s, c] = sincos(rad + real_t(PI/2));
        const auto delta = Vector2_t<T>{c, s} * length;
        return {
            .pos = pos + delta,
            .rad = rad
        };
    }

    [[nodiscard]] constexpr Gest2_t<T> revolve_by_radius_and_rotation(
            const T radius, const T rot) const {

        const auto ar = Vector2_t<T>::from_rotation(rad, radius).rotated(
            rot > 0 ? T(PI/2) : T(-PI/2)
        );

        const auto org = pos + ar;
        const auto delta = (-ar).rotated(rot);
        return {org + delta, rad + rot};
        // return {org, rad + rot};
    }
};

// struct ElementWithPlacement;

using BoundingBox = Rect2_t<real_t>;

struct Placement{
    Vector2_t<real_t> pos;

    // real_t rotation = 0;
    // real_t scale = 1;

    // Vector2 apply_transform(Vector2 p) const {
    // return (p * scale).rotated(rotation) + pos;
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

// template<typename T>
// struct Element{
//     constexpr bool is_covered(const Vector2_t<real_t> offset) const {
//         return T::is_covered(static_cast<const T &>(*this));
//     }
// };

struct AnnularSector{

    real_t inner_radius;
    real_t outer_radius;
    
    real_t start_rad;
    real_t stop_rad;

    constexpr bool does_contains_rad(const real_t rad) const {
        return IN_RANGE(rad, start_rad, stop_rad);
    }

    constexpr Rect2_t<real_t> get_bounding_box() const {
        const bool x_reached_left = does_contains_rad(real_t(PI));
        const bool x_reached_right = does_contains_rad(real_t(0));
        const bool y_reached_top = does_contains_rad(real_t(PI/2));
        const bool y_reached_bottom = does_contains_rad(real_t(-PI/2));

        const auto p1 = Vector2_t<real_t>::from_rotation(start_rad, outer_radius);
        const auto p2 = Vector2_t<real_t>::from_rotation(stop_rad, outer_radius);

        const auto x_min = x_reached_left ? (-outer_radius) : MIN(p1.x, p2.x);
        const auto x_max = x_reached_right ? outer_radius : MAX(p1.x, p2.x);
        const auto y_min = y_reached_top ? (-outer_radius) : MIN(p1.y, p2.y);
        const auto y_max = y_reached_bottom ? outer_radius : MAX(p1.y, p2.y);

        return Rect2_t<real_t>(x_min, x_max, y_min, y_max);
    }

    struct alignas(4) Cache{
        real_t squ_inner_radius;
        real_t squ_outer_radius;
        Vector2_t<real_t> start_norm_vec;
        Vector2_t<real_t> stop_norm_vec;
        bool is_close;


        __fast_inline constexpr bool is_covered(const Vector2_t<real_t> offset) const {
            return s_is_covered(*this, offset);
        }
    private:
        static constexpr bool s_is_covered(const Cache & self, const Vector2_t<real_t> offset){
            const auto len_squ = offset.length_squared();

            if((len_squ - self.squ_outer_radius) * (len_squ - self.squ_inner_radius) > 0)
                return false;

            const auto b1 = offset.is_count_clockwise_to(self.start_norm_vec);
            const auto b2 = offset.is_clockwise_to(self.stop_norm_vec);

            if(self.is_close) return b1 and b2;
            else return b1 or b2;
        } 
    };

    constexpr auto to_cache() const {
        const auto v1 = Vector2_t<real_t>::from_idenity_rotation(start_rad);
        const auto v2 = Vector2_t<real_t>::from_idenity_rotation(stop_rad);
        return Cache{
            .squ_inner_radius = square(inner_radius),
            .squ_outer_radius = square(outer_radius),
            .start_norm_vec = v1,
            .stop_norm_vec = v2,
            .is_close = v2.is_count_clockwise_to(v1)
        };
    }

    constexpr auto to_bounding_box() const {
        //TODO 更精细的包围盒划分
        return BoundingBox::from_center_and_halfsize(
            {0,0}, 
            {outer_radius, outer_radius}
        );
    }
};

struct RectBlob{
    real_t width;
    real_t height;

    __fast_inline constexpr bool is_covered(const Vector2_t<real_t> offset) const {
        return s_is_covered(*this, offset);
    }


    struct Cache{
        real_t half_width;
        real_t half_height;

        __fast_inline constexpr bool is_covered(const Vector2_t<real_t> offset) const {
            return s_is_covered(*this, offset);
        }
    private:
        __fast_inline static constexpr bool s_is_covered(const Cache & self, const Vector2_t<real_t> offset){
            return 
                (abs(offset.x) - (self.half_width) <= 0)
                and (abs(offset.y) - (self.half_height) <= 0)
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
private:
    __fast_inline static constexpr bool s_is_covered(const RectBlob & self, const Vector2_t<real_t> offset){
        return 
            (abs(offset.x) - (self.width >> 1) <= 0)
            and (abs(offset.y) - (self.height >> 1) <= 0)
        ;
    }
};

struct RotatedRect{
    real_t width;
    real_t height;
    real_t rotation;

    __fast_inline constexpr bool is_covered(const Vector2_t<real_t> offset) const {
        return s_is_covered(*this, offset);
    }
    struct alignas(4) Cache{
        real_t half_width;
        real_t half_height;
        real_t s;
        real_t c;

        __fast_inline constexpr bool is_covered(const Vector2_t<real_t> offset) const {
            return s_is_covered(*this, offset);
        }
    private:
        __fast_inline static constexpr bool s_is_covered(
            const Cache & self, const Vector2_t<real_t> offset){
            // -s * p.x + c * p.y;
            // -c * p.x - s * p.y;
            return 
                (abs(-self.s * offset.x + self.c * offset.y)
                    <= self.half_height) and
                (abs(-self.c * offset.x - self.s * offset.y) 
                    <= self.half_width)
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
        const auto rot = Vector2_t<real_t>::from_idenity_rotation(rotation);
        const std::array<Vector2_t<real_t>, 4> points = {
            get_raw_point<0>().improduct(rot),
            get_raw_point<1>().improduct(rot),
            get_raw_point<2>().improduct(rot),
            get_raw_point<3>().improduct(rot)
        };

        return BoundingBox::from_minimal_bounding_box(std::span(points));
    }

    template<size_t I>
    requires ((0 <= I) and (I < 4))
    constexpr Vector2_t<real_t> get_raw_point() const {
        switch(I){
            case 0: return {-width / 2, height / 2};
            case 1: return {width / 2, height / 2};
            case 2: return {-width / 2, -height / 2};
            case 3: return {width / 2, -height / 2};
            default: __builtin_unreachable();
        }
    }
private:
    __fast_inline static constexpr bool s_is_covered(const RotatedRect & self, const Vector2_t<real_t> offset){
        return 
            (abs(offset.x) - (self.width >> 1) <= 0)
            and (abs(offset.y) - (self.height >> 1) <= 0)
        ;
    }
};


// namespace static_test{
// static constexpr auto blob = RectBlob{
//     .width = 1,
//     .height = 1
// };

// static constexpr auto arc = AnnularSector{
//     .inner_radius = 1,
//     .outer_radius = 2,
    
//     .start_rad = 0,
//     .stop_rad = 1.5_r
// };

// static_assert(blob.is_covered({1,1}) == false);
// static_assert(blob.is_covered({0,0}) == true);

// // static_assert(arc.is_covered({1,1}) == true);
// // static_assert(arc.is_covered({0,0}) == false);
// // static_assert(arc.is_covered({-0.02_r,1.1_r}) == false);
// // static_assert(arc.is_covered({1.3_r,0.1_r}) == true);  // 0°方向
// // static_assert(arc.is_covered({-1.9_r,0}) == false); // 180°方向（超出角度范围）

// // static_assert(Vector2_t<real_t>(1,1).is_clockwise_to(Vector2_t<real_t>(1,0)));

// }


// static constexpr Vector2u CAMERA_SIZE = {94, 60};
// static constexpr Vector2u CAMERA_SIZE = {188, 120};
static constexpr Vector2u CAMERA_SIZE = {120, 80};
static constexpr Vector2u HALF_CAMERA_SIZE = CAMERA_SIZE / 2;

static constexpr Vector2 transform_camera_to_scene(
    const Vector2u pixel, const Gest2_t<real_t> viewpoint, const real_t scale) {
    const Vector2i pixel_offset = {
        int(pixel.x) - int(HALF_CAMERA_SIZE.x), 
        int(HALF_CAMERA_SIZE.y) - int(pixel.y)};

    const Vector2 camera_offset = Vector2(pixel_offset) * scale;
    const auto rot = viewpoint.rad - real_t(PI/2);
    return viewpoint.pos + camera_offset.rotated(rot);
}


namespace details{
    PRO_DEF_MEM_DISPATCH(MemIsCovered, is_covered);

    struct ElementFacade : pro::facade_builder
        ::support_copy<pro::constraint_level::none>
        ::add_convention<MemIsCovered, bool(const Vector2_t<real_t>) const>
        ::build {};
}




class SceneIntf{
public:
    SceneIntf() = default;
    SceneIntf(const SceneIntf &) = delete;
    SceneIntf(SceneIntf &&) = default;
    virtual ~SceneIntf() = default;
    virtual Image<Grayscale> render(const Gest2_t<real_t> viewpoint, const real_t scale) const = 0;
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

//     Image<Grayscale> render(const Gest2_t<real_t> viewpoint) const {
//         Image<Grayscale> ret{CAMERA_SIZE};

//         const auto org = transform_camera_to_scene({0,0}, viewpoint);
//         const auto y_step = transform_camera_to_scene({0,1}, viewpoint) - org;
//         const auto x_step = transform_camera_to_scene({1,0}, viewpoint) - org;
        
//         auto offset = org;
//         for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
//             const auto beg = offset;
//             for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
//                 const bool covered = this->is_covered(offset);
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

//     bool is_covered(const Vector2_t<real_t> offset) const{
//         for(const auto & element : elements_){
//             if(element->is_covered(offset)){
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


    Image<Grayscale> render(const Gest2_t<real_t> viewpoint, const real_t scale) const {
        static constexpr auto EXTENDED_BOUND_LENGTH = 1.3_r;
        const auto pdata = std::make_shared<uint8_t[]>(CAMERA_SIZE.x * CAMERA_SIZE.y);
        const auto org = transform_camera_to_scene({0,0}, viewpoint, scale);
        const auto y_step = transform_camera_to_scene({0,1}, viewpoint, scale) - org;
        const auto x_step = transform_camera_to_scene({1,0}, viewpoint, scale) - org;
        
        const auto window = Rect2_t<real_t>::from_corners(
            org, transform_camera_to_scene(CAMERA_SIZE, viewpoint, scale))
            .grow(EXTENDED_BOUND_LENGTH);
            
        bool dirty = false;
        auto apply_render = [&](const bool rough_judge_passed, auto && fn){
            if(!rough_judge_passed) return;
            dirty = true;
            using Fn = std::decay_t<decltype(fn)>;
            auto local_pos = org;
            for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
                const auto beg = local_pos;
                for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
                    const bool covered = std::forward<Fn>(fn)(local_pos);
                    const auto color = covered ? 255 : 0;
                    pdata[y * CAMERA_SIZE.x + x] |= color;
                    local_pos += x_step;
                }
                local_pos = beg;
                local_pos += y_step;
            }
        };

        std::apply([&](const auto&... object){
            (apply_render(
                window.intersects(object.bounding_box.shift(object.placement.pos)),
                [&](const Vector2_t<real_t> local_pos) { 
                    return object.cache.is_covered(local_pos - object.placement.pos); 
                }), ...);
        }, objects_);

        if(!dirty) std::memset(pdata.get(), 0, CAMERA_SIZE.x * CAMERA_SIZE.y);
        return Image<Grayscale>(std::move(
            std::reinterpret_pointer_cast<Grayscale[]>(pdata)), CAMERA_SIZE);
    }
private:
    std::tuple<Objects...> objects_;  // Expanded parameter pack
};


template<typename... Objects>
static constexpr auto make_static_scene(Objects && ... objects){
    return StaticScene<Objects...>(std::forward<Objects>(objects)...);
}

}
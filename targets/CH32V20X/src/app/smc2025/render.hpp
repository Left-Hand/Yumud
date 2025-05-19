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


// struct ElementWithPlacement;

struct Placement{
    Vector2_t<real_t> pos;

    real_t rotation = 0;
    real_t scale = 1;

    Vector2 apply_transform(Vector2 p) const {
    return (p * scale).rotated(rotation) + pos;
    }
};

template<typename T>
struct cache_of{
    using type = typename T::Cache;
};

template<typename T>
using cache_of_t = cache_of<T>::type;

template<typename T>
struct ElementWithPlacement{
    using Cache = cache_of_t<T>;
    T element;
    Placement place;
    Cache cache;
};

template<typename T>
constexpr ElementWithPlacement<T> operator | (const T & element, const Placement& place){
    return ElementWithPlacement<T>{
        .element = element,
        .place = place,
        .cache = element.to_cache()
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

    struct Cache{
        real_t squ_inner_radius;
        real_t squ_outer_radius;
        Vector2_t<real_t> start_norm_vec;
        Vector2_t<real_t> stop_norm_vec;

        constexpr bool is_covered(const Vector2_t<real_t> offset) const {
            return s_is_covered(*this, offset);
        }
    private:
        static constexpr bool s_is_covered(const Cache & self, const Vector2_t<real_t> offset){
            const auto len_squ = offset.length_squared();

            if((len_squ - self.squ_outer_radius) * (len_squ - self.squ_inner_radius) > 0)
                return false;

            if(offset.is_clockwise_to(self.start_norm_vec)) return false;
            if(offset.is_count_clockwise_to(self.stop_norm_vec)) return false;

            return true;
        } 
    };

    constexpr auto to_cache() const {
        return Cache{
            .squ_inner_radius = square(inner_radius),
            .squ_outer_radius = square(outer_radius),
            .start_norm_vec = Vector2_t<real_t>::from_idenity_rotation(start_rad),
            .stop_norm_vec = Vector2_t<real_t>::from_idenity_rotation(stop_rad)
        };
    }
};

struct RectBlob{
    // real_t x;
    // real_t y;
    
    real_t width;
    real_t height;

    __fast_inline constexpr bool is_covered(const Vector2_t<real_t> offset) const {
        return s_is_covered(*this, offset);
    }

    // constexpr Rect2_t<real_t> get_bounding_box() const {
    //     return Rect2_t<real_t>{x,y,width, height};
    // }

    struct Cache{
        real_t half_width;
        real_t half_height;

        __fast_inline constexpr bool is_covered(const Vector2_t<real_t> offset) const {
            return s_is_covered(*this, offset);
        }
    private:
        __fast_inline static constexpr bool s_is_covered(const Cache & self, const Vector2_t<real_t> offset){
            return 
                (ABS(offset.x) - (self.half_width) <= 0)
                and (ABS(offset.y) - (self.half_height) <= 0)
            ;
        }
    };

    constexpr auto to_cache() const {
        return Cache{
            .half_width = width / 2,
            .half_height = height / 2,
        };
    }
private:
    __fast_inline static constexpr bool s_is_covered(const RectBlob & self, const Vector2_t<real_t> offset){
        //fastest solution by benchmark
        // return self.to_cache().is_covered({offset.x - self.x, offset.y - self.y});
        return 
            // IN_RANGE(offset.x - self.x,  - (self.width >> 1), (self.width >> 1))
            // (square(offset.x - self.x) - square(self.width >> 1) <= 0)
            (ABS(offset.x) - (self.width >> 1) <= 0)
            and (ABS(offset.y) - (self.height >> 1) <= 0)
            // std::signbit(ABS(offset.x - self.x) - (self.width >> 1))
            // and std::signbit(ABS(offset.y - self.y) - (self.height >> 1))
            // and IN_RANGE(offset.y - self.y, - (self.height >> 1), (self.height >> 1));
        ;

        // const auto width_squ_4 = square(self.width >> 1);
        // const auto height_squ_4 = square(self.height >> 1);

        // return (std::bit_cast<uint32_t>((square(offset.x - self.x) - width_squ_4).value.to_i32()) & 
        //         std::bit_cast<uint32_t>((square(offset.y - self.y) - height_squ_4).value.to_i32()))
        //         & 0x80000000;
    }

    // template<size_t Q>
    // static constexpr bool s_is_covered(const Rect2_t<iq_t<Q>> rect, const Vector2_t<iq_t<Q>> offset){
    //     auto fast_square = [](int32_t qn){ 
    //         return int64_t(qn) * int64_t(qn);
    //     }; 

    //     const auto qn_x_err_squ = fast_square((offset.x - rect.x).value.to_i32());
    //     const auto qn_y_err_squ = fast_square((offset.y - rect.y).value.to_i32());
        
    // }
};


namespace static_test{
static constexpr auto blob = RectBlob{
    .width = 1,
    .height = 1
};

static constexpr auto arc = AnnularSector{
    .inner_radius = 1,
    .outer_radius = 2,
    
    .start_rad = 0,
    .stop_rad = 1.5_r
};

static_assert(blob.is_covered({1,1}) == false);
static_assert(blob.is_covered({0,0}) == true);

// static_assert(arc.is_covered({1,1}) == true);
// static_assert(arc.is_covered({0,0}) == false);
// static_assert(arc.is_covered({-0.02_r,1.1_r}) == false);
// static_assert(arc.is_covered({1.3_r,0.1_r}) == true);  // 0°方向
// static_assert(arc.is_covered({-1.9_r,0}) == false); // 180°方向（超出角度范围）

// static_assert(Vector2_t<real_t>(1,1).is_clockwise_to(Vector2_t<real_t>(1,0)));

}

static constexpr real_t PIXELS_PER_METER = 70;
static constexpr real_t METERS_PER_PIXEL = 1 / PIXELS_PER_METER;
// static constexpr Vector2u CAMERA_SIZE = {188, 120};
static constexpr Vector2u CAMERA_SIZE = {94, 60};
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




class SceneIntf{
public:
    SceneIntf() = default;
    SceneIntf(const SceneIntf &) = delete;
    SceneIntf(SceneIntf &&) = default;
    virtual ~SceneIntf() = default;
    virtual Image<Grayscale> render(const Ray2_t<real_t> viewpoint) const = 0;
};

class DynamicScene final:public SceneIntf{
public:
    DynamicScene(){;}

    template<typename T>
    void add_element(T && element){
        elements_.emplace_back(
            pro::make_proxy<details::ElementFacade>(
                std::forward<T>(element))
        );
    }

    Image<Grayscale> render(const Ray2_t<real_t> viewpoint) const {
        Image<Grayscale> ret{CAMERA_SIZE};

        const auto org = transform_camera_to_scene({0,0}, viewpoint);
        const auto y_step = transform_camera_to_scene({0,1}, viewpoint) - org;
        const auto x_step = transform_camera_to_scene({1,0}, viewpoint) - org;
        
        auto offset = org;
        for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
            const auto beg = offset;
            for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
                const bool covered = this->is_covered(offset);
                ret.set_pixel({x,y}, covered ? Grayscale{255} : Grayscale{0});
                offset += x_step;
            }
            offset = beg;
            offset += y_step;
        }

        return ret;
    }
private:
    std::vector<pro::proxy<details::ElementFacade>> elements_;

    bool is_covered(const Vector2_t<real_t> offset) const{
        for(const auto & element : elements_){
            if(element->is_covered(offset)){
                return true;
            }
        }
        return false;
    }
};


template<typename ...Objects>
class StaticScene final:public SceneIntf{ 
public:
    using Container = std::tuple<Objects...>;

    ~StaticScene() = default;
    constexpr StaticScene(Objects&&... objects):
        objects_(std::make_tuple(std::forward<Objects>(objects)...)){}


    Image<Grayscale> render(const Ray2_t<real_t> viewpoint) const {
        Image<Grayscale> ret{CAMERA_SIZE};

        const auto org = transform_camera_to_scene({0,0}, viewpoint);
        const auto y_step = transform_camera_to_scene({0,1}, viewpoint) - org;
        const auto x_step = transform_camera_to_scene({1,0}, viewpoint) - org;
        
        auto pos = org;
        for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
            const auto beg = pos;
            for(size_t x = 0; x < CAMERA_SIZE.x; ++x){
                const bool covered = this->is_covered(pos);
                const auto color = covered ? Grayscale{255} : Grayscale{0};
                ret.set_pixel({x,y}, color);
                pos += x_step;
            }
            pos = beg;
            pos += y_step;
        }

        return ret;
    }
private:
    // Objects elements_;

    // template<typename T>
    // using element_with_placement_t = ElementWithPlacement<T>;

    std::tuple<Objects...> objects_;  // Expanded parameter pack

    // constexpr bool is_covered(const Vector2_t<real_t> offset) const { 
    //     return std::apply([&](const auto&... objects){
    //         return (objects->is_covered(offset) || ...);
    //     }, objects);
    // }

    const bool is_covered(const Vector2_t<real_t> pos) const { 
        return std::apply([&](const auto&... object){
            return (object.cache.is_covered(pos - object.place.pos) || ...);  // Fixed member access
            // return ((objects.get_bounding_box().has_point(offset) ? objects.is_covered(offset) : false) || ...);  // Fixed member access
        }, objects_);
    }
};


template<typename... Objects>
static constexpr auto make_static_scene(Objects && ... objects){
    return StaticScene<Objects...>(std::forward<Objects>(objects)...);
}

}
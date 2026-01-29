#pragma once

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"
#include "primitive/image/painter/painter.hpp"

#include "algebra/vectors/vec3.hpp"
#include "algebra/shapes/box_rect.hpp"
#include "algebra/shapes/annular_sector.hpp"
#include "algebra/shapes/rotated_rect.hpp"

#include "middlewares/nvcv2/shape/shape.hpp"

#include "algebra/regions/ray2.hpp"
#include "algebra/regions/line2.hpp"
#include "algebra/gesture/isometry2.hpp"

// static constexpr math::Vec2u CAMERA_SIZE = {94/2, 60/2};
// static constexpr math::Vec2u CAMERA_SIZE = {94, 60};
static constexpr math::Vec2u CAMERA_SIZE = {94 * 3 / 2, 60 * 3 / 2};
// static constexpr math::Vec2u CAMERA_SIZE = {188, 120};
// static constexpr math::Vec2u CAMERA_SIZE = {120, 80};
// static constexpr math::Vec2u CAMERA_SIZE = {120, 80};
static constexpr math::Vec2u HALF_CAMERA_SIZE = CAMERA_SIZE / 2;
static constexpr uint8_t WHITE_COLOR = 0x9f;


using namespace ymd;

template<typename T>
using cache_of_t = CacheOf<T, bool>;




using BoundingBox = math::Rect2<iq16>;

namespace ymd{
struct RotatedZebraRect{
    iq16 width;
    iq16 height;
    Angular<iq16> rotation;


    template<size_t I>
    requires ((0 <= I) and (I < 4))
    constexpr math::Vec2<iq16> get_corner() const {
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

    iq16 half_width;
    iq16 half_height;
    iq16 s;
    iq16 c;

    static constexpr Self from(const RotatedZebraRect & obj){
        const auto [s,c] = obj.rotation.sincos();
        return Self{
            .half_width = obj.width / 2,
            .half_height = obj.height / 2,
            .s = s,
            .c = c
        };
    }

    __fast_inline constexpr uint8_t color_from_point(const math::Vec2<iq16> offset) const {
        return s_color_from_point(*this, offset);
    }
private:
    __fast_inline static constexpr uint8_t s_color_from_point(
        const Self & self, const math::Vec2<iq16> offset){
        // -s * p.x + c * p.y;
        // -c * p.x - s * p.y;
        const auto x_offset = - self.c * offset.x - self.s * offset.y;
        
        return 
            ((math::abs(-self.s * offset.x + self.c * offset.y)
                <= self.half_height) and
            (math::abs(x_offset) 
                <= self.half_width))
                
            // ? (static_cast<uint8_t>(CLAMP(math::sinpu(x_offset * 15) * 3 + 1, 0, 2) * WHITE_COLOR / 2)) : 0
            ? (WHITE_COLOR) : 0
        ;
    }
};

template<>
struct BoundingBoxOf<RotatedZebraRect>{


    static constexpr BoundingBox bounding_box(const RotatedZebraRect & obj){
        const auto rotation = math::Vec2<iq16>::from_angle(obj.rotation);
        const std::array<math::Vec2<iq16>, 4> points = {
            obj.get_corner<0>().improduct(rotation),
            obj.get_corner<1>().improduct(rotation),
            obj.get_corner<2>().improduct(rotation),
            obj.get_corner<3>().improduct(rotation)
        };

        return BoundingBox::from_minimal_bounding_box(std::span(points));
    }

};


struct SpotLight final{
    iq16 radius = 1;
};


template<>
struct CacheOf<SpotLight, bool>{
    using Self = CacheOf<SpotLight, bool>;

    iq16 squ_radius;

    static constexpr auto from(const SpotLight & obj) {
        return CacheOf{obj.radius * obj.radius};
    }

    __fast_inline constexpr uint8_t color_from_point(const math::Vec2<iq16> offset) const {
        return s_color_from_point(*this, offset);
    }
private:
    __fast_inline static constexpr uint8_t s_color_from_point(
        const Self & self, 
        const math::Vec2<iq16> offset
    ){
        const auto len_squ = offset.length_squared();
        // const auto temp = MAX(9 * len_squ, 1);
        // return uint8_t(130 / temp);
        const auto temp = CLAMP(2 - 3 * static_cast<iq16>(math::sqrt(static_cast<uq16>(len_squ))),0, 1);
        return uint8_t(130 * temp);
    }
};

template<>
struct BoundingBoxOf<SpotLight>{
    static constexpr auto bounding_box(const SpotLight & obj){
        return math::Rect2u16{
            math::Vec2u16{static_cast<uint16_t>(-obj.radius), 
                static_cast<uint16_t>(-obj.radius)}, 
            math::Vec2u16{
                static_cast<uint16_t>(2 * obj.radius), 
                static_cast<uint16_t>(2 * obj.radius)}
        };
    }
};


}

namespace ymd::smc::sim{

struct Placement{
    math::Vec2<iq16> translation;
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


struct PrecomputedTable final{
    using Self = PrecomputedTable;
    struct [[nodiscard]] Config{
        size_t width;
        uq16 fov;
    };

    explicit PrecomputedTable(const Config & config){
        reconf(config);
    }

    void reconf(const Config & cfg){
        if (cfg.width == 0 || cfg.fov == 0) PANIC{};

        const auto width = cfg.width;
        const auto half_fov = cfg.fov / 2;
        const auto theta_begin = -iq16(half_fov);
        // const auto theta_end = half_fov;
        const auto theta_step = cfg.fov / width;

        table_ = std::make_unique<iq16[]>(width);
        // for(auto theta = theta_begin; theta < theta_end; theta += theta_step){
        //     table_[static_cast<size_t>(theta)] = math::tan(theta);
        // }
        for(size_t i = 0; i < width; ++i){
            table_[i] = math::tan(static_cast<iq16>(theta_begin + i * theta_step));
        }

        width_ = width;
    }

    [[nodiscard]] constexpr iq16 operator()(const iq16 x) const {
        const int32_t idx = static_cast<int32_t>(x * width_) + (width_ >> 1);
        if(idx < 0 or static_cast<size_t>(idx) >= width_) PANIC{};
        return table_.get()[static_cast<size_t>(idx)];
    }

    [[nodiscard]] constexpr size_t width() const {
        return width_;
    }

    [[nodiscard]] constexpr iq16 operator[](const size_t i) const {
        return table_[i];
    }

    [[nodiscard]] constexpr iq16 at(const size_t i) const{
        if(i >= width_) PANIC{};
        return table_[i];
    }
private:    
    std::unique_ptr<iq16[]> table_;
    size_t width_;
};

//将相机像素转换为地面坐标
static constexpr math::Vec2<iq16> project_pixel_to_ground(
    const math::Vec2u pixel, 
    const math::Isometry2<iq16> pose, 
    const iq16 zoom
) {
    const math::Vec2i pixel_offset = {
        int(pixel.x) - int(HALF_CAMERA_SIZE.x), 
        int(HALF_CAMERA_SIZE.y) - int(pixel.y)};

    const math::Vec2<iq16> camera_offset = math::Vec2<iq16>(pixel_offset) * zoom;
    const auto rotation = pose.rotation.backward_90deg();
    return pose.translation + rotation * camera_offset;
}


static constexpr math::Vec2u project_ground_to_pixel(
    const math::Vec2<iq16>& ground_pos,
    const math::Isometry2<iq16> pose,
    const iq16 zoom
){
    // 1. Remove pose translation offset
    const math::Vec2<iq16> relative_pos = ground_pos - pose.translation;
    
    // 2. Calculate inverse rotation (original rotation was pose.rotation - PI/2)
    const auto [s, c] = (-(pose.rotation.backward_90deg())).sincos();
    
    // 3. Apply inverse rotation matrix (transpose of original rotation matrix)
    const math::Vec2<iq16> unrotated = {
        relative_pos.x * c - relative_pos.y * s,
        relative_pos.x * s + relative_pos.y * c
    };
    
    // 4. Remove scaling and convert to pixel space
    const math::Vec2<iq16> pixel_offset = unrotated / zoom;
    
    // 5. Convert to camera coordinates and clamp to pixel grid
    return math::Vec2u{
        math::round_cast<uint>(pixel_offset.x + HALF_CAMERA_SIZE.x),
        math::round_cast<uint>(HALF_CAMERA_SIZE.y - pixel_offset.y)
    };
}

namespace details{
    PRO_DEF_MEM_DISPATCH(MemIsCovered, color_from_point);

    struct ElementFacade : pro::facade_builder
        ::support_copy<pro::constraint_level::none>
        ::add_convention<MemIsCovered, bool(const math::Vec2<iq16>) const>
        ::build {};
}

struct ViewPoint{
    math::Vec3<iq16> position;
    Angular<iq16> yaw;
    Angular<iq16> pitch;
    Angular<iq16> fov;
};


class SceneIntf{
public:
    SceneIntf() = default;
    SceneIntf(const SceneIntf &) = delete;
    SceneIntf(SceneIntf &&) = default;
    virtual ~SceneIntf() = default;
    virtual Image<Gray> render(const math::Isometry2<iq16> pose, const iq16 zoom) const = 0;
};

template<typename ...Objects>
class StaticScene final:public SceneIntf{ 
public:
    using Container = std::tuple<Objects...>;

    ~StaticScene() = default;
    constexpr StaticScene(Objects&&... objects):
        objects_(std::make_tuple(std::forward<Objects>(objects)...)){}


    Image<Gray> render(const math::Isometry2<iq16> pose, const iq16 zoom) const {
        // static constexpr auto EXTENDED_BOUND_LENGTH = 1.3_r;
        const auto pbuf = std::make_shared<uint8_t[]>(CAMERA_SIZE.x * CAMERA_SIZE.y);
        const auto org =    project_pixel_to_ground({0,0}, pose, zoom);
        const auto y_step = project_pixel_to_ground({0,1}, pose, zoom) - org;
        const auto x_step = project_pixel_to_ground({1,0}, pose, zoom) - org;

        const auto ground_bbox = math::Rect2<iq16>::from_minimal_bounding_box({
            org, 
            project_pixel_to_ground({CAMERA_SIZE.x,0},    pose, zoom),
            project_pixel_to_ground({0,CAMERA_SIZE.y},    pose, zoom),
            project_pixel_to_ground(CAMERA_SIZE,          pose, zoom)
        });

            
        bool dirty = false;
        auto apply_render = [&]<typename Fn>(const bool rough_judge_passed, Fn && fn){
            if(!rough_judge_passed) return;
            dirty = true;
            auto local_pos = org;
            for(size_t y = 0; y < CAMERA_SIZE.y; ++y){
                const auto beg = local_pos;
                auto * row_begin = &pbuf[y * CAMERA_SIZE.x];
                auto * ptr = row_begin;
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
                ground_bbox.intersects(object.bounding_box.shift(object.placement.translation)),
                // true,
                [&](const math::Vec2<iq16> local_pos) { 
                    return object.cache.color_from_point(local_pos - object.placement.translation); 
                }), ...);
        }, objects_);

        if(!dirty) std::fill_n(pbuf.get(), CAMERA_SIZE.x * CAMERA_SIZE.y, 0);
        return Image<Gray>(std::move(
            std::reinterpret_pointer_cast<Gray[]>(pbuf)), CAMERA_SIZE);
    }

    Image<Gray> render2(const ViewPoint & view_point) const {
        // static constexpr auto EXTENDED_BOUND_LENGTH = 1.3_r;
        const auto pbuf = std::make_shared<uint8_t[]>(CAMERA_SIZE.x * CAMERA_SIZE.y);
        const auto p0 =    math::Vec2(view_point.position.x, view_point.position.y);

        const auto norm_dir1 = math::UnitVec2<iq16>::from_angle(view_point.yaw);
        const auto norm_dir2 = math::UnitVec2<iq16>(-norm_dir1.y, norm_dir1.x);
        const auto h = view_point.position.z;
        const auto half_fov = view_point.fov / 2;

        auto project_pixel_to_ground_pp = [&](const Angular<iq16> alpha, const Angular<iq16> beta){
            const auto d = h * math::cot(alpha);
            const auto p1 = p0 + d * norm_dir1;
            const auto l = h / iq16(math::sin(alpha));
            const auto s = l * math::tan(beta);
        };
        const auto ground_bbox = math::Rect2<iq16>::from_minimal_bounding_box({
            p0, p0, p0, p0
        });

            
        bool dirty = false;
        auto apply_render = [&]<typename Fn>(const bool rough_judge_passed, Fn && fn){
            if(!rough_judge_passed) return;
            dirty = true;

        };

        std::apply([&](const auto&... object){
            (apply_render(
                ground_bbox.intersects(object.bounding_box.shift(object.placement.translation)),
                // true,
                [&](const math::Vec2<iq16> local_pos) { 
                    return object.cache.color_from_point(local_pos - object.placement.translation); 
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
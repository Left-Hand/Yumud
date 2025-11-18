#pragma once

#include "types/shapes/prelude.hpp"
#include "types/regions/rect2.hpp"
#include "primitive/arithmetic/angle_range.hpp"

namespace ymd{

template<typename T, typename D>
struct AnnularSector;

template<typename T, typename D>
struct BoundingBoxOf<AnnularSector<T, D>>;


template<typename T, typename D>
struct AnnularSector final{
    static_assert(not std::is_integral_v<D>);

    Vec2<T> center;
    Range2<T> radius_range;
    AngleRange<D> angle_range;

    constexpr bool contains_angle(const Angle<D> angle) const {
        return angle_range.contains_angle(angle);
    }

    constexpr 
    Rect2<T> bounding_box() const {
        auto & self = *this;
        const bool x_reached_left = self.angle_range.
            contains_angle(Angle<D>::HALF_LAP);
        const bool x_reached_right = self.angle_range.
            contains_angle(Angle<D>::ZERO);
        const bool y_reached_top = self.angle_range.
            contains_angle(Angle<D>::QUARTER_LAP);
        const bool y_reached_bottom = self.angle_range.
            contains_angle(Angle<D>::NEG_QUARTER_LAP);

        const Vec2<iq16> n1 = Vec2<iq16>::from_angle(self.angle_range.start);
        const Vec2<iq16> n2 = Vec2<iq16>::from_angle(self.angle_range.stop());

        const Vec2<iq16> p1 = (n1).flip_y() * self.radius_range.stop;
        const Vec2<iq16> p2 = (n2).flip_y() * self.radius_range.stop;

        const Vec2<iq16> p3 = (n1).flip_y() * self.radius_range.start;
        const Vec2<iq16> p4 = (n2).flip_y() * self.radius_range.start;

        const T x_min = floor_cast<T>(self.center.x + (x_reached_left ?     
            static_cast<iq16>(-self.radius_range.stop) : MIN(p1.x, p2.x, p3.x, p4.x)));
        const T y_min = floor_cast<T>(self.center.y + (y_reached_top ?      
            static_cast<iq16>(-self.radius_range.stop) : MIN(p1.y, p2.y, p3.y, p4.y)));

        const T x_max = ceil_cast<T>(self.center.x + (x_reached_right ?    
            static_cast<iq16>(self.radius_range.stop) : MAX(p1.x, p2.x, p3.x, p4.x)));
        const T y_max = ceil_cast<T>(self.center.y + (y_reached_bottom ?   
            static_cast<iq16>(self.radius_range.stop) : MAX(p1.y, p2.y, p3.y, p4.y)));

        return Rect2<T>(
            Vec2<T>{
                x_min, y_min
            },

            Vec2<T>{
                static_cast<T>(x_max - x_min), 
                static_cast<T>(y_max - y_min)
            }
        );
    }

    __fast_inline constexpr bool contains_point(
        const Vec2<T>& p
    ) const{
        const auto offset = p - center;
        const auto p_r_squ = offset.length_squared();
        if(p_r_squ < square(radius_range.start)) return false;
        if(p_r_squ > square(radius_range.stop)) return false;
        return contains_angle(offset.angle());
    }

    __fast_inline constexpr bool contains_norm_dir(
        const Vec2<D> norm_dir_vec
    ) const {
        const auto n1 = Vec2<D>::from_angle(angle_range.start);
        const auto n2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            n1, n2, n2.is_counter_clockwise_to(n1)
        );
    }

    friend class BoundingBoxOf<AnnularSector<T, D>>;

private:
    static constexpr bool contains_angle_helper(
        const T angle,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        return contains_angle_helper(
            Vec2<D>::from_angle(angle), 
            start_norm_vec, 
            stop_norm_vec, 
            is_minor
        );
    }

    static constexpr bool contains_angle_helper(
        const Vec2<T> offset,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec);
        if(is_minor) return b1 and b2;
        else return b1 or b2;
    }
};

template<typename T, typename D>
struct is_placed_t<AnnularSector<T, D>>:std::true_type{;};


template<typename T, typename D>
struct RoundedAnnularSector final{
    static_assert(not std::is_integral_v<D>);

    Vec2<T> center;
    Range2<T> radius_range;
    AngleRange<D> angle_range;

    constexpr 
    Rect2<T> bounding_box() const {
        auto & self = *this;
        const bool x_reached_left = self.angle_range.
            contains_angle(Angle<D>::HALF_LAP);
        const bool x_reached_right = self.angle_range.
            contains_angle(Angle<D>::ZERO);
        const bool y_reached_top = self.angle_range.
            contains_angle(Angle<D>::QUARTER_LAP);
        const bool y_reached_bottom = self.angle_range.
            contains_angle(Angle<D>::NEG_QUARTER_LAP);

        const Vec2<iq16> n1 = Vec2<iq16>::from_angle(self.angle_range.start);
        const Vec2<iq16> n2 = Vec2<iq16>::from_angle(self.angle_range.stop());

        const Vec2<iq16> p1 = (n1).flip_y() * self.radius_range.stop;
        const Vec2<iq16> p2 = (n2).flip_y() * self.radius_range.stop;

        const Vec2<iq16> p3 = (n1).flip_y()* self.radius_range.start;
        const Vec2<iq16> p4 = (n2).flip_y() * self.radius_range.start;

        const T x_min = floor_cast<T>(self.center.x + (x_reached_left ?     
            static_cast<iq16>(-self.radius_range.stop) : MIN(p1.x, p2.x, p3.x, p4.x)));
        const T y_min = floor_cast<T>(self.center.y + (y_reached_top ?      
            static_cast<iq16>(-self.radius_range.stop) : MIN(p1.y, p2.y, p3.y, p4.y)));

        const T x_max = ceil_cast<T>(self.center.x + (x_reached_right ?    
            static_cast<iq16>(self.radius_range.stop) : MAX(p1.x, p2.x, p3.x, p4.x)));
        const T y_max = ceil_cast<T>(self.center.y + (y_reached_bottom ?   
            static_cast<iq16>(self.radius_range.stop) : MAX(p1.y, p2.y, p3.y, p4.y)));

        return Rect2<T>(
            Vec2<T>{
                x_min, y_min
            },

            Vec2<T>{
                static_cast<T>(x_max - x_min), 
                static_cast<T>(y_max - y_min)
            }
        );
    }

    __fast_inline constexpr bool contains_norm_dir(
        const Vec2<D> norm_dir_vec
    ) const {
        const auto n1 = Vec2<D>::from_angle(angle_range.start);
        const auto n2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            n1, n2, n2.is_counter_clockwise_to(n1)
        );
    }

    friend class BoundingBoxOf<RoundedAnnularSector<T, D>>;

private:
    static constexpr bool contains_angle_helper(
        const T angle,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        return contains_angle_helper(
            Vec2<D>::from_angle(angle), 
            start_norm_vec, 
            stop_norm_vec, 
            is_minor
        );
    }

    static constexpr bool contains_angle_helper(
        const Vec2<T> offset,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec);
        if(is_minor) return b1 and b2;
        else return b1 or b2;
    }

    static constexpr Rect2<T> bounding_box_of_square(const Vec2<T> pos, const T radius){
            const auto ret = Rect2<T>::from_uninitialied();

            ret.left_top.x = pos.x - radius;
            ret.left_top.y = pos.y - radius;
            ret.size.x = radius * 2;
            ret.size.y = radius * 2;

            return ret;
        };
};

template<typename T, typename D>
struct is_placed_t<RoundedAnnularSector<T, D>>:std::true_type{;};



template<typename T, typename D>
struct Sector final{
    static_assert(not std::is_integral_v<D>);

    Vec2<T> center;
    T radius;
    AngleRange<D> angle_range;

    constexpr bool contains_angle(const Angle<D> angle) const {
        return angle_range.contains_angle(angle);
    }

    constexpr 
    Rect2<T> bounding_box() const {
        auto & self = *this;

        const bool x_reached_left = 
            self.angle_range.contains_angle(Angle<D>::HALF_LAP);

        const bool x_reached_right = 
            self.angle_range.contains_angle(Angle<D>::ZERO);

        const bool y_reached_top = 
            self.angle_range.contains_angle(Angle<D>::QUARTER_LAP);

        const bool y_reached_bottom = 
            self.angle_range.contains_angle(Angle<D>::NEG_QUARTER_LAP);

        const Vec2<iq16> p1 = Vec2<iq16>::from_angle(self.angle_range.start)
            .flip_y() * self.radius;
        const Vec2<iq16> p2 = Vec2<iq16>::from_angle(self.angle_range.stop())
            .flip_y() * self.radius;

        const T x_min = floor_cast<T>(self.center.x + (x_reached_left ?     
            static_cast<iq16>(-self.radius) : MIN(p1.x, p2.x, 0)));
        const T y_min = floor_cast<T>(self.center.y + (y_reached_top ?      
            static_cast<iq16>(-self.radius) : MIN(p1.y, p2.y, 0)));

        const T x_max = ceil_cast<T>(self.center.x + (x_reached_right ?    
            static_cast<iq16>(self.radius) : MAX(p1.x, p2.x, 0)));
        const T y_max = ceil_cast<T>(self.center.y + (y_reached_bottom ?   
            static_cast<iq16>(self.radius) : MAX(p1.y, p2.y, 0)));

        return Rect2<T>(
            Vec2<T>{
                x_min, y_min
            },

            Vec2<T>{
                static_cast<T>(x_max - x_min), 
                static_cast<T>(y_max - y_min)
            }
        );
    }

    __fast_inline constexpr bool contains_point(
        const Vec2<T>& p
    ) const{
        const auto offset = p - center;
        const auto p_r_squ = offset.length_squared();
        if(p_r_squ > square(radius)) return false;
        return contains_angle(offset.angle());
    }

    __fast_inline constexpr bool contains_norm_dir(
        const Vec2<D> norm_dir_vec
    ) const {
        const auto n1 = Vec2<D>::from_angle(angle_range.start);
        const auto n2 = Vec2<D>::from_angle(angle_range.stop());
        return contains_angle_helper(
            norm_dir_vec,
            n1, n2, n2.is_counter_clockwise_to(n1)
        );
    }

    friend class BoundingBoxOf<Sector<T, D>>;

private:
    static constexpr bool contains_angle_helper(
        const T angle,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        return contains_angle_helper(
            Vec2<D>::from_angle(angle), 
            start_norm_vec, 
            stop_norm_vec, 
            is_minor
        );
    }

    static constexpr bool contains_angle_helper(
        const Vec2<T> offset,
        const Vec2<D> start_norm_vec,
        const Vec2<D> stop_norm_vec,
        const bool is_minor
    ){
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec);
        if(is_minor) return b1 and b2;
        else return b1 or b2;
    }
};

template<typename T, typename D>
struct is_placed_t<Sector<T, D>>:std::true_type{;};



template<typename T, typename D>
struct DrawDispatchIterator<Sector<T, D>> {
    using Shape = Sector<T, D>;
    using Self = DrawDispatchIterator<Sector<T, D>>;

    struct CtorHelper{
        Range2<T> y_range;
        Range2<T> x_range;
        Vec2<iq16> v1;
        Vec2<iq16> v2;

        static constexpr CtorHelper from(const Shape & shape){
            const auto angle_range = shape.angle_range;
            const auto bb = shape.bounding_box(); 
            return {                
                .y_range = bb.y_range(),
                .x_range = bb.x_range(),
                .v1 = Vec2<iq16>::from_angle(Angle<iq16>::from_turns(
                    angle_range.start.to_turns())),
                .v2 = Vec2<iq16>::from_angle(Angle<iq16>::from_turns(
                    angle_range.stop().to_turns()))
            };
        }
    };


    constexpr DrawDispatchIterator(const Shape & shape):
        DrawDispatchIterator(shape, CtorHelper::from(shape)){;}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return y_ < y_stop_;
    }

    // 推进到下一行
    constexpr void forward() {
        y_++;
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename DestColor>
    Result<void, typename Target::Error> draw_filled(Target& target, const DestColor & dest_color) {
        const auto color = static_cast<RGB565>(dest_color);

        #pragma GCC unroll(4)
        for(T i = x_range_.start; i < x_range_.stop; i++){
            const bool is_inside = contains_point(Vec2<T>{i, y_});
            if(not is_inside) continue;
            if(const auto res = target.draw_x_unchecked(i, color);
                res.is_err()) return Err(res.unwrap_err());
            continue;
        }

        return Ok();
    }

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        return Ok();
    }


private:
    T y_;
    T y_stop_;
    Range2<T> x_range_;
    Vec2<T> center_;
    uint32_t squ_outer_radius_;
    Vec2<iq16> start_norm_vec_;
    Vec2<iq16> stop_norm_vec_;
    bool is_minor_;

    using ST = std::make_signed_t<T>;

    __fast_inline constexpr bool contains_point(const Vec2<T> p){
        const Vec2<int32_t> offset = (Vec2<int32_t>(p) - Vec2<int32_t>(center_)).flip_y();
        const uint32_t len_squ = static_cast<uint32_t>(offset.length_squared());

        if (len_squ > squ_outer_radius_) return false;

        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec_);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec_);

        return is_minor_ ? (b1 && b2) : (b1 || b2);

        return true;
    }

    constexpr DrawDispatchIterator(const Shape & shape, const CtorHelper & helper):
        y_(helper.y_range.start),
        y_stop_(helper.y_range.stop),
        x_range_(helper.x_range),
        center_(shape.center),
        squ_outer_radius_(static_cast<uint32_t>(
            square(static_cast<iq12>(shape.radius)))),
        start_norm_vec_(helper.v1),
        stop_norm_vec_(helper.v2),
        is_minor_(helper.v2.is_counter_clockwise_to(helper.v1))
        {;}
};



template<typename T, typename D>
struct DrawDispatchIterator<AnnularSector<T, D>> {
    using Shape = AnnularSector<T, D>;
    using Self = DrawDispatchIterator<AnnularSector<T, D>>;

    struct CtorHelper{
        Range2<T> y_range;
        Range2<T> x_range;
        Vec2<iq16> v1;
        Vec2<iq16> v2;

        static constexpr CtorHelper from(const Shape & shape){
            const auto angle_range = shape.angle_range;
            const auto bb = shape.bounding_box(); 
            return {                
                .y_range = bb.y_range(),
                .x_range = bb.x_range(),
                .v1 = Vec2<iq16>::from_angle(Angle<iq16>::from_turns(
                    angle_range.start.to_turns())),
                .v2 = Vec2<iq16>::from_angle(Angle<iq16>::from_turns(
                    angle_range.stop().to_turns()))
            };
        }
    };


    constexpr DrawDispatchIterator(const Shape & shape):
        DrawDispatchIterator(shape, CtorHelper::from(shape)){;}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return y_ < y_stop_;
    }

    // 推进到下一行
    constexpr void forward() {
        y_++;
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename DestColor>
    Result<void, typename Target::Error> draw_filled(Target& target, const DestColor& dest_color) {
        const auto x_range = x_range_;
        const auto color = static_cast<RGB565>(dest_color);

        #pragma GCC unroll(4)
        for(T i = x_range.start; i < x_range.stop; i++){
            const bool is_inside = contains_point(Vec2<T>{i, y_});
            if(not is_inside) continue;
            if(const auto res = target.draw_x_unchecked(i, color);
                res.is_err()) return Err(res.unwrap_err());
            continue;
        }

        return Ok();
    }

    template<DrawTargetConcept Target, typename DestColor>
    Result<void, typename Target::Error> draw_hollow(Target& target, const DestColor & dest_color) {
        return Ok();
    }


private:
    // Iterator iter_;
    uint16_t y_;
    uint16_t y_stop_;
    Range2<uint16_t> x_range_;
    Vec2<uint16_t> center_;
    uint32_t squ_inner_radius_;
    uint32_t squ_outer_radius_;
    Vec2<iq16> start_norm_vec_;
    Vec2<iq16> stop_norm_vec_;
    bool is_minor_;

    using ST = std::make_signed_t<T>;

    __fast_inline constexpr bool contains_point(const Vec2<uint32_t> p){
        const Vec2<int32_t> offset = (Vec2<int32_t>(p) - Vec2<int32_t>(center_)).flip_y();
        const uint32_t len_squ = static_cast<uint32_t>(offset.length_squared());

        //判断半径
        // if((len_squ & 0xff) > 0x7f) return false;
        // if((len_squ & 0x0f) > 7) return false;
        // if((len_squ & 0x020)) return false;
        // if((len_squ & 0x020)) return false;

        #if 1
        if (len_squ < squ_inner_radius_) return false;
        if (len_squ > squ_outer_radius_) return false;
        #else
        int8_t pass_cnt = 0;
        if (len_squ < squ_inner_radius_) pass_cnt -= 1;
        if (len_squ > squ_outer_radius_) pass_cnt -= 1;

        // if (len_squ < squ_inner_radius_/4) pass_cnt -= 1;
        // if (len_squ > squ_outer_radius_/4) pass_cnt -= 1;

        if(pass_cnt <= -1) return false;
        #endif

        #if 0
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec_);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec_);

        return is_minor_ ? (b1 && b2) : (b1 || b2);

        #else
        // if(Vec2<iq16>(offset).angle()
        //     .mod(Angle<iq16>::from_degrees(10)) > 
        //     Angle<iq16>::from_degrees(2)) return false;
        const auto b1 = offset.is_counter_clockwise_to(start_norm_vec_);
        const auto b2 = offset.is_clockwise_to(stop_norm_vec_);

        return is_minor_ ? (b1 && b2) : (b1 || b2);

        #endif


        return true;
    }

    constexpr DrawDispatchIterator(const Shape & shape, const CtorHelper & helper):
        y_(helper.y_range.start),
        y_stop_(helper.y_range.stop),
        x_range_(helper.x_range),
        center_(shape.center),
        squ_inner_radius_(static_cast<uint32_t>(
            square(static_cast<iq12>(shape.radius_range.start)))),
        squ_outer_radius_(static_cast<uint32_t>(
            square(static_cast<iq12>(shape.radius_range.stop)))),
        start_norm_vec_(helper.v1),
        stop_norm_vec_(helper.v2),
        is_minor_(helper.v2.is_counter_clockwise_to(helper.v1))
        {;}
};


template<typename T, typename D>
struct CacheOf<AnnularSector<T, D>, bool>{
    using Self = CacheOf<AnnularSector<T, D>, bool>;

    T squ_inner_radius;
    T squ_outer_radius;
    Vec2<T> start_norm_vec;
    Vec2<T> stop_norm_vec;
    bool is_minor;

    static constexpr Self from(const AnnularSector<T, D> & obj){
        const auto n1 = Vec2<D>::from_angle(obj.angle_range.start);
        const auto n2 = Vec2<D>::from_angle(obj.angle_range.stop());
        return Self{
            .squ_inner_radius = square(obj.radius_range.start),
            .squ_outer_radius = square(obj.radius_range.stop),
            .start_norm_vec = n1,
            .stop_norm_vec = n2,
            .is_minor = n2.is_counter_clockwise_to(n1)
        };

    } 
    __fast_inline constexpr uint8_t color_from_point(const Vec2<T> offset) const {
        return s_color_from_point(*this, offset);
    }
private:
    static constexpr uint8_t s_color_from_point(const Self & self, const Vec2<T> offset){
        const auto len_squ = offset.length_squared();

        if (len_squ < self.squ_inner_radius || 
            len_squ > self.squ_outer_radius) {
            return false;
        }

        return (self.is_minor 
            ? (offset.is_counter_clockwise_to(self.start_norm_vec) && 
            offset.is_clockwise_to(self.stop_norm_vec))
            : (offset.is_counter_clockwise_to(self.start_norm_vec) || 
            offset.is_clockwise_to(self.stop_norm_vec))) ? 0xff : 0;
    } 
};


template<typename T, typename D>
struct BoundingBoxOf<AnnularSector<T, D>> {
    static constexpr Rect2<T> bounding_box(const AnnularSector<T, D> & obj){
        const auto n1 = Vec2<D>::from_angle(obj.angle_range.start);
        const auto n2 = Vec2<D>::from_angle(obj.angle_range.stop());
        const bool is_minor = n2.is_counter_clockwise_to(n1);
    
        Rect2<T> bb = Rect2<T>::from_minimal_bounding_box({
            n1 * obj.radius_range.start,
            n1 * obj.radius_range.stop,
            n2 * obj.radius_range.start,
            n2 * obj.radius_range.stop
        });

        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {1,0}, n1, n2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {0,1}, n1, n2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {-1,0}, n1, n2, is_minor);
        BoundingBoxMergeHelper::merge_if_contains_angle(bb, {0,-1}, n1, n2, is_minor);

        return bb;
    }
private:
    static constexpr bool contains_angle(
        const Vec2<T> offset,
        const Vec2<T> start_norm_vec,
        const Vec2<T> stop_norm_vec,
        const bool is_minor
    ){
        return AnnularSector<T, D>::contains_angle_helper(
            offset,
            start_norm_vec,
            stop_norm_vec,
            is_minor
        );
    };

    struct BoundingBoxMergeHelper{

        static constexpr void merge_if_contains_angle(
            Rect2<T> & box,
            const Vec2<T> offset,
            const Vec2<T> start_norm_vec,
            const Vec2<T> stop_norm_vec,
            const bool is_minor
        ){
            if(contains_angle(offset, start_norm_vec, stop_norm_vec, is_minor)){
                box = box.merge(offset);
            }
        }
    };

};

}
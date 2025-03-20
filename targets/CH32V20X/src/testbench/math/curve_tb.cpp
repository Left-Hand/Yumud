#include "../tb.h"


#include "core/utils/setget/Getter.hpp"
#include "core/utils/setget/Setter.hpp"
#include "core/debug/debug.hpp"
#include "core/polymorphism/proxy.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "algo/interpolation/Polynomial.hpp"
#include "algo/interpolation/Linear.hpp"
#include "types/vector2/vector2.hpp"
#include "types/vector3/vector3.hpp"

#include "robots/curve/CurveConcept_t.hpp"
#include "robots/curve/CurveTrapezoid_t.hpp"

#include "robots/tween/Tween.hpp"



using Point = Vector2_t<real_t>;
using Points = std::vector<Vector2_t<real_t>>;

using namespace ymd::intp;
using namespace ymd::utils;
using namespace ymd::tween;
using namespace ymd::curve;
// void sort_po



auto compare_points_by_x = [](const Vector2_t<real_t> & a, const Vector2_t<real_t> & b) -> bool {
    return a.x < b.x;
};

auto get_iter_by_x = [](const Points & points, const real_t x) -> Points::const_iterator {
    // 使用 std::lower_bound 找到第一个 x 坐标大于等于给定 x 的点
    return std::lower_bound(points.begin(), points.end(), x, [](const Vector2_t<real_t> & p, const real_t val) {
        return p.x < val;
    });
};

auto get_y_by_x = [](const Points & points, const real_t x){
    // 查找第一个 x >= x 的点
    auto it = std::lower_bound(points.begin(), points.end(), x, [](const Vector2_t<real_t> & p, const real_t val) {
        return p.x < val;
    });

    // 处理边界情况
    if (it == points.begin()) {
        // x 小于所有点的 x 坐标
        return points.front().y;
    } else if (it == points.end()) {
        // x 大于所有点的 x 坐标
        return points.back().y;
    }

    // 插值计算
    const auto prev_it = std::prev(it);
    const auto delta_p = x - prev_it->x;
    const auto delta_x = it->x - prev_it->x;
    const auto ratio = delta_p / delta_x;

    return LERP(prev_it->y, it->y, ratio);
};

auto rasterization_points(const Points & points, const size_t n){
    std::vector<real_t> ret;

    for(size_t i = 0; i < n; i++){
        const auto x = real_t(i) / n;
        ret.emplace_back(get_y_by_x(points, x));
    }

    return ret;
}



template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

// template<typename T, Arithmetic U>
// concept Lerpable = requires(T a, T b, U t) {
//     { a + b } -> std::same_as<T>;
//     { a - b } -> std::same_as<T>;
//     { a * t } -> std::same_as<T>;
// };

auto rasterization_points(const auto & functor, const size_t n){
    std::vector<real_t> ret;

    for(size_t i = 0; i < n; i++){
        const auto x = real_t(i) / n;
        ret.emplace_back(functor(x));
    }

    return ret;
}


void curve_tb() {
    DEBUGGER_INST.init(DEBUG_UART_BAUD);
    DEBUG_PRINTLN(std::setprecision(4));

    using Vector3 = Vector3_t<real_t>;

    Points points = {
        {0,0},
        {0.2_r,0.8_r},
        {0.9_r,0.3_r},
        {0.4_r,0.5_r},
        {1,1},
    };

    std::sort(points.begin(), points.end(), Vector2::sort_by_x);


    class Ball{
    public:
        void setSize(const real_t & size){
            DEBUG_PRINTLN(size);
        }

        void setPosition(const Vector2 & pos){
            auto [x,y] = pos;
            DEBUG_PRINTLN(x,y, sin(time()));
        }

        void setScale(const Vector3 & scale){
            auto [x,y,z] = scale;
            DEBUG_PRINTLN(x,y,z);
        }

        Vector2 getPosition(){
            // DEBUG_PRINTLN
            return Vector2(1,0).rotated(time());
        }

        operator real_t(){
            // DEBUG_PRINTLN("??")
            return time();
        }
    };

    Ball ball;


    auto pos_setter = make_setter(ball, &Ball::setPosition);
    auto curve = make_curve<Vector2>({0,0}, {1,4}, 1, CosineInterpolation());
    auto curve2 = make_curve(-2, 9, 2, CosineInterpolation());
    auto curve3 = make_curve<CurveTrapezoid_t, Vector2>({9,0}, {30, 8}, 20, 90);

    auto getter = make_getter(ball, &Ball::getPosition);
    auto getter2 = make_getter(ball, &Ball::operator real_t);


    // [[maybe_unused]] auto tweener = make_tweener(
    //     ball, &Ball::setPosition,
    //     1, {1,0}, {-0.3_r,4}, CosineInterpolation()
    // );

    // [[maybe_unused]] Tweener_t<Vector2> * tw2 = new_tweener<Vector2>(
    //     pos_setter,curve3
    // );
    // [[maybe_unused]] auto tweener = make_tweener(
    //     ball, &Ball::setPosition,
    //     1, {40,0}, {-0.3_r,4}, CosineInterpolation()
    // );

    // TweenerProxy tp = &tweener;
    [[maybe_unused]] auto tp2 = make_twproxy(
        ball, &Ball::setPosition,
        1, {40,0}, {-0.3_r,4}, LinearInterpolation()
    );

    [[maybe_unused]] auto tp = make_twproxy(
        ball, &Ball::setScale,
        1, {40,0, 3}, {-0.3_r,4, 9}, CosineInterpolation()
    );


    // auto tweener4 = TweenerStatic_t<Vector2>(pos_setter, curve3);
    // sizeof(TweenerStatic_t<Vector2>::Curve &);
    // sizeof(tweener4._curve);
    // using setter_type = decltype(&tweener4._setter);
    // using curve_type = decltype(&tweener4._curve);
    // sizeof(setter_type);
    // sizeof(curve_type);
    // auto a = "??";
    // sizeof(a);
    // sizeof()
    // scexpr auto a = sizeof(std::remove_pointer_t<decltype(tweener)>);
    // scexpr auto a = sizeof(std::remove_pointer_t<decltype(tweener)>::CurveWrapper);
    // scexpr auto a = sizeof(std::remove_pointer_t<decltype(tweener)>::SetterWrapper);

    // [[maybe_unused]] auto tw3 = new_tweener<Vector2>(
    //     pos_setter,curve3
    // );

    // [[maybe_unused]] auto tw3 = make_tweener(
    //     ball, &Ball::setSize, 
    //     CosineInterpolation(), 0, 1
    // );


    // for(auto & p : points) {
    //     DEBUG_PRINTLN(p);
    //     delay(10);
    // }

    // DEBUG_PRINTLN(get_y_by_x(points, 0.1_r));
    // DEBUG_PRINTLN(get_y_by_x(points, -0.1_r));
    // DEBUG_PRINTLN(get_y_by_x(points, 0.95_r));
    // DEBUG_PRINTLN(get_y_by_x(points, 1.04_r));

    // auto rpoints = rasterization_points(points, 50);


    // auto rpoints = rasterization_points(intp::CosineInterpolation(), 50);
    // auto rpoints = rasterization_points(intp::SeatInterpolation({0.6_r, 0.2_r}), 50);
    // auto rpoints = rasterization_points(intp::QuadraticSeatInterpolation({0.6_r, 0.2_r}, 10), 50);
    // auto rpoints = rasterization_points(intp::SymmetricInterpolation({0.6_r, 0.2_r}, 10), 50);
    // [[maybe_unused]] auto tw1 = make_tweener(0, 4, intp::CosineInterpolation());
    // [[maybe_unused]] auto tw2 = make_tweener(Vector2(0,0), Vector2(4,4), intp::CosineInterpolation());


    // auto rpoints = rasterization_points(tw1, 50);
    // auto rpoints = rasterization_points(make_tweener(Vector2(0,0), Vector2(1,4), intp::CosineInterpolation()), 50);

    // for(auto & p : rpoints) {
    //     DEBUG_PRINTLN(p);
    //     delay(1);
    // }
    delay(10);
    
    while(true){
        // DEBUG_PRINTLN(getter());
        // tweener.update(frac(t));
        // setter = Vector2(1,0).rotated(t);

        // DEBUG_PRINTLN(getter(), getter2());
        // tw3.update(frac(time()));
        // pos_setter({sin(time()), cos(time())});

        // tw2->update(fmod(t, tw2->period()));
        // tweener.update(fmod(t, tweener.period()));
        // DEBUG_PRINTLN(size_t(&Ball::setPosition));
        tp->update(fmod(time(), tp->period()));
        tp2->update(fmod(time(), tp2->period()));
        // DEBUG_PRINTLN(sizeof(decltype(tweener4)));P
        // DEBUG_PRINTLN( sizeof(std::remove_pointer_t<decltype(tweener)>));
        // DEBUG_PRINTLN( sizeof(std::remove_pointer_t<decltype(tweener4)>));
        // DEBUG_PRINTLN( sizeof(std::remove_pointer_t<decltype(pos_setter)>));
        // DEBUG_PRINTLN( sizeof(size_t));
        // DEBUG_PRINTLN( sizeof(int*));
        // tw3.update()
        // auto [x,y] = curve3(fmod(t, curve3.period()));
        // DEBUG_PRINTLN(x,y);
        // static int i = 0;
        // DEBUG_PRINTLN(real_t(i++) / 10 + real_t(0.0001))
        // DEBUG_PRINTLN(2 * sin(time()))
        // auto pos = getter();
        // setter = Vector2(getter);
        // setter = getter;
        // setter = Vector2(getter);
        // setter(getter());
        delay(10);
    }

    // while(true){
    //     DEBUG_PRINTLN(millis());
    //     delay(10);
    // }
}
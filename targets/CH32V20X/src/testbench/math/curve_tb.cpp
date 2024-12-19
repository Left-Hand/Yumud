#include "../tb.h"


#include "hal/bus/uart/uarthw.hpp"
#include "algo/interpolation/Polynomial.hpp"
#include "sys/debug/debug_inc.h"
#include "types/vector2/vector2_t.hpp"

using Point = Vector2_t<real_t>;
using Points = std::vector<Vector2_t<real_t>>;

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
concept Functor = requires(T f, real_t x) {
    { f(x) } -> std::same_as<real_t>;
};

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

// template<typename T, Arithmetic U>
// concept Lerpable = requires(T a, T b, U t) {
//     { a + b } -> std::same_as<T>;
//     { a - b } -> std::same_as<T>;
//     { a * t } -> std::same_as<T>;
// };

auto rasterization_points(const Functor auto & functor, const size_t n){
    std::vector<real_t> ret;

    for(size_t i = 0; i < n; i++){
        const auto x = real_t(i) / n;
        ret.emplace_back(functor(x));
    }

    return ret;
}

// template<Lerpable T, Functor U>
template<typename T, typename U>
class Tweener_t{
    const T from_;
    const T delta_;

    using Interpolation = intp::Interpolation;
    const U _func; 
public:
    Tweener_t(const T & from,const T & to, const U & func) : from_(from), delta_(to - from), _func(std::move(func)) {}

    T operator()(const real_t x) const {
        return from_ + delta_ * real_t(_func(x));
    }
};

using namespace ymd::intp;
using Tween_CosVec2 = Tweener_t<Vector2_t<real_t>, CosineInterpolation>;

using Tween_Cos = Tweener_t<real_t, CosineInterpolation>;

template<arithmetic T, typename U>
auto make_tweener(const T & from, const T & to,const U & inst){
    return Tweener_t<real_t, U>(from, to, inst);
}

template<typename T, typename U>
auto make_tweener(const T & from, const T & to,const U & inst){
    return Tweener_t<T, U>(from, to, inst);
}

void curve_tb() {
    DEBUGGER_INST.init(DEBUG_UART_BAUD);
    DEBUG_PRINTLN(std::setprecision(4));


    Points points = {
        {0,0},
        {0.2_r,0.8_r},
        {0.9_r,0.3_r},
        {0.4_r,0.5_r},
        {1,1},
    };

    std::sort(points.begin(), points.end(), compare_points_by_x);

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
    [[maybe_unused]] auto tw1 = make_tweener(0, 4, intp::CosineInterpolation());
    [[maybe_unused]] auto tw2 = make_tweener(Vector2(0,0), Vector2(4,4), intp::CosineInterpolation());

    auto rpoints = rasterization_points(tw1, 50);
    // auto rpoints = rasterization_points(make_tweener(Vector2(0,0), Vector2(1,4), intp::CosineInterpolation()), 50);

    for(auto & p : rpoints) {
        DEBUG_PRINTLN(p);
        delay(1);
    }
    delay(10);
    
    while(true);

    // while(true){
    //     DEBUG_PRINTLN(millis());
    //     delay(10);
    // }
}
#include "../tb.h"


#include "hal/bus/uart/uarthw.hpp"
#include "algo/interpolation/Polynomial.hpp"
#include "sys/debug/debug_inc.h"
#include "types/vector2/vector2_t.hpp"

using Point = Vector2_t<real_t>;
using Points = std::vector<Vector2_t<real_t>>;

using namespace ymd::intp;
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


template<typename T>
class CurveConcept_t{
public:
    CurveConcept_t(const CurveConcept_t & other) = delete;
    CurveConcept_t(CurveConcept_t && other) = default;

    CurveConcept_t() = default;
    virtual ~CurveConcept_t() = default;

    virtual T operator()(const real_t x) const = 0;
};

template<typename T, Functor U>
class CurveFunctor_t:public CurveConcept_t<T>{
    const U _functor; 
    const T from_;
    const T delta_;
    const real_t dur_;
public:
    CurveFunctor_t(U && functor, const T & from,const T & to, const real_t dur = 2) :
        _functor(std::move(functor)),
        from_(from), 
        delta_(to - from),
        dur_(dur)
        {}

    T operator()(const real_t x) const override{
        return from_ + delta_ * real_t(_functor(x));
    }
};

template<arithmetic T, typename U>
auto make_curve(U && functor, const T from, const T to) {
    return CurveFunctor_t<real_t, U>(std::move(functor), from, to);
}

template<typename T, typename U>
auto make_curve(U && functor, const T from, const T to) {
    return CurveFunctor_t<T, U>(std::move(functor), from, to);
}

template<typename T>
class SetterConcept_t{
public:
    SetterConcept_t(const SetterConcept_t & other) = delete;
    SetterConcept_t(SetterConcept_t && other) = default;

    SetterConcept_t() = default;

    virtual ~SetterConcept_t() = default;

    virtual SetterConcept_t & operator =(const T & value) = 0; 
};

template<typename T>
class LambdaSetter_t: public SetterConcept_t<T>{
public:
    using Setter = std::function<void(T)>;
protected:
    Setter _setter;
public:
    template<typename F>
    LambdaSetter_t(F && setter)
        : _setter(std::forward<F>(setter)) {}

    LambdaSetter_t & operator =(const T & value){
        _setter(value);
        return *this;
    }
};

template<typename ObjType, typename MemberFuncPtr>
auto make_setter(ObjType & obj, MemberFuncPtr member_func_ptr) {
    return LambdaSetter_t<Vector2>(
        [&obj, member_func_ptr](const Vector2 & value) {
            (obj.*member_func_ptr)(value);
        });
}

template<typename Setter, typename Curve>
class Tweener_t{
    Setter _setter;
    Curve _curve; 
public:
    Tweener_t(Setter && setter, Curve && curve):
        _setter(std::move(setter)),
        _curve(std::move(curve)){}

    void update(const real_t time){
        _setter = _curve(time);
    }
};


template<typename T, typename U>
auto make_tweener(T && setter, U && curve){
    return Tweener_t<T, U>(std::move(setter), std::move(curve));
}

template<typename ObjType, typename ValueType, typename Interpolator>
auto make_tweener(
    ObjType & obj, 
    void(ObjType::*member_func_ptr)(const ValueType &),
    const Interpolator & interpolator, 
    const ValueType & from, 
    const ValueType & to)
{

    auto setter = make_setter(obj, member_func_ptr);
    auto curve = make_curve(interpolator, from, to);   
    return Tweener_t<decltype(setter), decltype(curve)>(std::move(setter), std::move(curve));
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

    class Ball{
    public:
        void setPosition(const Vector2 & pos){
            DEBUG_PRINTLN("ball moved to", pos);
        }
    };

    Ball ball;

    auto tweener = make_tweener(
        ball, &Ball::setPosition, 
        CosineInterpolation(), {0,0}, {1,1}
    );

    std::sort(points.begin(), points.end(), compare_points_by_x);

    auto tw1 = make_tweener(
        make_setter(ball, &Ball::setPosition), 
        make_curve(CosineInterpolation(), Vector2{0,0}, Vector2{1,1})
    );

    auto tw2 = make_tweener(
        ball, &Ball::setPosition, 
        CosineInterpolation(), Vector2{0,0}, Vector2{1,1}
    );
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
    
    while(true);

    // while(true){
    //     DEBUG_PRINTLN(millis());
    //     delay(10);
    // }
}
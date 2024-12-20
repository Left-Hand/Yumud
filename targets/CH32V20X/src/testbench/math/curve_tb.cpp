#include "../tb.h"


#include "hal/bus/uart/uarthw.hpp"
#include "algo/interpolation/Polynomial.hpp"
#include "sys/debug/debug_inc.h"
#include "types/vector2/vector2_t.hpp"
#include "types/vector3/vector3_t.hpp"
#include "robots/curve/CurveConcept_t.hpp"

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

template<typename ValueType, typename ObjType>
auto make_setter(ObjType & obj, void(ObjType::*member_func_ptr)(const ValueType &)) {
    return LambdaSetter_t<ValueType>(
        [&obj, member_func_ptr](const ValueType & value) {
            (obj.*member_func_ptr)(value);
        });
}


template<typename T>
class GetterConcept_t{
public:
    GetterConcept_t(const GetterConcept_t & other) = delete;
    GetterConcept_t(GetterConcept_t && other) = default;

    GetterConcept_t() = default;

    virtual ~GetterConcept_t() = default;

    virtual T operator()() = 0; 
};


template<typename T>
class LambdaGetter_t: public GetterConcept_t<T>{
public:
    using Getter = std::function<T(void)>;
protected:
    Getter _getter;
public:
    template<typename F>
    LambdaGetter_t(F && getter)
        : _getter(std::forward<F>(getter)) {}

    T operator ()() override {
        return _getter();
    }
};

template<typename ValueType, typename ObjType>
auto make_getter(ObjType & obj, ValueType(ObjType::*member_func_ptr)()) {
    return LambdaGetter_t<ValueType>(
        [&obj, member_func_ptr]() {
            return (obj.*member_func_ptr)();
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


template<typename ValueType, typename Interpolator, typename U = ValueType>
auto make_tweener(
    auto & obj, 
    void(std::remove_reference_t<decltype(obj)>::*member_func_ptr)(const ValueType &),
    const Interpolator & interpolator, 
    const U & from, 
    const U & to)
{

    auto setter = make_setter<ValueType>(obj, member_func_ptr);

    using CurveType = std::conditional_t<std::is_arithmetic_v<ValueType>, real_t, ValueType>;
    auto curve = make_curve<CurveType>(interpolator, from, to);

    return Tweener_t<decltype(setter), decltype(curve)>(std::move(setter), std::move(curve));
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

    std::sort(points.begin(), points.end(), compare_points_by_x);


    class Ball{
    public:
        void setSize(const real_t & size){
            DEBUG_PRINTLN("size", size);
        }

        void setPosition(const Vector2 & pos){
            DEBUG_PRINTLN("ball moved to", pos);
        }

        void setScale(const Vector3 & scale){
            DEBUG_PRINTLN("ball scale is", scale);
        }

        Vector2 getPosition(){
            // DEBUG_PRINTLN
            return Vector2(1,0).rotated(t);
        }
    };

    Ball ball;

    auto getter = make_getter(ball, &Ball::getPosition);

    auto tweener = make_tweener(
        ball, &Ball::setPosition, 
        CosineInterpolation(), {0,0}, {1,1}
    );

    auto tw2 = make_tweener(
        ball, &Ball::setScale, 
        CosineInterpolation(), {0,0,0}, {1,1,1}
    );

    auto tw3 = make_tweener(
        ball, &Ball::setSize, 
        CosineInterpolation(), 0, 1
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
    
    while(true){
        DEBUG_PRINTLN(getter());
    }

    // while(true){
    //     DEBUG_PRINTLN(millis());
    //     delay(10);
    // }
}
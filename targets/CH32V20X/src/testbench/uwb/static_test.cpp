#include "geometry.hpp"

using namespace ymd;
using namespace ymd::geometry;

// 测试用例1: 不相交的圆（距离太远）
constexpr auto test_no_intersection = []->bool  {
    math::Circle2<double> circle1{math::Vec2<double>(0, 0), 1.0};
    math::Circle2<double> circle2{math::Vec2<double>(5, 0), 1.0};
    auto result = compute_intersection_points(circle1, circle2);
    return result.size() == 0; // 应该没有交点
};

// 测试用例2: 完全包含的圆
constexpr auto test_contained = []->bool  {
    math::Circle2<double> circle1{math::Vec2<double>(0, 0), 5.0};
    math::Circle2<double> circle2{math::Vec2<double>(1, 0), 1.0};
    auto result = compute_intersection_points(circle1, circle2);
    return result.size() == 0; // 小圆完全在大圆内，没有交点
};

// 测试用例3: 相切的圆
constexpr auto test_tangent = []->bool  {
    math::Circle2<double> circle1{math::Vec2<double>(0, 0), 2.0};
    math::Circle2<double> circle2{math::Vec2<double>(4, 0), 2.0};
    auto result = compute_intersection_points(circle1, circle2);
    return result.size() == 1; // 应该有一个切点
};

// 精确的相交测试（使用整数避免浮点误差）
constexpr auto test_exact_intersection_int = []->bool  {
    constexpr math::Circle2<float> circle1{math::Vec2<float>(0, 0), 5};  // 圆心在原点，半径5
    constexpr math::Circle2<float> circle2{math::Vec2<float>(6, 0), 5};  // 圆心在(6,0)，半径5
    
    constexpr auto result = compute_intersection_points(circle1, circle2);
    
    // 两个圆应该在(3,4)和(3,-4)相交
    if (result.size() != 2) return false;
    
    // 检查第一个交点 (3,4)
    // if ( || result[0].y != 4) return false;
    static_assert(math::is_equal_approx(result[0].x, 3.0f, 0.00001f), "浮点数测试失败");
    static_assert(math::is_equal_approx(result[0].y, 4.0f, 0.00001f), "浮点数测试失败");
    
    // 检查第二个交点 (3,-4)  
    // if (result[1].x != 3 || result[1].y != -4) return false;
    static_assert(math::is_equal_approx(result[1].x, 3.0f, 0.00001f), "浮点数测试失败");
    static_assert(math::is_equal_approx(result[1].y, -4.0f, 0.00001f), "浮点数测试失败");
    
    return true;
};

// 测试用例4: 负半径检查
constexpr auto test_negative_radius = []->bool  {
    math::Circle2<double> circle1{math::Vec2<double>(0, 0), -1.0}; // 负半径
    math::Circle2<double> circle2{math::Vec2<double>(1, 0), 1.0};
    auto result = compute_intersection_points(circle1, circle2);
    return result.size() == 0; // 应该返回空
};


// 测试编译时计算能力
constexpr auto compile_time_test = []{
    math::Circle2<double> circle1{math::Vec2<double>(0, 0), 3.0};
    math::Circle2<double> circle2{math::Vec2<double>(4, 0), 2.0};
    return compute_intersection_points(circle1, circle2);
};


// 编译时静态测试
static_assert(test_no_intersection(), "不相交测试失败");
static_assert(test_contained(), "包含测试失败");
static_assert(test_negative_radius(), "负半径测试失败");
static_assert(test_exact_intersection_int(), "精确相交测试失败");
static_assert(compile_time_test().size() == 2, "编译时计算测试失败");

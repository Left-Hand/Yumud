#include "sphere_fit.hpp"

using namespace ymd;
using namespace ymd::robots;

namespace{


[[maybe_unused]] 
void test_sphere(){
    {
        using T = float;

        static constexpr math::Vec3<T> points[] = {
            {1.0, 1.0, 1.0},
            {2.0, 2.0, 2.0},
            {3.0, 3.0, 3.0}
        };

        [[maybe_unused]] static constexpr auto result = [&]{
            auto inter = LeastSquaresSphere<T>::Intermediate::zero();
            inter.push_points(points);
            return inter.solve(100, 0.1);
        }();
    }

}
}
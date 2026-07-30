#include "../abdq.hpp"
#include "middlewares/algebra/gesture/rotation2.hpp"

using namespace ymd;
using namespace ymd::digipw;






namespace{


[[maybe_unused]] static void test_spiniq31(){
    
    {
        constexpr auto ROTATION = math::Rotation2<iq31>::from_angle(Angular<uq32>::from_degrees(30));
        static_assert(std::abs((float)ROTATION.sine() - 0.5f) < 1e-2f);
        static_assert(std::abs((float)ROTATION.cosine() - 0.866f) < 1e-2f);

    }

}

}


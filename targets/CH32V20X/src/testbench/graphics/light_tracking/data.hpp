#pragma once

#include "core/math/float/bf16.hpp"

using namespace ymd;

using namespace ymd::math;

static constexpr std::array<TriangleSurface<bf16>,32> triangles =
{
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.2400_iq16,	1.9800_iq16,	-0.2200_iq16},	math::Vec3<bf16>{	0.2300_iq16,	1.9800_iq16,	0.1600_iq16},	math::Vec3<bf16>{	-0.2400_iq16,	1.9800_iq16,	0.1600_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.2400_iq16,	1.9800_iq16,	-0.2200_iq16},	math::Vec3<bf16>{	0.2300_iq16,	1.9800_iq16,	-0.2200_iq16},	math::Vec3<bf16>{	0.2300_iq16,	1.9800_iq16,	0.1600_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000_iq16,	0.0000_iq16,	0.9900_iq16},	math::Vec3<bf16>{	-0.9900_iq16,	0.0000_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	-1.0100_iq16,	0.0000_iq16,	0.9900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0200_iq16,	1.9900_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	0.9900_iq16},	math::Vec3<bf16>{	-1.0200_iq16,	1.9900_iq16,	0.9900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.9900_iq16,	0.0000_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	-1.0200_iq16,	1.9900_iq16,	-1.0400_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000_iq16,	0.0000_iq16,	0.9900_iq16},	math::Vec3<bf16>{	1.0000_iq16,	0.0000_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	-0.9900_iq16,	0.0000_iq16,	-1.0400_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0200_iq16,	1.9900_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	0.9900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.9900_iq16,	0.0000_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	1.0000_iq16,	0.0000_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	-1.0400_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0100_iq16,	0.0000_iq16,	0.9900_iq16},	math::Vec3<bf16>{	-1.0200_iq16,	1.9900_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	-1.0200_iq16,	1.9900_iq16,	0.9900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0100_iq16,	0.0000_iq16,	0.9900_iq16},	math::Vec3<bf16>{	-0.9900_iq16,	0.0000_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	-1.0200_iq16,	1.9900_iq16,	-1.0400_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000_iq16,	0.0000_iq16,	0.9900_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	-1.0400_iq16},	math::Vec3<bf16>{	1.0000_iq16,	0.0000_iq16,	-1.0400_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000_iq16,	0.0000_iq16,	0.9900_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	0.9900_iq16},	math::Vec3<bf16>{	1.0000_iq16,	1.9900_iq16,	-1.0400_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400_iq16,	1.2000_iq16,	-0.0900_iq16},	math::Vec3<bf16>{	-0.7100_iq16,	1.2000_iq16,	-0.4900_iq16},	math::Vec3<bf16>{	-0.5300_iq16,	1.2000_iq16,	0.0900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.5300_iq16,	1.2000_iq16,	0.0900_iq16},	math::Vec3<bf16>{	-0.7100_iq16,	0.0000_iq16,	-0.4900_iq16},	math::Vec3<bf16>{	-0.5300_iq16,	0.0000_iq16,	0.0900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.7100_iq16,	1.2000_iq16,	-0.4900_iq16},	math::Vec3<bf16>{	-0.1400_iq16,	0.0000_iq16,	-0.6700_iq16},	math::Vec3<bf16>{	-0.7100_iq16,	0.0000_iq16,	-0.4900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.1400_iq16,	1.2000_iq16,	-0.6700_iq16},	math::Vec3<bf16>{	0.0400_iq16,	0.0000_iq16,	-0.0900_iq16},	math::Vec3<bf16>{	-0.1400_iq16,	0.0000_iq16,	-0.6700_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400_iq16,	1.2000_iq16,	-0.0900_iq16},	math::Vec3<bf16>{	-0.5300_iq16,	0.0000_iq16,	0.0900_iq16},	math::Vec3<bf16>{	0.0400_iq16,	0.0000_iq16,	-0.0900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400_iq16,	1.2000_iq16,	-0.0900_iq16},	math::Vec3<bf16>{	-0.1400_iq16,	1.2000_iq16,	-0.6700_iq16},	math::Vec3<bf16>{	-0.7100_iq16,	1.2000_iq16,	-0.4900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.5300_iq16,	1.2000_iq16,	0.0900_iq16},	math::Vec3<bf16>{	-0.7100_iq16,	1.2000_iq16,	-0.4900_iq16},	math::Vec3<bf16>{	-0.7100_iq16,	0.0000_iq16,	-0.4900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.7100_iq16,	1.2000_iq16,	-0.4900_iq16},	math::Vec3<bf16>{	-0.1400_iq16,	1.2000_iq16,	-0.6700_iq16},	math::Vec3<bf16>{	-0.1400_iq16,	0.0000_iq16,	-0.6700_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.1400_iq16,	1.2000_iq16,	-0.6700_iq16},	math::Vec3<bf16>{	0.0400_iq16,	1.2000_iq16,	-0.0900_iq16},	math::Vec3<bf16>{	0.0400_iq16,	0.0000_iq16,	-0.0900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400_iq16,	1.2000_iq16,	-0.0900_iq16},	math::Vec3<bf16>{	-0.5300_iq16,	1.2000_iq16,	0.0900_iq16},	math::Vec3<bf16>{	-0.5300_iq16,	0.0000_iq16,	0.0900_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300_iq16,	0.6000_iq16,	0.7500_iq16},	math::Vec3<bf16>{	0.1300_iq16,	0.6000_iq16,	0.0000_iq16},	math::Vec3<bf16>{	-0.0500_iq16,	0.6000_iq16,	0.5700_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.0500_iq16,	0.6000_iq16,	0.5700_iq16},	math::Vec3<bf16>{	0.1300_iq16,	0.0000_iq16,	0.0000_iq16},	math::Vec3<bf16>{	-0.0500_iq16,	0.0000_iq16,	0.5700_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300_iq16,	0.6000_iq16,	0.7500_iq16},	math::Vec3<bf16>{	-0.0500_iq16,	0.0000_iq16,	0.5700_iq16},	math::Vec3<bf16>{	0.5300_iq16,	0.0000_iq16,	0.7500_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.7000_iq16,	0.6000_iq16,	0.1700_iq16},	math::Vec3<bf16>{	0.5300_iq16,	0.0000_iq16,	0.7500_iq16},	math::Vec3<bf16>{	0.7000_iq16,	0.0000_iq16,	0.1700_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.1300_iq16,	0.6000_iq16,	0.0000_iq16},	math::Vec3<bf16>{	0.7000_iq16,	0.0000_iq16,	0.1700_iq16},	math::Vec3<bf16>{	0.1300_iq16,	0.0000_iq16,	0.0000_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300_iq16,	0.6000_iq16,	0.7500_iq16},	math::Vec3<bf16>{	0.7000_iq16,	0.6000_iq16,	0.1700_iq16},	math::Vec3<bf16>{	0.1300_iq16,	0.6000_iq16,	0.0000_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.0500_iq16,	0.6000_iq16,	0.5700_iq16},	math::Vec3<bf16>{	0.1300_iq16,	0.6000_iq16,	0.0000_iq16},	math::Vec3<bf16>{	0.1300_iq16,	0.0000_iq16,	0.0000_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300_iq16,	0.6000_iq16,	0.7500_iq16},	math::Vec3<bf16>{	-0.0500_iq16,	0.6000_iq16,	0.5700_iq16},	math::Vec3<bf16>{	-0.0500_iq16,	0.0000_iq16,	0.5700_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.7000_iq16,	0.6000_iq16,	0.1700_iq16},	math::Vec3<bf16>{	0.5300_iq16,	0.6000_iq16,	0.7500_iq16},	math::Vec3<bf16>{	0.5300_iq16,	0.0000_iq16,	0.7500_iq16}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.1300_iq16,	0.6000_iq16,	0.0000_iq16},	math::Vec3<bf16>{	0.7000_iq16,	0.6000_iq16,	0.1700_iq16},	math::Vec3<bf16>{	0.7000_iq16,	0.0000_iq16,	0.1700_iq16}},
};
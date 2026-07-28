#pragma once

#include "core/math/float/bf16.hpp"

using namespace ymd;
using math::bf16;

static constexpr std::array<TriangleSurface<bf16>,32> triangles =
{
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.2400f,	1.9800f,	-0.2200f},	math::Vec3<bf16>{	0.2300f,	1.9800f,	0.1600f},	math::Vec3<bf16>{	-0.2400f,	1.9800f,	0.1600f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.2400f,	1.9800f,	-0.2200f},	math::Vec3<bf16>{	0.2300f,	1.9800f,	-0.2200f},	math::Vec3<bf16>{	0.2300f,	1.9800f,	0.1600f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000f,	0.0000f,	0.9900f},	math::Vec3<bf16>{	-0.9900f,	0.0000f,	-1.0400f},	math::Vec3<bf16>{	-1.0100f,	0.0000f,	0.9900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0200f,	1.9900f,	-1.0400f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	0.9900f},	math::Vec3<bf16>{	-1.0200f,	1.9900f,	0.9900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.9900f,	0.0000f,	-1.0400f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	-1.0400f},	math::Vec3<bf16>{	-1.0200f,	1.9900f,	-1.0400f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000f,	0.0000f,	0.9900f},	math::Vec3<bf16>{	1.0000f,	0.0000f,	-1.0400f},	math::Vec3<bf16>{	-0.9900f,	0.0000f,	-1.0400f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0200f,	1.9900f,	-1.0400f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	-1.0400f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	0.9900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.9900f,	0.0000f,	-1.0400f},	math::Vec3<bf16>{	1.0000f,	0.0000f,	-1.0400f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	-1.0400f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0100f,	0.0000f,	0.9900f},	math::Vec3<bf16>{	-1.0200f,	1.9900f,	-1.0400f},	math::Vec3<bf16>{	-1.0200f,	1.9900f,	0.9900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-1.0100f,	0.0000f,	0.9900f},	math::Vec3<bf16>{	-0.9900f,	0.0000f,	-1.0400f},	math::Vec3<bf16>{	-1.0200f,	1.9900f,	-1.0400f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000f,	0.0000f,	0.9900f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	-1.0400f},	math::Vec3<bf16>{	1.0000f,	0.0000f,	-1.0400f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	1.0000f,	0.0000f,	0.9900f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	0.9900f},	math::Vec3<bf16>{	1.0000f,	1.9900f,	-1.0400f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400f,	1.2000f,	-0.0900f},	math::Vec3<bf16>{	-0.7100f,	1.2000f,	-0.4900f},	math::Vec3<bf16>{	-0.5300f,	1.2000f,	0.0900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.5300f,	1.2000f,	0.0900f},	math::Vec3<bf16>{	-0.7100f,	0.0000f,	-0.4900f},	math::Vec3<bf16>{	-0.5300f,	0.0000f,	0.0900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.7100f,	1.2000f,	-0.4900f},	math::Vec3<bf16>{	-0.1400f,	0.0000f,	-0.6700f},	math::Vec3<bf16>{	-0.7100f,	0.0000f,	-0.4900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.1400f,	1.2000f,	-0.6700f},	math::Vec3<bf16>{	0.0400f,	0.0000f,	-0.0900f},	math::Vec3<bf16>{	-0.1400f,	0.0000f,	-0.6700f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400f,	1.2000f,	-0.0900f},	math::Vec3<bf16>{	-0.5300f,	0.0000f,	0.0900f},	math::Vec3<bf16>{	0.0400f,	0.0000f,	-0.0900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400f,	1.2000f,	-0.0900f},	math::Vec3<bf16>{	-0.1400f,	1.2000f,	-0.6700f},	math::Vec3<bf16>{	-0.7100f,	1.2000f,	-0.4900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.5300f,	1.2000f,	0.0900f},	math::Vec3<bf16>{	-0.7100f,	1.2000f,	-0.4900f},	math::Vec3<bf16>{	-0.7100f,	0.0000f,	-0.4900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.7100f,	1.2000f,	-0.4900f},	math::Vec3<bf16>{	-0.1400f,	1.2000f,	-0.6700f},	math::Vec3<bf16>{	-0.1400f,	0.0000f,	-0.6700f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.1400f,	1.2000f,	-0.6700f},	math::Vec3<bf16>{	0.0400f,	1.2000f,	-0.0900f},	math::Vec3<bf16>{	0.0400f,	0.0000f,	-0.0900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.0400f,	1.2000f,	-0.0900f},	math::Vec3<bf16>{	-0.5300f,	1.2000f,	0.0900f},	math::Vec3<bf16>{	-0.5300f,	0.0000f,	0.0900f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300f,	0.6000f,	0.7500f},	math::Vec3<bf16>{	0.1300f,	0.6000f,	0.0000f},	math::Vec3<bf16>{	-0.0500f,	0.6000f,	0.5700f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.0500f,	0.6000f,	0.5700f},	math::Vec3<bf16>{	0.1300f,	0.0000f,	0.0000f},	math::Vec3<bf16>{	-0.0500f,	0.0000f,	0.5700f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300f,	0.6000f,	0.7500f},	math::Vec3<bf16>{	-0.0500f,	0.0000f,	0.5700f},	math::Vec3<bf16>{	0.5300f,	0.0000f,	0.7500f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.7000f,	0.6000f,	0.1700f},	math::Vec3<bf16>{	0.5300f,	0.0000f,	0.7500f},	math::Vec3<bf16>{	0.7000f,	0.0000f,	0.1700f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.1300f,	0.6000f,	0.0000f},	math::Vec3<bf16>{	0.7000f,	0.0000f,	0.1700f},	math::Vec3<bf16>{	0.1300f,	0.0000f,	0.0000f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300f,	0.6000f,	0.7500f},	math::Vec3<bf16>{	0.7000f,	0.6000f,	0.1700f},	math::Vec3<bf16>{	0.1300f,	0.6000f,	0.0000f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	-0.0500f,	0.6000f,	0.5700f},	math::Vec3<bf16>{	0.1300f,	0.6000f,	0.0000f},	math::Vec3<bf16>{	0.1300f,	0.0000f,	0.0000f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.5300f,	0.6000f,	0.7500f},	math::Vec3<bf16>{	-0.0500f,	0.6000f,	0.5700f},	math::Vec3<bf16>{	-0.0500f,	0.0000f,	0.5700f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.7000f,	0.6000f,	0.1700f},	math::Vec3<bf16>{	0.5300f,	0.6000f,	0.7500f},	math::Vec3<bf16>{	0.5300f,	0.0000f,	0.7500f}},
    TriangleSurface<bf16>{math::Vec3<bf16>{	0.1300f,	0.6000f,	0.0000f},	math::Vec3<bf16>{	0.7000f,	0.6000f,	0.1700f},	math::Vec3<bf16>{	0.7000f,	0.0000f,	0.1700f}},
};
#pragma once

#include "core/math/float/bf16.hpp"

using namespace ymd;

static constexpr std::array<TriangleSurface_t<bf16>,32> triangles =
{
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.2400_r,	1.9800_r,	-0.2200_r},	Vec3<bf16>{	0.2300_r,	1.9800_r,	0.1600_r},	Vec3<bf16>{	-0.2400_r,	1.9800_r,	0.1600_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.2400_r,	1.9800_r,	-0.2200_r},	Vec3<bf16>{	0.2300_r,	1.9800_r,	-0.2200_r},	Vec3<bf16>{	0.2300_r,	1.9800_r,	0.1600_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	1.0000_r,	0.0000_r,	0.9900_r},	Vec3<bf16>{	-0.9900_r,	0.0000_r,	-1.0400_r},	Vec3<bf16>{	-1.0100_r,	0.0000_r,	0.9900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-1.0200_r,	1.9900_r,	-1.0400_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	0.9900_r},	Vec3<bf16>{	-1.0200_r,	1.9900_r,	0.9900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.9900_r,	0.0000_r,	-1.0400_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	-1.0400_r},	Vec3<bf16>{	-1.0200_r,	1.9900_r,	-1.0400_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	1.0000_r,	0.0000_r,	0.9900_r},	Vec3<bf16>{	1.0000_r,	0.0000_r,	-1.0400_r},	Vec3<bf16>{	-0.9900_r,	0.0000_r,	-1.0400_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-1.0200_r,	1.9900_r,	-1.0400_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	-1.0400_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	0.9900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.9900_r,	0.0000_r,	-1.0400_r},	Vec3<bf16>{	1.0000_r,	0.0000_r,	-1.0400_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	-1.0400_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-1.0100_r,	0.0000_r,	0.9900_r},	Vec3<bf16>{	-1.0200_r,	1.9900_r,	-1.0400_r},	Vec3<bf16>{	-1.0200_r,	1.9900_r,	0.9900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-1.0100_r,	0.0000_r,	0.9900_r},	Vec3<bf16>{	-0.9900_r,	0.0000_r,	-1.0400_r},	Vec3<bf16>{	-1.0200_r,	1.9900_r,	-1.0400_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	1.0000_r,	0.0000_r,	0.9900_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	-1.0400_r},	Vec3<bf16>{	1.0000_r,	0.0000_r,	-1.0400_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	1.0000_r,	0.0000_r,	0.9900_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	0.9900_r},	Vec3<bf16>{	1.0000_r,	1.9900_r,	-1.0400_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.0400_r,	1.2000_r,	-0.0900_r},	Vec3<bf16>{	-0.7100_r,	1.2000_r,	-0.4900_r},	Vec3<bf16>{	-0.5300_r,	1.2000_r,	0.0900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.5300_r,	1.2000_r,	0.0900_r},	Vec3<bf16>{	-0.7100_r,	0.0000_r,	-0.4900_r},	Vec3<bf16>{	-0.5300_r,	0.0000_r,	0.0900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.7100_r,	1.2000_r,	-0.4900_r},	Vec3<bf16>{	-0.1400_r,	0.0000_r,	-0.6700_r},	Vec3<bf16>{	-0.7100_r,	0.0000_r,	-0.4900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.1400_r,	1.2000_r,	-0.6700_r},	Vec3<bf16>{	0.0400_r,	0.0000_r,	-0.0900_r},	Vec3<bf16>{	-0.1400_r,	0.0000_r,	-0.6700_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.0400_r,	1.2000_r,	-0.0900_r},	Vec3<bf16>{	-0.5300_r,	0.0000_r,	0.0900_r},	Vec3<bf16>{	0.0400_r,	0.0000_r,	-0.0900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.0400_r,	1.2000_r,	-0.0900_r},	Vec3<bf16>{	-0.1400_r,	1.2000_r,	-0.6700_r},	Vec3<bf16>{	-0.7100_r,	1.2000_r,	-0.4900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.5300_r,	1.2000_r,	0.0900_r},	Vec3<bf16>{	-0.7100_r,	1.2000_r,	-0.4900_r},	Vec3<bf16>{	-0.7100_r,	0.0000_r,	-0.4900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.7100_r,	1.2000_r,	-0.4900_r},	Vec3<bf16>{	-0.1400_r,	1.2000_r,	-0.6700_r},	Vec3<bf16>{	-0.1400_r,	0.0000_r,	-0.6700_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.1400_r,	1.2000_r,	-0.6700_r},	Vec3<bf16>{	0.0400_r,	1.2000_r,	-0.0900_r},	Vec3<bf16>{	0.0400_r,	0.0000_r,	-0.0900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.0400_r,	1.2000_r,	-0.0900_r},	Vec3<bf16>{	-0.5300_r,	1.2000_r,	0.0900_r},	Vec3<bf16>{	-0.5300_r,	0.0000_r,	0.0900_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.5300_r,	0.6000_r,	0.7500_r},	Vec3<bf16>{	0.1300_r,	0.6000_r,	0.0000_r},	Vec3<bf16>{	-0.0500_r,	0.6000_r,	0.5700_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.0500_r,	0.6000_r,	0.5700_r},	Vec3<bf16>{	0.1300_r,	0.0000_r,	0.0000_r},	Vec3<bf16>{	-0.0500_r,	0.0000_r,	0.5700_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.5300_r,	0.6000_r,	0.7500_r},	Vec3<bf16>{	-0.0500_r,	0.0000_r,	0.5700_r},	Vec3<bf16>{	0.5300_r,	0.0000_r,	0.7500_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.7000_r,	0.6000_r,	0.1700_r},	Vec3<bf16>{	0.5300_r,	0.0000_r,	0.7500_r},	Vec3<bf16>{	0.7000_r,	0.0000_r,	0.1700_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.1300_r,	0.6000_r,	0.0000_r},	Vec3<bf16>{	0.7000_r,	0.0000_r,	0.1700_r},	Vec3<bf16>{	0.1300_r,	0.0000_r,	0.0000_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.5300_r,	0.6000_r,	0.7500_r},	Vec3<bf16>{	0.7000_r,	0.6000_r,	0.1700_r},	Vec3<bf16>{	0.1300_r,	0.6000_r,	0.0000_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	-0.0500_r,	0.6000_r,	0.5700_r},	Vec3<bf16>{	0.1300_r,	0.6000_r,	0.0000_r},	Vec3<bf16>{	0.1300_r,	0.0000_r,	0.0000_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.5300_r,	0.6000_r,	0.7500_r},	Vec3<bf16>{	-0.0500_r,	0.6000_r,	0.5700_r},	Vec3<bf16>{	-0.0500_r,	0.0000_r,	0.5700_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.7000_r,	0.6000_r,	0.1700_r},	Vec3<bf16>{	0.5300_r,	0.6000_r,	0.7500_r},	Vec3<bf16>{	0.5300_r,	0.0000_r,	0.7500_r}},
    TriangleSurface_t<bf16>{Vec3<bf16>{	0.1300_r,	0.6000_r,	0.0000_r},	Vec3<bf16>{	0.7000_r,	0.6000_r,	0.1700_r},	Vec3<bf16>{	0.7000_r,	0.0000_r,	0.1700_r}},
};
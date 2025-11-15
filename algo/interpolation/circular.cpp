#include "circular.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::intp;


#if 0
template<typename T>
static __fast_inline T square(const T x){
    return x * x;
}

bool ArcInterpolation::IsPerpendicular(const Vec2<iq16> & pt1,const Vec2<iq16> & pt2,const Vec2<iq16> & pt3) const {
    const auto [pt1x, pt1y] = pt1;
    const auto [pt2x, pt2y] = pt2;
    const auto [pt3x, pt3y] = pt3;

    const real_t yDelta_a = pt2y - pt1y;
    const real_t xDelta_a = pt2x - pt1x;
    const real_t yDelta_b = pt3y - pt2y;
    const real_t xDelta_b = pt3x - pt2x;
    static constexpr real_t epsilon = real_t(0.001);

    // checking whether the line of the two pts are vertical
    if (std::abs(xDelta_a) <= epsilon && std::abs(yDelta_b) <= epsilon){
        return false;
    }
    if (std::abs(yDelta_a) <= epsilon){
        return true;
    }
    else if (std::abs(yDelta_b) <= epsilon){
        return true;
    }
    else if (std::abs(xDelta_a)<= epsilon){
        return true;
    }
    else if (std::abs(xDelta_b)<= epsilon){
        return true;
    }
    else return false;
}

std::tuple<real_t, real_t, real_t> ArcInterpolation::calcCircleFrom3Points (const Vec2<iq16> & pt1,const Vec2<iq16> & pt2,const Vec2<iq16> & pt3) const {
    const auto [pt1x, pt1y] = pt1;
    const auto [pt2x, pt2y] = pt2;
    const auto [pt3x, pt3y] = pt3;

    const real_t yDelta_a = pt2y - pt1y;
    const real_t xDelta_a = pt2x - pt1x;
    const real_t yDelta_b = pt3y - pt2y;
    const real_t xDelta_b = pt3x - pt2x;
    static constexpr real_t epsilon = real_t(0.001);

    if (std::abs(xDelta_a) <= epsilon && std::abs(yDelta_b) <= epsilon){
        // m_Centerx = (pt2x + pt3x) / 2;
        // m_Centery = (pt1y + pt2y) / 2;
        // m_dRadius = std::sqrt(square(m_Centerx-pt1x) + square(m_Centery-pt1y));
        // return;
        return {
            (pt2x + pt3x) / 2,
            (pt1y + pt2y) / 2,
            std::sqrt(square(m_Centerx-pt1x) + square(m_Centery-pt1y))
        };
    }

    // IsPerpendicular() assure that xDelta(s) are not zero
    real_t aSlope = yDelta_a / xDelta_a; 
    real_t bSlope = yDelta_b / xDelta_b;

    // if (std::abs(aSlope-bSlope) <= epsilon){	
    //     // checking whether the given points are colinear. 	
    //     return;
    // }

    // calc center
    return {(
        aSlope*bSlope*(pt1y - pt3y) + 
        bSlope*(pt1x + pt2x) - 
        aSlope*(pt2x+pt3x) )
        /(2* (bSlope-aSlope) ),
        -1*(m_Centerx - (pt1x+pt2x)/2)/aSlope +  (pt1y+pt2y)/2,
        std::sqrt(square(m_Centerx-pt1x) + square(m_Centery-pt1y))
    };
}

ArcInterpolation::ArcInterpolation(const Vec2<iq16> & handle):
    _a(handle.x), _b(handle.y){
        static constexpr real_t pt1x = 0;
        static constexpr real_t pt1y = 0;
        real_t pt2x = _a;
        real_t pt2y = _b;
        static constexpr real_t pt3x = 1;
        static constexpr real_t pt3y = 1;
        auto && result = [&]() -> std::tuple<real_t, real_t, real_t> {
            if      (!IsPerpendicular(Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt3x,pt3y}))		
                calcCircleFrom3Points (Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt3x,pt3y});	
            else if (!IsPerpendicular(Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt2x,pt2y}))		
                calcCircleFrom3Points (Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt2x,pt2y});	
            else if (!IsPerpendicular(Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt3x,pt3y}))		
                calcCircleFrom3Points (Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt3x,pt3y});	
            else if (!IsPerpendicular(Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt1x,pt1y}))		
                calcCircleFrom3Points (Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt1x,pt1y});	
            else if (!IsPerpendicular(Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt1x,pt1y}))		
                calcCircleFrom3Points (Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt2x,pt2y}, Vec2<iq16>{pt1x,pt1y});	
            else if (!IsPerpendicular(Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt2x,pt2y}))		
                calcCircleFrom3Points (Vec2<iq16>{pt3x,pt3y}, Vec2<iq16>{pt1x,pt1y}, Vec2<iq16>{pt2x,pt2y});	
            return {0,0,0};
        }();

        std::tie(m_Centerx, m_Centery, m_dRadius) = result;
    };


real_t ArcInterpolation::forward(real_t x) const {
    static constexpr real_t epsilon = real_t(0.001);
    // static constexpr real_t min_param_a = real_t(0 + epsilon);
    // static constexpr real_t max_param_a = real_t(1 - epsilon);
    // static constexpr real_t min_param_b = real_t(0 + epsilon);
    // static constexpr real_t max_param_b = real_t(1 - epsilon);

    // auto [a,b] = handle;
    // a = MIN(max_param_a, MAX(min_param_a, a));
    // b = MIN(max_param_b, MAX(min_param_b, b));
    x = CLAMP(x, epsilon, real_t(1)-epsilon);
    



    // constrain
    // if ((m_Centerx > 0) && (m_Centerx < 1)){
    //     if (_a < m_Centerx){
    //         m_Centerx = 1;
    //         m_Centery = 0;
    //         m_dRadius = 1;
    //     } else {
    //         m_Centerx = 0;
    //         m_Centery = 1;
    //         m_dRadius = 1;
    //     }
    // }
    
    real_t y = 0;
    if (x >= m_Centerx){
        y = m_Centery - std::sqrt(square(m_dRadius) - square(x-m_Centerx)); 
    } else {
        y = m_Centery + std::sqrt(square(m_dRadius) - square(x-m_Centerx)); 
    }
    return y;
}



//------------------------------------------
// Return signed distance from line Ax + By + C = 0 to point P.
static real_t linetopoint (real_t a, real_t b, real_t c, real_t ptx, real_t pty){
  real_t lp = 0;
  real_t d = std::sqrt((a*a)+(b*b));
  if (d != 0){
    lp = (a*ptx + b*pty + c)/d;
  }
  return lp;
}

//------------------------------------------
// Compute the parameters of a circular arc 
// fillet between lines L1 (p1 to p2) and
// L2 (p3 to p4) with radius R.  
// 
void CircularFilletInterpoation::computeFilletParameters (
        const Vec2<iq16> & pt1,
        const Vec2<iq16> & pt2,
        const Vec2<iq16> & pt3,
        const Vec2<iq16> & pt4,
        real_t r) {

    auto [p1x, p1y] = pt1;
    auto [p2x, p2y] = pt2;
    auto [p3x, p3y] = pt3;
    auto [p4x, p4y] = pt4;
    real_t c1   = p2x*p1y - p1x*p2y;
    real_t a1   = p2y-p1y;
    real_t b1   = p1x-p2x;
    real_t c2   = p4x*p3y - p3x*p4y;
    real_t a2   = p4y-p3y;
    real_t b2   = p3x-p4x;
    if ((a1*b2) == (a2*b1)){  /* Parallel or coincident lines */
    return;
    }

    real_t mPx, mPy;
    mPx = (p3x + p4x)/2;
    mPy = (p3y + p4y)/2;
    const real_t d1 = linetopoint(a1,b1,c1,mPx,mPy);  /* Find distance p1p2 to p3 */
    if (d1 == 0) return; 

    mPx = (p1x + p2x)/2;
    mPy = (p1y + p2y)/2;
    const real_t d2 = linetopoint(a2,b2,c2,mPx,mPy);  /* Find distance p3p4 to p2 */
    if (d2 == 0) return;

    real_t c1p, c2p, d;
    real_t rr = r;
    if (d1 <= 0) {
    rr= -rr;
    }
    c1p = c1 - rr*std::sqrt((a1*a1)+(b1*b1));  /* Line parallel l1 at d */
    rr = r;
    if (d2 <= 0){
    rr = -rr;
    }
    c2p = c2 - rr*std::sqrt((a2*a2)+(b2*b2));  /* Line parallel l2 at d */
    d = (a1*b2)-(a2*b1);

    real_t pCx = (c2p*b1-c1p*b2)/d; /* Intersect constructed lines */
    real_t pCy = (c1p*a2-c2p*a1)/d; /* to find center of arc */
    real_t pAx = 0;
    real_t pAy = 0;
    real_t pBx = 0;
    real_t pBy = 0;
    real_t dP,cP;

    dP = (a1*a1) + (b1*b1);        /* Clip or extend lines as required */
    if (dP != 0){
        cP = a1*pCy - b1*pCx;
        pAx = (-a1*c1 - b1*cP)/dP;
        pAy = ( a1*cP - b1*c1)/dP;
    }

    dP = (a2*a2) + (b2*b2);
    if (dP != 0){
        cP = a2*pCy - b2*pCx;
        pBx = (-a2*c2 - b2*cP)/dP;
        pBy = ( a2*cP - b2*c2)/dP;
    }

    real_t gv1x = pAx-pCx; 
    real_t gv1y = pAy-pCy;
    real_t gv2x = pBx-pCx; 
    real_t gv2y = pBy-pCy;

    real_t arcStart = real_t(std::atan2(gv1y,gv1x)); 
    real_t arcAngle = 0;
    real_t dd = std::sqrt(((gv1x*gv1x)+(gv1y*gv1y)) * ((gv2x*gv2x)+(gv2y*gv2y)));
    if (dd != 0){
    arcAngle = (acos((gv1x*gv2x + gv1y*gv2y)/dd));
    } 
    real_t crossProduct = (gv1x*gv2y - gv2x*gv1y);
    if (crossProduct < 0){ 
    arcStart -= arcAngle;
    }

    real_t arc1 = arcStart;
    real_t arc2 = arcStart + arcAngle;
    if (crossProduct < 0){
    arc1 = arcStart + arcAngle;
    arc2 = arcStart;
    }

    arcCenterX    = pCx;
    arcCenterY    = pCy;
    arcStartAngle = arc1;
    arcEndAngle   = arc2;
    arcRadius     = r;
    arcStartX     = arcCenterX + arcRadius*cos(arcStartAngle);
    arcStartY     = arcCenterY + arcRadius*sin(arcStartAngle);
    arcEndX       = arcCenterX + arcRadius*cos(arcEndAngle);
    arcEndY       = arcCenterY + arcRadius*sin(arcEndAngle);
}

//--------------------------------------------------------
// Joining Two Lines with a Circular Arc Fillet
// Adapted from Robert D. Miller / Graphics Gems III.

real_t arcStartAngle;
real_t arcEndAngle;
real_t arcStartX,  arcStartY;
real_t arcEndX,    arcEndY;
real_t arcCenterX, arcCenterY;
real_t arcRadius;

//--------------------------------------------------------
real_t CircularFilletInterpoation::forward(real_t x) const {
  
//   static constexpr real_t epsilon = real_t(0.001);
//   static constexpr real_t min_param_a = real_t(0 + epsilon);
//   static constexpr real_t max_param_a = real_t(1 - epsilon);
//   static constexpr real_t min_param_b = real_t(0 + epsilon);
//   static constexpr real_t max_param_b = real_t(1 - epsilon);

//   auto [a,b] = handle;

//   a = MAX(min_param_a, MIN(max_param_a, a)); 
//   b = MAX(min_param_b, MIN(max_param_b, b)); 


  real_t t = 0;
  real_t y = 0;
  x = MAX(0, MIN(1, x));
  
  if (x <= arcStartX){
    t = x / arcStartX;
    y = t * arcStartY;
  } else if (x >= arcEndX){
    t = (x - arcEndX)/(1 - arcEndX);
    y = arcEndY + t*(1 - arcEndY);
  } else {
    if (x >= arcCenterX){
      y = arcCenterY - std::sqrt(square(arcRadius) - square(x-arcCenterX)); 
    } else{
      y = arcCenterY + std::sqrt(square(arcRadius) - square(x-arcCenterX)); 
    }
  }
  return y;
}

#endif
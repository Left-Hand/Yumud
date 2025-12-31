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

    const iq16 yDelta_a = pt2y - pt1y;
    const iq16 xDelta_a = pt2x - pt1x;
    const iq16 yDelta_b = pt3y - pt2y;
    const iq16 xDelta_b = pt3x - pt2x;
    static constexpr iq16 epsilon = iq16(0.001);

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

std::tuple<iq16, iq16, iq16> ArcInterpolation::calcCircleFrom3Points (const Vec2<iq16> & pt1,const Vec2<iq16> & pt2,const Vec2<iq16> & pt3) const {
    const auto [pt1x, pt1y] = pt1;
    const auto [pt2x, pt2y] = pt2;
    const auto [pt3x, pt3y] = pt3;

    const iq16 yDelta_a = pt2y - pt1y;
    const iq16 xDelta_a = pt2x - pt1x;
    const iq16 yDelta_b = pt3y - pt2y;
    const iq16 xDelta_b = pt3x - pt2x;
    static constexpr iq16 epsilon = iq16(0.001);

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
    iq16 aSlope = yDelta_a / xDelta_a; 
    iq16 bSlope = yDelta_b / xDelta_b;

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
        static constexpr iq16 pt1x = 0;
        static constexpr iq16 pt1y = 0;
        iq16 pt2x = _a;
        iq16 pt2y = _b;
        static constexpr iq16 pt3x = 1;
        static constexpr iq16 pt3y = 1;
        auto && result = [&]() -> std::tuple<iq16, iq16, iq16> {
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


iq16 ArcInterpolation::forward(iq16 x) const {
    static constexpr iq16 epsilon = iq16(0.001);
    // static constexpr iq16 min_param_a = iq16(0 + epsilon);
    // static constexpr iq16 max_param_a = iq16(1 - epsilon);
    // static constexpr iq16 min_param_b = iq16(0 + epsilon);
    // static constexpr iq16 max_param_b = iq16(1 - epsilon);

    // auto [a,b] = handle;
    // a = MIN(max_param_a, MAX(min_param_a, a));
    // b = MIN(max_param_b, MAX(min_param_b, b));
    x = CLAMP(x, epsilon, iq16(1)-epsilon);
    



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
    
    iq16 y = 0;
    if (x >= m_Centerx){
        y = m_Centery - std::sqrt(square(m_dRadius) - square(x-m_Centerx)); 
    } else {
        y = m_Centery + std::sqrt(square(m_dRadius) - square(x-m_Centerx)); 
    }
    return y;
}



//------------------------------------------
// Return signed distance from line Ax + By + C = 0 to point P.
static iq16 linetopoint (iq16 a, iq16 b, iq16 c, iq16 ptx, iq16 pty){
  iq16 lp = 0;
  iq16 d = std::sqrt((a*a)+(b*b));
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
        iq16 r) {

    auto [p1x, p1y] = pt1;
    auto [p2x, p2y] = pt2;
    auto [p3x, p3y] = pt3;
    auto [p4x, p4y] = pt4;
    iq16 c1   = p2x*p1y - p1x*p2y;
    iq16 a1   = p2y-p1y;
    iq16 b1   = p1x-p2x;
    iq16 c2   = p4x*p3y - p3x*p4y;
    iq16 a2   = p4y-p3y;
    iq16 b2   = p3x-p4x;
    if ((a1*b2) == (a2*b1)){  /* Parallel or coincident lines */
    return;
    }

    iq16 mPx, mPy;
    mPx = (p3x + p4x)/2;
    mPy = (p3y + p4y)/2;
    const iq16 d1 = linetopoint(a1,b1,c1,mPx,mPy);  /* Find distance p1p2 to p3 */
    if (d1 == 0) return; 

    mPx = (p1x + p2x)/2;
    mPy = (p1y + p2y)/2;
    const iq16 d2 = linetopoint(a2,b2,c2,mPx,mPy);  /* Find distance p3p4 to p2 */
    if (d2 == 0) return;

    iq16 c1p, c2p, d;
    iq16 rr = r;
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

    iq16 pCx = (c2p*b1-c1p*b2)/d; /* Intersect constructed lines */
    iq16 pCy = (c1p*a2-c2p*a1)/d; /* to find center of arc */
    iq16 pAx = 0;
    iq16 pAy = 0;
    iq16 pBx = 0;
    iq16 pBy = 0;
    iq16 dP,cP;

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

    iq16 gv1x = pAx-pCx; 
    iq16 gv1y = pAy-pCy;
    iq16 gv2x = pBx-pCx; 
    iq16 gv2y = pBy-pCy;

    iq16 arcStart = iq16(std::atan2(gv1y,gv1x)); 
    iq16 arcAngle = 0;
    iq16 dd = std::sqrt(((gv1x*gv1x)+(gv1y*gv1y)) * ((gv2x*gv2x)+(gv2y*gv2y)));
    if (dd != 0){
    arcAngle = (acos((gv1x*gv2x + gv1y*gv2y)/dd));
    } 
    iq16 crossProduct = (gv1x*gv2y - gv2x*gv1y);
    if (crossProduct < 0){ 
    arcStart -= arcAngle;
    }

    iq16 arc1 = arcStart;
    iq16 arc2 = arcStart + arcAngle;
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

iq16 arcStartAngle;
iq16 arcEndAngle;
iq16 arcStartX,  arcStartY;
iq16 arcEndX,    arcEndY;
iq16 arcCenterX, arcCenterY;
iq16 arcRadius;

//--------------------------------------------------------
iq16 CircularFilletInterpoation::forward(iq16 x) const {
  
//   static constexpr iq16 epsilon = iq16(0.001);
//   static constexpr iq16 min_param_a = iq16(0 + epsilon);
//   static constexpr iq16 max_param_a = iq16(1 - epsilon);
//   static constexpr iq16 min_param_b = iq16(0 + epsilon);
//   static constexpr iq16 max_param_b = iq16(1 - epsilon);

//   auto [a,b] = handle;

//   a = MAX(min_param_a, MIN(max_param_a, a)); 
//   b = MAX(min_param_b, MIN(max_param_b, b)); 


  iq16 t = 0;
  iq16 y = 0;
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
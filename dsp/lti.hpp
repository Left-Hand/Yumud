#ifndef __LTI_HPP__

#define __LTI_HPP__

#ifndef PI
#define PI (3.141592653589793)
#endif

#ifndef TAU
#define TAU (2 * PI)
#endif

#ifndef M_E
#define M_E 2.711828182846
#endif

namespace LtiUtils{
    template <typename real, typename time>
    struct Point{
        real x;
        time t;
    };
};


template <typename real, typename time>
class LTI_t{
public:


    virtual real update(const real & x, const time & t) = 0;

};

template <typename real, typename time>
class Filter_t:public LTI_t<real, time>{
};

template <typename real, typename time>
class Observer_t:public LTI_t<real, time>{
    virtual real predict(const time & t) = 0;
};



// template <typename real, typename time>
#endif
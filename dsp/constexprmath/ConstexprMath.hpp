#pragma once

// #include <math.h>


struct ConstexprMath{
    public:

#define constexpr_decl(return_type, function_name, ...) \
    scexpr return_type function_name(__VA_ARGS__) noexcept\

#define constexpr_name(function_name) function_name##_constexpr

constexpr_decl(double, sqrt, double x){
    if (x < 0)
        return -1; // error code for negative input
    double curr = x, prev = 0;
    while (curr != prev)
    {
        prev = curr;
        curr = (curr + x / curr) / 2;
    }
    return curr;
}


constexpr_decl(double, pow, double x, size_t n)
{
    double result = 1.0;
    for (size_t i = 0; i < n; ++i)
    {
        result *= x;
    }
    return result;
}

constexpr double sum_byhalf(size_t start, size_t end, double (*f)(size_t))
{
    size_t half = (start + end) / 2;
    return start == end ? f(start) : sum_byhalf(start, half, f) + sum_byhalf(half + 1, end, f);
};

template <size_t N = 21>
constexpr_decl(auto, factorial)
{
    static_assert(N <= 21, "factorial overflow");
    struct
    {
        size_t factorials[N];
    } ret;
    ret.factorials[0] = 1;
    for (size_t i = 1; i < N; ++i)
    {
        ret.factorials[i] = ret.factorials[i - 1] * i;
    }
    return ret;
}



static consteval int double_factorial(int x){
    int sum = 1;
    
    if(x <= 2) return 1;
    while(x > 2)
    {
        sum *= x;
        x -= 2;
    }
    return sum;
}

template <int Nrep = 65536, typename float_type = double>
        constexpr_decl(float_type, e, bool directaddmode = true)
        {// e = sum(1/n!,n=0,inf)
            float_type e = 0.0, laste = 0.0;
            float_type add = 1.0;
            size_t N = Nrep < 1 ? ~size_t(0) : Nrep;
            if (directaddmode)
            {
                for (size_t i = 0; i < N; ++i)
                {
                    laste = e;
                    e += add;
                    add /= i + 1;
                    if (e == laste)
                        break;
                }
                return e;
            }
            else
            {
                auto N65536 = N > 65536 ? 65536 : N;
                auto adder_e = [](size_t i) -> float_type
                {
                    float_type M_add = 1.0;
                    for (size_t j = 1; j <= i; ++j)
                    {
                        M_add /= j;
                    }
                    return M_add;
                };

                return sum_byhalf(0, N65536-1, adder_e);
            }
            return e;
        }

 template <int Nrep = 65536, typename float_type = double>
        constexpr_decl(float_type, exp_0_to_1, float_type _0_to_1)
        { // e^x = sum(x^n/n!,n=0,inf)
            float_type e = float_type(0), laste = float_type(0);
            float_type add = float_type(1);
            size_t N = Nrep < 1 ? ~size_t(0) : Nrep;
            size_t i;
            // 0<exp_0_to_1<1
            if (_0_to_1 < (float_type)0 || _0_to_1 > (float_type)1)
                return (float_type)0;

            for (i = 0; i < N; ++i)
            {
                laste = e;
                e += add;
                add *= _0_to_1 / (i + 1);
                if (e == laste)
                    break;
            }

            return e;
        }
    
    template <int Nrep = 65536, typename float_type = double>
        constexpr_decl(float_type, exp, float_type x)
        { // e^(2*x) = (e^x)*(e^x)
            size_t cnt = 0;
            bool x_neg = false;
            if (x < float_type(0))
            {
                x_neg = true;
                x = -x;
            }
            while (x > float_type(1))
            {
                x /= 2;
                ++cnt;
            }
            float_type exp = exp_0_to_1<Nrep>(x);
            if (x_neg)
            {
                exp = 1 / exp;
            }
            for (size_t i = 0; i < cnt; i++)
            {
                exp *= exp;
            }
            return exp;
        };
    
    template <int Nrep = 65536, typename float_type = double>
        constexpr_decl(float_type, ln_xadd1, float_type x_add1)
        { // ln(x+1) = sum((-1)^n*x^(n+1)/(n+1),n=0,inf)=x-x^2/2+x^3/3-x^4/4+...

            // do not accept too large x or small x
            //-0.25 <= 1+x <= 0.5
            // so 0.75 <= x <= 1.5

            if (x_add1 < float_type(-0.25) || x_add1 > float_type(0.5))
                return 0;

            float_type ln = 0.0, lastln = 0.0;
            float_type add = x_add1;
            size_t N = Nrep < 1 ? ~size_t(0) : Nrep;
            for (size_t i = 0; i < N; ++i)
            {
                lastln = ln;
                ln += add / (i + 1);
                add *= -x_add1;
                if (ln == lastln)
                    break;
            }
            return ln;
        }
    
    template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, ln, float_type _x){
        double x = static_cast<double>(_x);
        // ln(2*x) = ln(x)+ln(2)
            constexpr double sqrt2 = sqrt(2);
            constexpr double ln2 = ln_xadd1<Nrep>(sqrt2 - 1) * 2;
            size_t cnt = 0;
            // error
            if (x <= float_type(0))
                return 0;
            else if (x < float_type(0.75))
            {
                cnt = 0;
                while (x < float_type(0.75))
                {
                    x *= float_type(2);
                    ++cnt;
                }
                return (float_type)(ln_xadd1<Nrep>(x - 1) - ln2 * cnt);
            }
            else
            {
                cnt = 0;
                while (x > float_type(1.5))
                {
                    x /= float_type(2);
                    ++cnt;
                }
                return (float_type)(ln_xadd1<Nrep>(x - 1) + ln2 * cnt);
            }
        };
    
    // pow by float
        template <typename float_type>
        constexpr_decl(float_type, pow, float_type x, float_type y)
        {
            if (y == 0)
                return 1;
            else if (y == 1)
                return x;
            else if (x == 0)
                return 0;
            else if (x == 1)
                return 1;
            else if (y == 0.5)
                return sqrt(x);
            else //x^y = e^(y*ln(x))
                return exp(y * ln(x));
        }
    
    template <typename float_type>
        constexpr_decl(float_type, log, float_type x, float_type y)
        {
            if (x == 0 || y == 0)
                return 0;
            else if (x == 1)
                return 0;
            else if (y == 1)
                return 0;
            else if (x == y)
                return 1;
            else //log(x,y) = ln(y)/ln(x)
                return constexpr_name(ln)(y) / constexpr_name(ln)(x);
        }

     // sin 
        // template <int Nrep = 65536, typename float_type>
        // constexpr_decl(double, why, float_type _x){
        //     return static_cast<double>(_x);
        // }

        template <int Nrep = 65536, typename float_type>
        scexpr double why(float_type _x){
            return static_cast<double>(_x);
        }

        template <int Nrep = 65536, typename float_type>
        constexpr_decl(double, sin, float_type _x){
            double x = static_cast<double>(_x);
            bool x_neg = false;
            if (x < 0)
            {
                x_neg = !x_neg;
                x = -x;
            }
            double pi = M_PI;
            double halfpi = M_PI / 2;
            double twopi = M_PI * 2;
            while (x > pi)
            {
                x -= twopi;
            } // -pi <= x <= pi
            if (x < 0)
            {
                x = -x;
                x_neg = !x_neg;
            } // 0 <= x <= pi
            if (x > halfpi)
            {
                x = pi - x;
            } // 0 <= x <= pi/2
            auto ret = sin_0_to_halfpi<Nrep>(x);
            if (x_neg)
            {
                ret = -ret;
            }
            return ret;
        }

        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, cos, float_type _x){
            double x = static_cast<double>(_x);
            bool x_neg = false;
            if (x < 0)
                x = -x;
            double pi = M_PI;
            double halfpi = M_PI / 2;
            double twopi = M_PI * 2;
            while (x > pi)
            {
                x -= twopi;
            } // -pi <= x <= pi
            if (x < 0)
            {
                x = -x;
            } // 0 <= x <= pi
            if (x > halfpi)
            {
                x = pi - x;
                x_neg = !x_neg;
            } // 0 <= x <= pi/2
            auto ret = cos_0_to_halfpi<Nrep>(x);
            if (x_neg)
            {
                ret = -ret;
            }
            return ret;
        }

        
        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, tan, float_type _x){
            double x = static_cast<double>(_x);
            return static_cast<float_type>(sin(x) / cos(x));
        }


        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, asin, float_type _x)
        {
            double x = static_cast<double>(_x);
            return abs(_x) > 1 ? static_cast<float_type>(NAN) : static_cast<float_type>(atan(x / sqrt(1 - x * x)));
        }


        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, acos, float_type _x)
        {
            double x = static_cast<double>(_x);
            return static_cast<float_type>(M_PI / 2 - asin(x));
        }

        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, atan, float_type _x)
        {
            if(_x > 0){
                return _x > 1 ? ((M_PI/2) - (atan_0_to_1(1.0 / _x))) : atan_0_to_1(_x);
            }else{
                return -atan(-_x);
            }
        }

        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, atan2, float_type _x, float_type _y){
            static_assert(_x != 0 && _y != 0, "atan2 undefined when x and y are zero");
            if(_x > 0){
                atan(_y / _x);
            }else if(_x < 0){
                if(_y >= 0){
                    return atan(_y / _x) + M_PI;
                }else{
                    return atan(_y / _x) - M_PI;
                }
            }else if(_x == 0){
                if(_y > 0){
                    return M_PI / 2;
                }else if(_y < 0){
                    return -M_PI / 2;
                }
            }
        }

        protected:
        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, atan_0_to_1, float_type _x)
        {
            double x = static_cast<double>(_x);
            double x_m = x;
            double x_2 = x * x;
            double c = 1.0;
            double atangent = 0.0, lastatangent = 0.0;
            size_t N = Nrep < 1 ? ~size_t(0) : Nrep;
            for (size_t i = 0; i < N; ++i)
            {
                lastatangent = atangent;

                atangent += c * x_m;
                x_m *= x_2;
                c *= - double(2 * i + 1) / double(2 * i + 3);
                if (atangent == lastatangent)
                    break;
            }
            return static_cast<float_type>(atangent);
        }


        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, cos_0_to_halfpi, float_type _x)
        {
            double x = static_cast<double>(_x);
            // cos(x) = sum((-1)^n*x^(2n)/(2n)!,n=0,inf)=1-x^2/2!+x^4/4!-x^6/6!+...
            // 0 <= x <= pi/2
            double cosine = 0.0, lastcosine = 0.0;
            double add = 1.0;
            size_t N = Nrep < 1 ? ~size_t(0) : Nrep;
            for (size_t i = 0; i < N; ++i)
            {
                lastcosine = cosine;
                cosine += add;
                add *= -x * x / ((2 * i + 1) * (2 * i + 2));
                if (cosine == lastcosine)
                    break;
            }
            return static_cast<float_type>(cosine);
        }

        template <int Nrep = 65536, typename float_type>
        constexpr_decl(float_type, sin_0_to_halfpi, float_type _x)
        {
            double x = static_cast<double>(_x);
            // sin(x) = sum((-1)^n*x^(2n+1)/(2n+1)!,n=0,inf)=x-x^3/3!+x^5/5!-x^7/7!+...
            // 0 <= x <= pi/2
            double sine = 0.0, lastsine = 0.0;
            double add = x;
            size_t N = Nrep < 1 ? ~size_t(0) : Nrep;
            for (size_t i = 0; i < N; ++i)
            {
                lastsine = sine;
                sine += add;
                add *= -x * x / ((2 * i + 2) * (2 * i + 3));
                if (sine == lastsine)
                    break;
            }
            return static_cast<float_type>(sine);
        }


};
///
///codes here
///

#pragma once

#include <ratio>

#include "core/stream/ostream.hpp"

namespace ymd::unit{

template<
    int L,  //长度
    int M,  //质量
    int T,  //时间
    int I,  //电流
    int Q,  //热力学温度
    int N,  //物质的量
    int J,  //发光强度
    int A   //转角(这个单位并不在传统量纲分析中出现 但分析广义力时需要)
>
struct Policy{
private:
    template<int L2,int M2, int T2, int I2, int Q2, int N2, int J2, int A2>
    using _Mul = Policy<L + L2, M + M2, T + T2, I + I2, Q + Q2, N + N2, J + J2, A + A2>;

    template<int L2,int M2, int T2, int I2, int Q2, int N2, int J2, int A2>
    using _Div = Policy<L - L2, M - M2, T - T2, I - I2, Q - Q2, N - N2, J - J2, A - A2>;
public:
    static constexpr int m_L = L;
    static constexpr int m_M = M;
    static constexpr int m_T = T;
    static constexpr int m_I = I;
    static constexpr int m_Q = Q;
    static constexpr int m_N = N;
    static constexpr int m_J = J;
    static constexpr int m_A = A;

    template<typename Other>
    using mul_with_t = _Mul<
        Other::m_L,
        Other::m_M,
        Other::m_T,
        Other::m_I,
        Other::m_Q,
        Other::m_N,
        Other::m_J,
        Other::m_A
    >;

    template<typename Other>
    using div_with_t = _Div<
        Other::m_L,
        Other::m_M,
        Other::m_T,
        Other::m_I,
        Other::m_Q,
        Other::m_N,
        Other::m_J,
        Other::m_A
    >;

    using inv_t = Policy<
        -m_L,
        -m_M,
        -m_T,
        -m_I,
        -m_Q,
        -m_N,
        -m_J,
        -m_A
    >;

};

using identity_ratio = std::ratio<1,1>;


namespace policy{
    using none =                    Policy<0, 0, 0, 0, 0, 0, 0, 0>;
    using mass =                    Policy<1, 0, 0, 0, 0, 0, 0, 0>;
    using length =                  Policy<0, 1, 0, 0, 0, 0, 0, 0>;
    using time =                    Policy<0, 0, 1, 0, 0, 0, 0, 0>;
    using current =                 Policy<0, 0, 0, 1, 0, 0, 0, 0>;
    using temperature =             Policy<0, 0, 0, 0, 1, 0, 0, 0>;
    using intensity =               Policy<0, 0, 0, 0, 0, 1, 0, 0>;
    using amount_of_substance =     Policy<0, 0, 0, 0, 0, 0, 1, 0>;
    using spin             =       Policy<0, 0, 0, 0, 0, 0, 0, 1>;

    using frequency = time::inv_t;
    using speed = length::div_with_t<time>;
    using angular_speed = spin::div_with_t<time>;
    using charge = current::mul_with_t<time>;

    // Kinematics
    using acceleration = speed::div_with_t<time>;          // m/s²
    using jerk = acceleration::div_with_t<time>;           // m/s³

    // Dynamics
    using force = mass::mul_with_t<acceleration>;          // N (kg·m/s²)
    using torque = force::mul_with_t<length>::mul_with_t<spin>;  // N·m·rad⁻¹
    using energy = force::mul_with_t<length>;  
    using power = energy::div_with_t<time>;                // W (J/s)

    // Electromagnetism
    using voltage = power::div_with_t<current>;            // V (W/A)
    using resistance = voltage::div_with_t<current>;       // Ω (V/A)
    using capacitance = charge::div_with_t<voltage>;       // F (C/V)

    // Magnetic flux (Weber)
    using magnetic_flux = voltage::mul_with_t<time>;  // Wb = V·s
    
    // Magnetic flux density (Tesla)
    using magnetic_flux_density = magnetic_flux::div_with_t<length::mul_with_t<length>>; // T = Wb/m²
    
    // Inductance (Henry)
    using inductance = magnetic_flux::div_with_t<current>;  // H = Wb/A
    
    // Permeability
    using permeability = inductance::div_with_t<length>;   // H/m

    using entropy = energy::div_with_t<temperature>;               // J/K
}

template<typename TPolicy, typename T, typename TRatio>
struct Quantity{
public:
    using Self = Quantity;

    using ratio = TRatio;

    T count;

    constexpr Quantity(const T & _count):count(_count){;}


    template<typename TRatio2>
    constexpr Quantity & operator = (const Quantity<TPolicy, T, TRatio2> & other) {
        using scaler = std::ratio_divide<TRatio2, TRatio>;
        constexpr auto scale = static_cast<T>(scaler::num) / scaler::den; 
        count = other.count * scale;
        return *this;
    }
    constexpr Quantity & operator +=(const Quantity & other){
        count = count + other.count;
        return *this;
    }

    constexpr Quantity & operator -=(const Quantity & other){
        count = count - other.count;
        return *this;
    }

    
    template<typename TPolicy2, typename TRatio2>
    constexpr auto operator * (const Quantity<TPolicy2, T, TRatio2> & other) const noexcept {
        using Ret = Quantity<typename TPolicy::mul_with_t<TPolicy2>, T, std::ratio_multiply<TRatio, TRatio2>>;
        return Ret(count * other.count);
    }

    template<typename TPolicy2, typename TRatio2>
    constexpr auto operator / (const Quantity<TPolicy2, T, TRatio2> & other) const noexcept {
        using Ret = Quantity<typename TPolicy::div_with_t<TPolicy2>, T, std::ratio_divide<TRatio, TRatio2>>;
        return Ret(count / other.count);
    }

    constexpr Quantity operator *(const T & other){
        return Quantity(count * other);
    }

    constexpr Quantity operator /(const T & other){
        return Quantity(count / other);
    }

    
    template<typename TRatio2>
    constexpr auto operator + (const Quantity<TPolicy, T, TRatio2> & other) const noexcept {
        return add_sub_impl([](auto v1, auto v2){return v1 + v2;},other);
    }

    template<typename TRatio2>
    constexpr auto operator - (const Quantity<TPolicy, T, TRatio2> & other) const noexcept {
        return add_sub_impl([](auto v1, auto v2){return v1 - v2;},other);
    }

private:
    template<typename Fn, typename TRatio2>
    constexpr auto add_sub_impl(Fn && fn, const Quantity<TPolicy, T, TRatio2> & other) const noexcept {
        using scaler = std::ratio_divide<TRatio2, TRatio>;
        using Ret = Quantity<TPolicy, T, TRatio>;
        constexpr auto scale = static_cast<T>(scaler::num) / scaler::den; 
        const auto rhs = other.count * scale;
        return Ret(std::forward<Fn>(fn)(count, rhs));
    }
};

template<typename TPolicy, typename T, typename TRatio>
[[nodiscard]] __attribute__((always_inline)) constexpr 
std::strong_ordering operator <=> (const Quantity<TPolicy, T, TRatio> & self, const Quantity<TPolicy, T, TRatio> & other) {
    return self.count <=> other.count;
}

template<typename TPolicy, typename T, typename TRatio>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool operator == (const Quantity<TPolicy, T, TRatio> & self, const Quantity<TPolicy, T, TRatio> & other) {
    return self.count == other.count;
}


#define DEF_META_TEMPLATE(name, policy, ratio, prefix)\
template<typename T>\
using name = Quantity<policy, T, ratio>;\
consteval name<float> operator""_##prefix(long double x) { return name<float>(x); }\
consteval name<float> operator""_##prefix(uint64_t x) { return name<float>(float(x)); }\
template<typename T>\
::ymd::OutputStream & operator <<(::ymd::OutputStream & os, const Quantity<policy, T, ratio> & _count){\
    return os <<_count.get() << #prefix ;\
}\


#define DEF_RATIO(num, den) std::ratio<num, den>

DEF_META_TEMPLATE(MetersPerSecond,  policy::speed,          identity_ratio,     m_s)

DEF_META_TEMPLATE(Seconds,          policy::time,           identity_ratio,     s)
DEF_META_TEMPLATE(MilliSeconds,     policy::time,           std::milli,         ms)
DEF_META_TEMPLATE(MicroSeconds,     policy::time,           std::micro,         us)
DEF_META_TEMPLATE(Minutes,          policy::time,           DEF_RATIO(60, 1),   minutes)
DEF_META_TEMPLATE(Hours,            policy::time,           DEF_RATIO(3600, 1), hours)

DEF_META_TEMPLATE(BeatPerMinute,    policy::frequency,      DEF_RATIO(1,60),    bpm)
DEF_META_TEMPLATE(Hertz,            policy::frequency,      identity_ratio,     hz)
DEF_META_TEMPLATE(KiloHertz,        policy::frequency,      std::kilo,          khz)
DEF_META_TEMPLATE(MegaHertz,        policy::frequency,      std::mega,          mhz)
DEF_META_TEMPLATE(GigaHertz,        policy::frequency,      std::giga,          ghz)

DEF_META_TEMPLATE(Revolutions,           policy::spin,                      identity_ratio,     r)
DEF_META_TEMPLATE(RevolutionsPerSecond,  policy::spin::div_with_t<policy::time>,   identity_ratio,     rps)
DEF_META_TEMPLATE(RevolutionsPerMinute,  policy::spin::div_with_t<policy::time>,   DEF_RATIO(1,60),    rpm)

DEF_META_TEMPLATE(Meter,            policy::length,         identity_ratio,     m)
DEF_META_TEMPLATE(KiliMeter,        policy::length,         std::kilo,          km)
DEF_META_TEMPLATE(MilliMeter,       policy::length,         std::milli,         mm)
DEF_META_TEMPLATE(MicroMeter,       policy::length,         std::micro,         um)


DEF_META_TEMPLATE(Watt,             policy::power,          identity_ratio,     W)
DEF_META_TEMPLATE(MilliWatt,        policy::power,          std::milli,         mW)
DEF_META_TEMPLATE(MicroWatt,        policy::power,          std::micro,         uW)

DEF_META_TEMPLATE(Newton,           policy::force,          identity_ratio,     n)
DEF_META_TEMPLATE(KilloNewton,      policy::force,          std::kilo,          kn)

DEF_META_TEMPLATE(Volt,             policy::voltage,        identity_ratio,     V)
DEF_META_TEMPLATE(MilliVolt,        policy::voltage,        std::milli,         mV)
DEF_META_TEMPLATE(MicroVolt,        policy::voltage,        std::micro,         uV)

DEF_META_TEMPLATE(Ampere,           policy::current,        identity_ratio,     A)
DEF_META_TEMPLATE(MilliAmpere,      policy::current,        std::milli,         mA)
DEF_META_TEMPLATE(MicroAmpere,      policy::current,        std::micro,         uA)

DEF_META_TEMPLATE(Farad,            policy::capacitance,    identity_ratio,     F)
DEF_META_TEMPLATE(MilliFarad,       policy::capacitance,    std::milli,         mF)
DEF_META_TEMPLATE(MicroFarad,       policy::capacitance,    std::micro,         uF)
DEF_META_TEMPLATE(ManoFarad,        policy::capacitance,    std::nano,          nF)
DEF_META_TEMPLATE(PicoFarad,        policy::capacitance,    std::pico,          pF)

DEF_META_TEMPLATE(Ohm,              policy::resistance,     identity_ratio,     Ohm)
DEF_META_TEMPLATE(KiliOhm,          policy::resistance,     std::kilo,          kOhm)
DEF_META_TEMPLATE(MilliOhm,         policy::resistance,     std::milli,         mOhm)
DEF_META_TEMPLATE(MicroOhm,         policy::resistance,     std::micro,         uOhm)

DEF_META_TEMPLATE(Henry,            policy::inductance,     identity_ratio,     H)
DEF_META_TEMPLATE(MilliHenry,       policy::inductance,     std::milli,         mH)
DEF_META_TEMPLATE(MicroHenry,       policy::inductance,     std::micro,         uH)


DEF_META_TEMPLATE(AmperePerMeter,   policy::current::div_with_t<policy::length>, identity_ratio, A_m)
DEF_META_TEMPLATE(Weber,            policy::magnetic_flux,         identity_ratio,     Wb)
DEF_META_TEMPLATE(Tesla,            policy::magnetic_flux_density, identity_ratio,     T)
DEF_META_TEMPLATE(Gauss,            policy::magnetic_flux_density, DEF_RATIO(1,10000), Gs) // 1T = 10⁴Gs

DEF_META_TEMPLATE(Kelvin,           policy::temperature,    identity_ratio,     K)
DEF_META_TEMPLATE(JoulePerKelvin,   policy::entropy,        identity_ratio,     J_K)


DEF_META_TEMPLATE(NewtonMeter,      policy::torque,         identity_ratio,     Nm)
DEF_META_TEMPLATE(Joule,            policy::energy,         identity_ratio,     J)

#undef DEF_META_TEMPLATE
#undef DEF_RATIO


}
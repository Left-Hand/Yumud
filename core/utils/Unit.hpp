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
    using Mul = _Mul<
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
    using Div = _Div<
        Other::m_L,
        Other::m_M,
        Other::m_T,
        Other::m_I,
        Other::m_Q,
        Other::m_N,
        Other::m_J,
        Other::m_A
    >;

    using Inv = Policy<
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

    using frequency = time::Inv;
    using speed = length::Div<time>;
    using angular_speed = spin::Div<time>;
    using charge = current::Mul<time>;

    // Kinematics
    using acceleration = speed::Div<time>;          // m/s²
    using jerk = acceleration::Div<time>;           // m/s³

    // Dynamics
    using force = mass::Mul<acceleration>;          // N (kg·m/s²)
    using torque = force::Mul<length>::Mul<spin>;  // N·m·rad⁻¹
    using energy = force::Mul<length>;  
    using power = energy::Div<time>;                // W (J/s)

    // Electromagnetism
    using voltage = power::Div<current>;            // V (W/A)
    using resistance = voltage::Div<current>;       // Ω (V/A)
    using capacitance = charge::Div<voltage>;       // F (C/V)

    // Magnetic flux (Weber)
    using magnetic_flux = voltage::Mul<time>;  // Wb = V·s
    
    // Magnetic flux density (Tesla)
    using magnetic_flux_density = magnetic_flux::Div<length::Mul<length>>; // T = Wb/m²
    
    // Inductance (Henry)
    using inductance = magnetic_flux::Div<current>;  // H = Wb/A
    
    // Permeability
    using permeability = inductance::Div<length>;   // H/m

    using entropy = energy::Div<temperature>;               // J/K
}

// // Add C++20 concept for unit checks
// template<typename U>
// concept is_unit = requires {
//     typename U::ratio;
//     typename U::TPolicy;
// };

// // Modify operators with concept constraints
// template<is_unit U1, is_unit U2>
// requires std::is_same_v<typename U1::TPolicy, typename U2::TPolicy>
// auto operator+(const U1& a, const U2& b) {
//     return a.template add_sub_impl([](auto x, auto y){ return x + y; }, b);
// }

// template<typename NewUnit>
// constexpr NewUnit convert() const {
//     static_assert(std::is_same_v<TPolicy, NewUnit::TPolicy>,
//         "Cannot convert between different dimension units");
//     using conv_ratio = std::ratio_divide<ratio, typename NewUnit::ratio>;
//     return NewUnit(value_ * conv_ratio::num / conv_ratio::den);
// }



template<typename TPolicy, typename T, typename TRatio>
struct Quantity{
public:
    using ratio = TRatio;
    constexpr Quantity(const T & value):value_(value){;}

    constexpr Quantity & operator +=(const Quantity & other){
        value_ = value_ + other.value_;
        return *this;
    }

    constexpr Quantity & operator -=(const Quantity & other){
        value_ = value_ - other.value_;
        return *this;
    }

    
    template<typename TPolicy2, typename TRatio2>
    constexpr auto operator * (const Quantity<TPolicy2, T, TRatio2> & other) const{
        using Ret = Quantity<typename TPolicy::Mul<TPolicy2>, T, std::ratio_multiply<TRatio, TRatio2>>;
        return Ret(value_ * other.value_);
    }

    template<typename TPolicy2, typename TRatio2>
    constexpr auto operator / (const Quantity<TPolicy2, T, TRatio2> & other) const{
        using Ret = Quantity<typename TPolicy::Div<TPolicy2>, T, std::ratio_divide<TRatio, TRatio2>>;
        return Ret(value_ / other.value_);
    }

    constexpr Quantity operator *(const T & other){
        return Quantity(value_ * other);
    }

    constexpr Quantity operator /(const T & other){
        return Quantity(value_ / other);
    }

    
    template<typename TRatio2>
    constexpr auto operator + (const Quantity<TPolicy, T, TRatio2> & other) const{
        return add_sub_impl([](auto v1, auto v2){return v1 + v2;},other);
    }

    template<typename TRatio2>
    constexpr auto operator - (const Quantity<TPolicy, T, TRatio2> & other) const{
        return add_sub_impl([](auto v1, auto v2){return v1 - v2;},other);
    }


    constexpr T get() const {return value_;}

public:
    T value_;

private:
    template<typename Fn, typename TRatio2>
    constexpr auto add_sub_impl(Fn && fn, const Quantity<TPolicy, T, TRatio2> & other) const{
        using scaler = std::ratio_divide<TRatio2, TRatio>;
        using Ret = Quantity<TPolicy, T, TRatio>;
        constexpr auto scale = static_cast<T>(scaler::num) / scaler::den; 
        const auto rhs = other.value_ * scale;
        return Ret(std::forward<Fn>(fn)(value_, rhs));
    }
};

#define DEF_META_TEMPLATE(name, policy, ratio, prefix)\
template<typename T>\
using name = Quantity<policy, T, ratio>;\
consteval name<float> operator"" _##prefix(long double x) { return name<float>(x); }\
consteval name<float> operator"" _##prefix(uint64_t x) { return name<float>(float(x)); }\
template<typename T>\
::ymd::OutputStream & operator <<(::ymd::OutputStream & os, const Quantity<policy, T, ratio> & value){\
    return os <<value.get() << #prefix ;\
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
DEF_META_TEMPLATE(RevolutionsPerSecond,  policy::spin::Div<policy::time>,   identity_ratio,     rps)
DEF_META_TEMPLATE(RevolutionsPerMinute,  policy::spin::Div<policy::time>,   DEF_RATIO(1,60),    rpm)

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


DEF_META_TEMPLATE(AmperePerMeter,   policy::current::Div<policy::length>, identity_ratio, A_m)
// DEF_META_TEMPLATE(AmpereTurn,       policy::current, identity_ratio, _At)
DEF_META_TEMPLATE(Weber,            policy::magnetic_flux,         identity_ratio,     Wb)
DEF_META_TEMPLATE(Tesla,            policy::magnetic_flux_density, identity_ratio,     T)
DEF_META_TEMPLATE(Gauss,            policy::magnetic_flux_density, DEF_RATIO(1,10000), Gs) // 1T = 10⁴Gs

DEF_META_TEMPLATE(Kelvin,           policy::temperature,    identity_ratio,     K)
DEF_META_TEMPLATE(JoulePerKelvin,   policy::entropy,        identity_ratio,     J_K)


DEF_META_TEMPLATE(NewtonMeter,      policy::torque,         identity_ratio,     Nm)
DEF_META_TEMPLATE(Joule,            policy::energy,         identity_ratio,     J)

#undef DEF_META_TEMPLATE


namespace details::test{
static constexpr auto l1 = 1_mm;
static constexpr auto t1 = 1_s;

static constexpr auto v1 = l1 / t1;
static constexpr auto v2 = 1_m_s;
static constexpr auto v3 = v1 + v2;
static constexpr auto v4 = v2 + v1;

static constexpr auto force1 = 2.5_kn + 300_n;      // 2800 N
static constexpr auto energy1 = 1.8_J * 5;          // 9 J
static constexpr auto power1 = energy1 / 2_s;       // 4.5 W
static constexpr auto torque1 = 150_n * 0.3_m; 
static constexpr auto work = 5_n * 2_m;        // 10_J (energy)
static constexpr auto rotation = 3.14_r;
static constexpr auto torque = 150_n * 0.3_m / 1_r; // 45_Nm (torque)
static constexpr auto dist = 1_km;
static constexpr auto volt = 1_W / 1_A;

constexpr auto L = 100_mH;
constexpr auto dI = 2_A;
constexpr auto dt = 50_ms;
constexpr auto V = L * dI / dt;  // 100mH * 2A / 50ms = 4V

// Magnetic flux calculation
constexpr auto B = 1.5_T;
constexpr auto A = 0.01_m * 0.02_m; // 0.0002m²
constexpr auto phi = B * A;         // 0.0003 Wb

// Inductive reactance XL = 2πfL
constexpr auto f = 50_ghz; // 1Hz = 1/s
constexpr auto XL = f * L * TAU; // ~31.4159 Ω
}
}
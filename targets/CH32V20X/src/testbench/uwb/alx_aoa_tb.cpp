#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/sync/timer.hpp"
#include "core/utils/zero.hpp"
#include "core/utils/bits/atomic_bitset.hpp"
#include "types/vectors/Vector2.hpp"
#include "types/vectors/Vector3.hpp"
#include "types/vectors/polar.hpp"
#include "types/vectors/spherical_coordinates.hpp"
#include "types/shapes/circle2.hpp"

#include "drivers/Proximeter/ALX_AOA/alx_aoa_prelude.hpp"

using namespace ymd;
using drivers::AlxAoa_Prelude;

// #define DEBUGGER_INST hal::uart2

using AlxEvent = AlxAoa_Prelude::Event;
using AlxError = AlxAoa_Prelude::Error;

using AlxLocation = drivers::AlxAoa_Prelude::Location;
using AlxHeartBeat = drivers::AlxAoa_Prelude::HeartBeat;


using Measurement = SphericalCoordinates<float>;
static constexpr Measurement loc_to_meas (const AlxLocation & loc){
    return Measurement{
        .distance = static_cast<float>(loc.distance.to_meters()),
        .azimuth = loc.azimuth.to_angle().into<float>(),
        .elevation = loc.elevation.to_angle().into<float>(),
    };
}

template<typename V, typename T>
struct PointWithDistance{
    V point;
    T distance;
};


template<typename T>
constexpr HeaplessVector<Vec2<T>, 2> compute_intersection_points(
    const Circle2<T>& circle_a, 
    const Circle2<T>& circle_b
) {

    // d = math.sqrt((x2 - x1)**2 + (y2 - y1)**2)
    // if d > r1 + r2 or d < abs(r1 - r2):
    // return None # 圆不相交


    using Container = HeaplessVector<Vec2<T>, 2>;
    // 卫语句1: 半径检查
    if (circle_a.radius < T(0) || circle_b.radius < T(0)) [[unlikely]] 
        return Container::from_empty();
        // PANIC();
    
    const Vec2<T>& p1 = circle_a.center;
    const Vec2<T>& p2 = circle_b.center;
    const T & r1 = circle_a.radius;
    const T & r2 = circle_b.radius;
    
    const Vec2<T> delta = p2 - p1;
    const T & dx = delta.x;
    const T & dy = delta.y;
    const T distance_squ = dx * dx + dy * dy;
    
    if(distance_squ > square(r1 + r2))
        return Container::from_empty();

    if(distance_squ < square(r1 - r2))
        return Container::from_empty();
    
    // a = (r1**2 - r2**2 + d**2) / (2 * d)
    const T d = std::sqrt(distance_squ);
    const T inv_d = T(1) / d;
    const T a = (r1 * r1 - r2 * r2 + distance_squ) * T(0.5) * inv_d;
    // h = math.sqrt(r1**2 - a**2)
    const T h = std::sqrt(r1 * r1 - a * a);
    // x0 = x1 + a * (x2 - x1) / d
    // y0 = y1 + a * (y2 - y1) / d
    const T x0 = p1.x + a * dx * inv_d;
    const T y0 = p1.y + a * dy * inv_d;
    // rx = -(y2 - y1) * (h / d)
    // ry = (x2 - x1) * (h / d)
    const T rx = -(dy) * (h * inv_d);
    const T ry = (dx) * (h * inv_d);
    // intersection1 = (x0 + rx, y0 + ry)
    // intersection2 = (x0 - rx, y0 - ry)
    // return intersection1, intersection2
    return Container(
        Vec2<T>(x0 + rx, y0 + ry),
        Vec2<T>(x0 - rx, y0 - ry)
    );
}

void alx_aoa_main(){

    #if defined(CH32V30X)
    DEBUGGER_INST.init({
        576000 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    // DEBUGGER.no_brackets(EN);
    DEBUGGER.no_brackets(DISEN);
    DEBUGGER.no_fieldname(EN);

    // using SensorMeasurementsBarrier = MeasurementBarrier<Measurement, 2>;
    using SensorMeasurements = std::array<Measurement, 2>;
    SensorMeasurements measurements_ = {Zero, Zero};

    auto alx_ev_handler = [&](const Result<AlxEvent, AlxError> & res, const size_t idx){ 
        // PANIC{nth.count()};
        // DEBUG_PRINTLN(nth.count());
        if(res.is_ok()){
            const auto & ev = res.unwrap();
            if(ev.is<AlxLocation>()){
                const AlxLocation & loc = ev.unwrap_as<AlxLocation>();
                const auto && measurement = loc_to_meas(loc);
                measurements_.at(idx) = measurement;

            }else if(ev.is<AlxHeartBeat>()){
                DEBUG_PRINTLN("alx_heartBeat", ev.unwrap_as<AlxHeartBeat>());
            }
        }else{
            [[maybe_unused]] const auto & err = res.unwrap_err();
            // DEBUG_PRINTLN("alx_err", err);

        }
    };





    auto alx_parser1 = drivers::AlxAoa_StreamParser(
        [&](const Result<AlxEvent, AlxError> & res){
            alx_ev_handler(res, 0);
        }
    );

    auto alx_parser2 = drivers::AlxAoa_StreamParser(
        [&](const Result<AlxEvent, AlxError> & res){
            alx_ev_handler(res, 1);
        }
    );

    auto & alx_uart1 = hal::uart1;
    auto & alx_uart2 = hal::uart2;

    alx_uart1.init({
        AlxAoa_Prelude::DEFAULT_UART_BUAD
    });

    alx_uart2.init({
        AlxAoa_Prelude::DEFAULT_UART_BUAD
    });


    auto red_led_gpio_ = hal::PC<13>();
    auto blue_led_gpio_ = hal::PC<14>();
    red_led_gpio_.outpp();
    blue_led_gpio_.outpp();
    
    struct BlinkProcedure{
        hal::Gpio & red_led_gpio_;
        hal::Gpio & blue_led_gpio_;

        void resume(){
            red_led_gpio_ = BoolLevel::from((
                uint32_t(clock::millis().count()) % 200) > 100);
            blue_led_gpio_ = BoolLevel::from((
                uint32_t(clock::millis().count()) % 400) > 200);
        }
    };

    struct AlxProcedure{
        hal::UartHw & alx_uart_;
        drivers::AlxAoa_StreamParser & alx_parser_;
        uint32_t received_bytes_cnt_ = 0;

        void resume(){
            if(alx_uart_.available() == 0) return;
            while(alx_uart_.available()){
                char chr;
                alx_uart_.read1(chr);
                // _bytes.push_back(uint8_t(chr));
                alx_parser_.push_byte(static_cast<uint8_t>(chr)); 
                received_bytes_cnt_++;
            }
        }
    };

    BlinkProcedure blink_procedure_{
        .red_led_gpio_ = red_led_gpio_,
        .blue_led_gpio_ = blue_led_gpio_
    };

    AlxProcedure alx_procedure1_{
        .alx_uart_ = alx_uart1,
        .alx_parser_ = alx_parser1
    };

    AlxProcedure alx_procedure2_{
        .alx_uart_ = alx_uart2,
        .alx_parser_ = alx_parser2
    };

    DEBUG_PRINTLN("setup done");
    while(true){

        // DEBUG_PRINTLN("alx_uart_rx", uint8_t(chr));
        alx_procedure1_.resume();
        alx_procedure2_.resume();


        blink_procedure_.resume();

        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        
        report_timer.invoke_if([&]{
            const auto & measurement = measurements_[0];
            // const auto [s,c] = measurement.azimuth.sincos(); 
            // const auto [x,y] = Vector2<float>::from_length_and_
            [[maybe_unused]] const auto vec3 = measurement.to_vec3();
            [[maybe_unused]] const auto [x,y,z] = vec3;
            [[maybe_unused]] const auto o1 = Vec2<float>(0.1, 0);
            [[maybe_unused]] const auto o2 = Vec2<float>(-0.1, 0);
            
            [[maybe_unused]] const auto circle_a = Circle2<float>{o1, measurements_[0].to_polar().amplitude};
            [[maybe_unused]] const auto circle_b = Circle2<float>{o2, measurements_[1].to_polar().amplitude};
            // [[maybe_unused]] const auto [x,y,z] = vec3;
            // const auto polar = Polar<float>{
            //     .amplitude = measurement.distance,
            //     .phase = measurement.azimuth
            // };

            // const auto [x,y] = polar.to_vec2();
            // const auto points = compute_intersection_points(circle_a, circle_b);
            const auto points = compute_intersection_points(
                Circle2<float>{Vec2<float>{-1,0}, 1.4141f}, 
                // Circle2<float>{Vec2<float>{1,0}, 1.4f}
                Circle2<float>{Vec2<float>{1,0}, 1.0f}
            );
            const auto p = points.at_or(0, Zero);
            // const auto p = [&] -> Vec2<float>{
            //     if(may_p.is_some()){
            //         // DEBUG_PRINTLN(may_p.unwrap());
            //         return may_p.unwrap();
            //     }else{ 
            //         return Vec2<float>(0,0);
            //     }
            // }();
            DEBUG_PRINTLN(
                // measuremen/ts_
                bool(measurements_[0].distance > measurements_[1].distance)
                // ,measurements_[0].distance, measurements_[1].distance
                // ,measurements_[0].azimuth.to_radian(), measurements_[1].azimuth.to_radian()
                ,p, points.size()
                // x,y,z
                // DEBUGGER.config().no_fieldname, 
                // DEBUGGER.field("distance")(1),
                // DEBUGGER.field("distanc")(2),
                // DEBUGGER.field("distane")(3)
                // DEBUGGER.scoped("meas")(measurement), 
                // DEBUGGER.scoped("xyz")(x, y, z)
                // DEBUGGER.scoped("xyz")(vec3),
                // DEBUGGER.scoped("abc")(vec3),
                // DEBUGGER.scoped("uvw")(std::ignore)
            );
            // if(bytes.size() == 0) return;
            // if(DEBUGGER.pending() != 0) return;
            // PANIC{bytes};
            // DEBUG_PRINTLN_IDLE(
            //     received_bytes_cnt_
            // );
            // for(const auto byte : bytes){
            //     DEBUG_PRINT(byte);
            // }
    });
    }

    #else

    PANIC{"not supported"}

    #endif
}
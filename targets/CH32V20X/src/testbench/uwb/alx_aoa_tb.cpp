#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/can/can.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/async/timer.hpp"
#include "core/utils/zero.hpp"
#include "core/utils/bits/atomic_bitset.hpp"
#include "geometry.hpp"
#include "drivers/Proximeter/MK8000TR/mk8000tr_stream.hpp"
#include "drivers/Proximeter/ALX_AOA/alx_aoa_prelude.hpp"


using namespace ymd;
using namespace ymd::drivers;

// #define DEBUGGER_INST hal::uart2

using AlxEvent = drivers::alx_aoa::Event;
using AlxError = drivers::alx_aoa::Error;

using AlxLocation = drivers::alx_aoa::Location;
using AlxHeartBeat = drivers::alx_aoa::HeartBeat;
using drivers::mk8000tr::MK8000TR_ParserSink;
using drivers::alx_aoa::AlxAoa_ParserSink;
using Mk8Event = drivers::mk8000tr::Event;

using AlxMeasurement = SphericalCoordinates<float>;

namespace alx_aoa_tb{
struct [[nodiscard]] Mk8Measurement{
    using Self = Mk8Measurement;

    float distance;
    float strength;

    static constexpr Self zero(){
        return Self{
            .distance = 0,
            .strength = 0
        };
    }

    friend OutputStream & operator << (OutputStream & os, const Self & self){
        return os
            << os.field("distance")(self.distance) << os.splitter()
            << os.field("strength")(self.strength)
        ;
    }
};


struct BlinkActivity{
    hal::Gpio & blue_led_pin_;
    void resume(){
        blue_led_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
    }
};

struct AlxActivity{
    hal::Uart & uart_;
    drivers::alx_aoa::AlxAoa_ParserSink & parser_;
    uint32_t received_bytes_cnt_ = 0;

    void resume(){
        if(uart_.available() == 0) return;
        while(uart_.available()){
            char chr;
            const auto len = uart_.try_read_char(chr);
            if(len == 0) break;
            // _bytes.push_back(uint8_t(chr));
            parser_.push_byte(static_cast<uint8_t>(chr)); 
            received_bytes_cnt_++;
        }
    }
};
}

using namespace alx_aoa_tb;


void alx_aoa_main(){

    #if defined(CH32V30X)
    hal::uart2.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 ,
        .tx_strategy = CommStrategy::Blocking
        // 115200
    });



    DEBUGGER.retarget(&hal::uart2);
    DEBUGGER.no_brackets(DISEN);
    DEBUGGER.no_fieldname(EN);

    using AlxMeasurements = std::array<AlxMeasurement, 2>;
    AlxMeasurements alx_measurements_ = {Zero, Zero};

    using Mk8Measurements = std::array<Mk8Measurement, 2>;

    [[maybe_unused]] auto alx_ev_handler = [&](const Result<AlxEvent, AlxError> & res, const size_t idx){ 

        if(res.is_ok()){
            const auto & ev = res.unwrap();
            if(ev.is<AlxLocation>()){
                const AlxLocation & loc = ev.unwrap_as<AlxLocation>();
                const auto && measurement = loc.to_spherical_coordinates<float>();
                alx_measurements_.at(idx) = measurement;
                // if(idx == 1)PANIC{idx};
            }else if(ev.is<AlxHeartBeat>()){
            }
        }else{
            [[maybe_unused]] const auto & err = res.unwrap_err();
            // PANIC{uint8_t(err)};
        }
    };

    auto alx_1_parser_ = AlxAoa_ParserSink(
        [&](const Result<AlxEvent, AlxError> & res){

            alx_ev_handler(res, 0);
        }
    );

    auto alx_2_parser_ = AlxAoa_ParserSink(
        [&](const Result<AlxEvent, AlxError> & res){
            alx_ev_handler(res, 1);
        }
    );

    
    hal::uart3.init({
        .remap = hal::UART3_REMAP_PB10_PB11,
        .baudrate = alx_aoa::DEFAULT_UART_BAUD,
    });

    hal::uart4.init({
        .remap = hal::UART4_REMAP_PC10_PC11,
        .baudrate = alx_aoa::DEFAULT_UART_BAUD,
    });


    [[maybe_unused]] auto & alx_1_uart_ = hal::uart3;
    [[maybe_unused]] auto & alx_2_uart_ = hal::uart4;

    alx_1_uart_.set_event_handler([&](const hal::UartEvent & ev){

        switch(ev.kind()){
            case hal::UartEvent::RxIdle:

                while(alx_1_uart_.available()){
                    char chr;
                    const auto read_len = alx_1_uart_.try_read_char(chr);
                    if(read_len == 0) break;
                    alx_1_parser_.push_byte(static_cast<uint8_t>(chr)); 
                }
                break;
            default:
                break;
        }
    });


    alx_2_uart_.set_event_handler([&](const hal::UartEvent & ev){
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:
                while(alx_2_uart_.available()){
                    char chr;
                    const auto read_len = alx_2_uart_.try_read_char(chr);
                    if(read_len == 0) break;
                    alx_2_parser_.push_byte(static_cast<uint8_t>(chr)); 
                }
                break;
            default:
                break;
        }
    });



    auto blue_led_pin_ = hal::PC<13>();

    blue_led_pin_.outpp();

    BlinkActivity blink_activity_{
        .blue_led_pin_ = blue_led_pin_
    };


    while(true){

        blink_activity_.resume();

        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        
        report_timer.invoke_if([&]{
            #if 0
            const auto & alx_measurement = alx_measurements_[0];
            // const auto [s,c] = alx_measurement.azimuth.sincos(); 
            // const auto [x,y] = Vector2<float>::from_length_and_
            [[maybe_unused]] const auto vec3 = alx_measurement.to_vec3();
            [[maybe_unused]] const auto [x,y,z] = vec3;
            [[maybe_unused]] const auto o1 = Vec2<float>(0.20, 0);
            [[maybe_unused]] const auto o2 = Vec2<float>(-0.20, 0);
            
            // [[maybe_unused]] const auto circle_a = Circle2<float>{o1, alx_measurements_[0].to_polar().amplitude};
            // [[maybe_unused]] const auto circle_b = Circle2<float>{o2, alx_measurements_[1].to_polar().amplitude};


            // [[maybe_unused]] const auto [x,y,z] = vec3;
            // const auto polar = Polar<float>{
            //     .amplitude = alx_measurement.distance,
            //     .phase = alx_measurement.azimuth
            // };

            // const auto [x,y] = polar.to_vec2();
            const auto points = geometry::compute_intersection_points(circle_a, circle_b);
            const auto p = points.at_or(0, Zero);
            DEBUG_PRINTLN(
                // measuremen/ts_
                // 0

                // bool(alx_measurements_[0].distance > alx_measurements_[1].distance)
                // ,alx_measurements_[0].distance, alx_measurements_[1].distance
                mk8_measurements_[0].distance, 
                mk8_measurements_[1].distance,
                p.x, p.y
                // ,alx_measurements_[0].azimuth.to_radians(), alx_measurements_[1].azimuth.to_radians()
                // ,p, points.size()
                // x,y,z
                // DEBUGGER.config().no_fieldname, 
                // DEBUGGER.field("distance")(1),
                // DEBUGGER.field("distanc")(2),
                // DEBUGGER.field("distane")(3)
                // DEBUGGER.scoped("meas")(alx_measurement), 
                // DEBUGGER.scoped("xyz")(x, y, z)
                // DEBUGGER.scoped("xyz")(vec3),
                // DEBUGGER.scoped("abc")(vec3),
                // DEBUGGER.scoped("uvw")(std::ignore)
            );
            #else
            [[maybe_unused]] const auto & alx_measurement = alx_measurements_[0];
            const auto & front_meas = alx_measurements_[1];
            const auto & back_meas = alx_measurements_[0];

            const auto front_offset_vec3 = front_meas.to_vec3();
            const auto back_offset_vec3 = back_meas.to_vec3();

            static constexpr float VERTICAL_DISJUDGE_CIRCLE_RADIUS = 0.6f;
            const auto front_offset_vec2 = Vec2f(front_offset_vec3.x, front_offset_vec3.y);
            const auto back_offset_vec2 = Vec2f(back_offset_vec3.x, back_offset_vec3.y);

            [[maybe_unused]] const auto FRONT_BASE = Vec2f(0.0, 0.37f);
            [[maybe_unused]] const auto BACK_BASE = Vec2f(0.0, -0.37f);

            [[maybe_unused]] const auto circle_front = Circle2<float>{FRONT_BASE, front_meas.distance};
            [[maybe_unused]] const auto circle_back = Circle2<float>{BACK_BASE, back_meas.distance};

            const auto points = geometry::compute_intersection_points(circle_front, circle_back);
            const auto may_p = points.at_or(0, Vec2f::ZERO);

            const bool is_in_side_disjudge_region = ABS(front_meas.distance - back_meas.distance) < 0.5f;
            const bool is_in_front_disjudge_region = front_meas.distance < VERTICAL_DISJUDGE_CIRCLE_RADIUS;
            const bool is_in_back_disjudge_region = back_meas.distance < VERTICAL_DISJUDGE_CIRCLE_RADIUS;

            const bool is_in_strong_judge_region =
                (is_in_front_disjudge_region == false) and (is_in_back_disjudge_region == false) and (is_in_side_disjudge_region == false);
            DEBUG_PRINTLN(
                // alx_measurement.distance,
                // is_in_side_disjudge_region,
                // front_meas.distance - back_meas.distance,
                // is_in_front_disjudge_region,
                // is_in_back_disjudge_region,
                // is_in_strong_judge_region,
                front_meas.azimuth.to_turns(),
                // back_meas.distance < VERTICAL_DISJUDGE_CIRCLE_RADIUS
                // front_meas.azimuth.to_turns()
                // alx_measurement.azimuth.to_radians()
                // points.at_or(0, Vec2f::ZERO),
                may_p.x,
                may_p.y
                // front_meas.distance,
                // front_meas.azimuth.to_degrees(),
                // back_meas.distance,
                // front_offset_vec2,
                // back_offset_vec2
                // front_vec2,
                // back_vec2
                // alx_measurements_[1]
            );
            #endif
    });
    }

    #else

    PANIC{"not supported"};

    #endif
}
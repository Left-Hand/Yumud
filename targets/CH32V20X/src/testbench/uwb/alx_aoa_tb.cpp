#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/dma/dma.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/async/timer.hpp"
#include "core/utils/zero.hpp"
#include "core/container/atomic_bitset.hpp"
#include "geometry.hpp"
#include "drivers/Proximeter/MK8000TR/mk8000tr_stream.hpp"
#include "drivers/Proximeter/ALX_AOA/alx_aoa_prelude.hpp"

#include "algebra/regions/Ray2.hpp"



using namespace ymd;
using namespace ymd::drivers;

// #define DEBUGGER_INST hal::usart2

using AlxEvent = drivers::alx_aoa::Event;
using AlxError = drivers::alx_aoa::Error;

using AlxLocation = drivers::alx_aoa::Location;
using AlxHeartBeat = drivers::alx_aoa::HeartBeat;
using drivers::mk8000tr::MK8000TR_ParseReceiver;
using drivers::alx_aoa::AlxAoa_ParseReceiver;
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
    drivers::alx_aoa::AlxAoa_ParseReceiver & parser_;
    uint32_t received_bytes_cnt_ = 0;

    void resume(){
        if(uart_.available() == 0) return;
        while(uart_.available()){
            uint8_t byte;
            const auto len = uart_.try_read_byte(byte);
            if(len == 0) break;
            // _bytes.push_back(uint8_t(byte));
            parser_.push_byte(static_cast<uint8_t>(byte));
            received_bytes_cnt_++;
        }
    }
};
}

using namespace alx_aoa_tb;


void alx_aoa_main(){

    #if defined(CH32V30X)



    auto blue_led_pin_ = hal::PC<13>();

    blue_led_pin_.outpp();

    BlinkActivity blink_activity_{
        .blue_led_pin_ = blue_led_pin_
    };


    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(115200 * 2),
        .tx_strategy = CommStrategy::Blocking
        // 115200
    });

    // hal::uart.init({
    //     .remap = hal::UART5_REMAP_PC12_PD2,
    //     .baudrate = hal::NearestFreq(115200 * 2),
    //     .tx_strategy = CommStrategy::Blocking
    //     // 115200
    // });



    DEBUGGER.retarget(&hal::usart2);
    DEBUGGER.no_brackets(DISEN);
    DEBUGGER.no_fieldname(EN);

    // while(true){
    //     clock::delay(5ms);
    //     DEBUG_PRINTLN_IDLE(0);
    // }

    using AlxMeasurements = std::array<AlxMeasurement, 2>;
    AlxMeasurements alx_measurements_ = {Zero, Zero};

    using Mk8Measurements = std::array<Mk8Measurement, 2>;

    [[maybe_unused]] auto alx_ev_handler = [&](const Result<AlxEvent, AlxError> & res, const size_t idx){

        if(res.is_ok()){
            const auto & ev = res.unwrap();
            if(ev.is<AlxLocation>()){
                const AlxLocation & loc = ev.unwrap_as<AlxLocation>();
                auto measurement = loc.to_spherical_coordinates<float>();
                measurement.azimuth = -measurement.azimuth;
                measurement.elevation = -measurement.elevation;
                alx_measurements_.at(idx) = measurement;
                // if(idx == 1)PANIC{idx};
            }else if(ev.is<AlxHeartBeat>()){
            }
        }else{
            [[maybe_unused]] const auto & err = res.unwrap_err();
            // PANIC{uint8_t(err)};
        }
    };

    auto alx_1_parser_ = AlxAoa_ParseReceiver(
        [&](const Result<AlxEvent, AlxError> & res){

            alx_ev_handler(res, 0);
        }
    );

    auto alx_2_parser_ = AlxAoa_ParseReceiver(
        [&](const Result<AlxEvent, AlxError> & res){
            alx_ev_handler(res, 1);
        }
    );


    hal::usart3.init({
        .remap = hal::USART3_REMAP_PB10_PB11,
        .baudrate = hal::NearestFreq(alx_aoa::DEFAULT_UART_BAUD),
        .tx_strategy = CommStrategy::Blocking
    });

    hal::uart4.init({
        .remap = hal::UART4_REMAP_PC10_PC11,
        .baudrate = hal::NearestFreq(alx_aoa::DEFAULT_UART_BAUD),
    });


    [[maybe_unused]] auto & alx_1_uart_ = hal::usart3;
    [[maybe_unused]] auto & alx_2_uart_ = hal::uart4;

    alx_1_uart_.set_event_callback([&](const hal::UartEvent & ev){

        switch(ev.kind()){
            case hal::UartEvent::RxIdle:

                while(alx_1_uart_.available()){
                    uint8_t byte;
                    const auto read_len = alx_1_uart_.try_read_byte(byte);
                    if(read_len == 0) break;
                    alx_1_parser_.push_byte(static_cast<uint8_t>(byte));
                }
                break;
            default:
                break;
        }
    });


    alx_2_uart_.set_event_callback([&](const hal::UartEvent & ev){
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:
                while(alx_2_uart_.available()){
                    uint8_t byte;
                    const auto read_len = alx_2_uart_.try_read_byte(byte);
                    if(read_len == 0) break;
                    alx_2_parser_.push_byte(static_cast<uint8_t>(byte));
                }
                break;
            default:
                break;
        }
    });




    while(true){

        blink_activity_.resume();

        static auto report_timer = async::RepeatTimer::from_duration(3ms);

        report_timer.invoke_if([&]{

            [[maybe_unused]] const auto & alx_measurement = alx_measurements_[0];
            const auto & left_raw_meas = alx_measurements_[1];
            const auto & right_raw_meas = alx_measurements_[0];

            const auto left_meas = std::get<0>(left_raw_meas.to_polar_and_height());
            const auto right_meas = std::get<0>(right_raw_meas.to_polar_and_height());
            // const auto left_offset_vec3 = left_raw_meas.to_vec3();
            // const auto right_offset_vec3 = right_raw_meas.to_vec3();

            // static constexpr float NEAR_JUDEGE_RADIUS = 0.6f;
            // static constexpr float FAR_JUDEGE_RADIUS = 1.6f;
            // [[maybe_unused]] const auto left_offset_vec2 = Vec2f(left_offset_vec3.x, left_offset_vec3.y);
            // [[maybe_unused]] const auto right_offset_vec2 = Vec2f(right_offset_vec3.x, right_offset_vec3.y);

            [[maybe_unused]] const auto LEFT_ANGLE_BASE = Angular<float>::from_degrees(135);
            [[maybe_unused]] const auto LEFT_BASE = Vec2f(-0.30f, 0.0f);
            [[maybe_unused]] const auto RIGHT_ANGLE_BASE = Angular<float>::from_degrees(45);
            [[maybe_unused]] const auto RIGHT_BASE = Vec2f(0.30f, -0.0f);

            [[maybe_unused]] const auto left_circle = Circle2<float>{LEFT_BASE, left_raw_meas.distance};
            [[maybe_unused]] const auto right_circle = Circle2<float>{RIGHT_BASE, right_raw_meas.distance};


            // const auto points = geometry::compute_intersection_points(left_circle, right_circle);
            // const auto may_p = points.at_or(0, Vec2f::ZERO);

            // const bool is_right_near = ABS(left_raw_meas.distance - right_raw_meas.distance) < 0.5f;
            // [[maybe_unused]] const bool is_left_near = left_meas.amplitude < NEAR_JUDEGE_RADIUS;
            // [[maybe_unused]] const bool is_right_near = right_meas.amplitude < NEAR_JUDEGE_RADIUS;

            // [[maybe_unused]] const bool is_left_far = left_meas.amplitude < FAR_JUDEGE_RADIUS;
            // [[maybe_unused]] const bool is_right_far = right_meas.amplitude < FAR_JUDEGE_RADIUS;

            // [[maybe_unused]] const bool is_near =
            //     (is_left_near ) or (is_right_near);

            // [[maybe_unused]] const bool is_far =
            //     (is_left_far) and (is_right_far);

            // const auto may_dual_p = geometry::compute_intersection_points(left_circle, right_circle);
            // const auto p = may_dual_p.size() ? may_dual_p[0] : Vec2f::ZERO;
            const auto left_ray = Ray2<float>(LEFT_BASE, LEFT_ANGLE_BASE + left_meas.phase);
            const auto right_ray = Ray2<float>(RIGHT_BASE, RIGHT_ANGLE_BASE + right_meas.phase);

            // const auto p = geometry::compute_intersection_point(left_ray, right_circle).unwrap_or(Vec2f::ZERO);

            [[maybe_unused]] const auto left_est_point = left_ray.endpoint_at_length(left_meas.amplitude);
            [[maybe_unused]] const auto right_est_point = right_ray.endpoint_at_length(right_meas.amplitude);
            // const auto may_p = left_ray.intersection(right_ray, 0.0001f);
            // const auto cp = may_p.unwrap_or(Vec2f::ZERO);
            // return;
            DEBUG_PRINTLN_IDLE(
                // alx_measurement.distance,
                // is_right_near,
                // left_meas.distance,
                // ,

                // left_meas.distance,
                // right_meas.distance,
                // left_meas.phase.to_degrees(),
                // left_est_err,
                // right_est_err,
                // left_meas.phase.to_radians(),
                // right_meas.phase.to_radians(),
                // cp.x,
                // cp.y,
                // (cp - LEFT_BASE.org).length(),
                // (cp - RIGHT_BASE.org).length(),
                // left_meas.distance,
                // right_meas.distance,
                // left_est_point,
                // right_est_point,
                // p
                // geometry::compute_intersection_point(left_ray, right_circle).unwrap_or(Vec2f::ZERO),
                // geometry::compute_intersection_point(right_ray, left_circle).unwrap_or(Vec2f::ZERO),
                hal::usart2.available(),
                hal::usart3.tx_dma_buf_index_,
                hal::usart3.tx_queue_.length(),
                // hal::usart3.try_write_bytes("1234567890", 10),
                USART3_TX_DMA_CH.pending_count()
                // 0
                // (left_raw_meas.distance > right_raw_meas.distance) ? right_est_point : right_est_point

                // ((left_est_point - RIGHT_BASE).angle() - (right_meas.phase + RIGHT_ANGLE_BASE)).to_turns(),
                // ((right_est_point - LEFT_BASE).angle() - (left_meas.phase + LEFT_ANGLE_BASE)).to_turns(),
                // left_est_point.x,
                // left_est_point.y,
                // (right_est_point - left_est_point).length(),
                // ((cp - LEFT_BASE.org).length() - left_meas.distance),
                // ((cp - RIGHT_BASE.org).length() - right_meas.distance),
                // 0
                // is_left_near,
                // is_in_right_disjudge_region,
                // is_near,
                // left_meas.azimuth.to_turns(),
                // right_meas.distance < NEAR_JUDEGE_RADIUS
                // left_meas.azimuth.to_turns()
                // alx_measurement.azimuth.to_radians()
                // points.at_or(0, Vec2f::ZERO),
                // may_p.x,
                // may_p.y
                // left_meas.distance,
                // left_meas.azimuth.to_degrees(),
                // right_meas.distance,
                // left_offset_vec2,
                // right_offset_vec2
                // left_vec2,
                // right_vec2
                // alx_measurements_[1]
            );
    });
    }

    #else

    PANIC{"not supported"};

    #endif
}

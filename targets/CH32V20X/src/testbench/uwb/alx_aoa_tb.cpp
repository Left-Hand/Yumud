#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/sync/timer.hpp"
#include "core/utils/zero.hpp"
#include "core/utils/bits/atomic_bitset.hpp"
#include "geometry.hpp"
#include "drivers/Proximeter/MK8000TR/mk8000tr_uart.hpp"
#include "drivers/Proximeter/ALX_AOA/alx_aoa_prelude.hpp"


using namespace ymd;
using drivers::MK8000TR_Prelude;
using drivers::AlxAoa_Prelude;

// #define DEBUGGER_INST hal::uart2

using AlxEvent = drivers::AlxAoa_Prelude::Event;
using AlxError = drivers::AlxAoa_Prelude::Error;

using AlxLocation = drivers::AlxAoa_Prelude::Location;
using AlxHeartBeat = drivers::AlxAoa_Prelude::HeartBeat;
using Mk8Event = drivers::MK8000TR_Prelude::Event;

using AlxMeasurement = SphericalCoordinates<float>;


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


struct BlinkProcedure{
    hal::Gpio & red_led_gpio_;
    hal::Gpio & blue_led_gpio_;

    void init(){

        red_led_gpio_.outpp();
        blue_led_gpio_.outpp();
    }
    void resume(){
        red_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        blue_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
    }
};

struct AlxProcedure{
    hal::UartHw & uart_;
    drivers::AlxAoa_StreamParser & parser_;
    uint32_t received_bytes_cnt_ = 0;

    void init(){
        uart_.init({
            AlxAoa_Prelude::DEFAULT_UART_BUAD
        });
    }
    void resume(){
        if(uart_.available() == 0) return;
        while(uart_.available()){
            char chr;
            uart_.read1(chr);
            // _bytes.push_back(uint8_t(chr));
            parser_.push_byte(static_cast<uint8_t>(chr)); 
            received_bytes_cnt_++;
        }
    }
};

struct MK8000Procedure{
    hal::UartHw & uart_;
    drivers::MK8000TR_StreamParser& parser_;
    uint32_t received_bytes_cnt_ = 0;
    void init(){
        uart_.init({
            115200 * 2
        });
    }
    void resume(){
        if(uart_.available() == 0) return;
        // DEBUG_PRINTLN(received_bytes_cnt_);
        while(uart_.available()){
            char chr;
            uart_.read1(chr);
            // _bytes.push_back(uint8_t(chr));
            parser_.push_byte(static_cast<uint8_t>(chr)); 
            received_bytes_cnt_++;
        }
    }
};



template<typename ... Ts>
struct ProcedureGroup{ 
    using procedures_type = std::tuple<std::reference_wrapper<Ts>...>;
    
    ProcedureGroup(Ts& ... args): 
        procedures_{std::ref(args)...} 
    {}

    void resume_all(){
        // 方法1：使用 std::apply 和折叠表达式
        std::apply([](auto&... procedures) {
            (procedures.get().resume(), ...);
        }, procedures_);
        
        // 或者方法2：使用索引序列（备选方案）
        // resume_all_impl(std::index_sequence_for<Ts...>{});
    }

    void init_all(){
        // 方法1：使用 std::apply 和折叠表达式
        std::apply([](auto&... procedures) {
            (procedures.get().init(), ...);
        }, procedures_);
        
        // 或者方法2：使用索引序列（备选方案）
        // resume_all_impl(std::index_sequence_for<Ts...>{});
    }

private:
    procedures_type procedures_;
};

// CTAD 指南
template<typename ... Ts>
ProcedureGroup(Ts& ... args) -> ProcedureGroup<Ts...>;
void alx_aoa_main(){

    #if defined(CH32V30X)
    DEBUGGER_INST.init({
        576000 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(DISEN);
    DEBUGGER.no_fieldname(EN);

    using AlxMeasurements = std::array<AlxMeasurement, 2>;
    AlxMeasurements alx_measurements_ = {Zero, Zero};

    using Mk8Measurements = std::array<Mk8Measurement, 2>;
    Mk8Measurements mk8_measurements_ = {Zero, Zero};

    auto mk8_ev_handler = [&](const Mk8Event & ev, const size_t idx){ 
        switch(idx){
            default:
                PANIC("mk8_ev_handler");
            case 0:
            case 1:
                mk8_measurements_[idx] = Mk8Measurement{
                    .distance = static_cast<float>(ev.dist_cm) / 100,
                    .strength = ev.signal_strength.to_dbm<float>()
                };
                break;
        }
    };

    auto alx_ev_handler = [&](const Result<AlxEvent, AlxError> & res, const size_t idx){ 
        // PANIC{nth.count()};
        // DEBUG_PRINTLN(nth.count());
        if(res.is_ok()){
            const auto & ev = res.unwrap();
            if(ev.is<AlxLocation>()){
                const AlxLocation & loc = ev.unwrap_as<AlxLocation>();
                const auto && measurement = loc.to_spherical_coordinates<float>();
                alx_measurements_.at(idx) = measurement;

            }else if(ev.is<AlxHeartBeat>()){
                // DEBUG_PRINTLN("alx_heartBeat", ev.unwrap_as<AlxHeartBeat>());
            }
        }else{
            [[maybe_unused]] const auto & err = res.unwrap_err();
            // DEBUG_PRINTLN("alx_err", err);

        }
    };

    auto alx_1_parser_ = drivers::AlxAoa_StreamParser(
        [&](const Result<AlxEvent, AlxError> & res){
            alx_ev_handler(res, 0);
        }
    );

    auto alx_2_parser_ = drivers::AlxAoa_StreamParser(
        [&](const Result<AlxEvent, AlxError> & res){
            alx_ev_handler(res, 1);
        }
    );

    auto mk8_1_parser_ = drivers::MK8000TR_StreamParser(
        [&](const Mk8Event & ev){
            mk8_ev_handler(ev, 0);
        }
    );

    auto mk8_2_parser_ = drivers::MK8000TR_StreamParser(
        [&](const Mk8Event & ev){
            mk8_ev_handler(ev, 1);
        }
    );


    [[maybe_unused]] auto & alx_1_uart_ = hal::uart1;
    [[maybe_unused]] auto & alx_2_uart_ = hal::uart2;
    [[maybe_unused]] auto & mk8_1_uart_ = hal::uart1;
    [[maybe_unused]] auto & mk8_2_uart_ = hal::uart2;

    auto red_led_gpio_ = hal::PC<13>();
    auto blue_led_gpio_ = hal::PC<14>();

    BlinkProcedure blink_procedure_{
        .red_led_gpio_ = red_led_gpio_,
        .blue_led_gpio_ = blue_led_gpio_
    };

    [[maybe_unused]] AlxProcedure alx_1_procedure_{
        .uart_ = alx_1_uart_,
        .parser_ = alx_1_parser_
    };

    [[maybe_unused]] AlxProcedure alx_2_procedure_{
        .uart_ = alx_2_uart_,
        .parser_ = alx_2_parser_
    };

    [[maybe_unused]] MK8000Procedure mk8_1_procedure_{
        .uart_ = mk8_1_uart_,
        .parser_ = mk8_1_parser_
    };

    [[maybe_unused]] MK8000Procedure mk8_2_procedure_{
        .uart_ = mk8_2_uart_,
        .parser_ = mk8_2_parser_
    };


    ProcedureGroup group_(
        blink_procedure_, 
        // alx_1_procedure_, 
        // alx_2_procedure_
        mk8_1_procedure_, 
        mk8_2_procedure_
    );

    DEBUG_PRINTLN("setup done");

    group_.init_all();

    while(true){
        group_.resume_all();

        blink_procedure_.resume();

        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        
        report_timer.invoke_if([&]{
            const auto & measurement = alx_measurements_[0];
            // const auto [s,c] = measurement.azimuth.sincos(); 
            // const auto [x,y] = Vector2<float>::from_length_and_
            [[maybe_unused]] const auto vec3 = measurement.to_vec3();
            [[maybe_unused]] const auto [x,y,z] = vec3;
            [[maybe_unused]] const auto o1 = Vec2<float>(0.20, 0);
            [[maybe_unused]] const auto o2 = Vec2<float>(-0.20, 0);
            
            // [[maybe_unused]] const auto circle_a = Circle2<float>{o1, alx_measurements_[0].to_polar().amplitude};
            // [[maybe_unused]] const auto circle_b = Circle2<float>{o2, alx_measurements_[1].to_polar().amplitude};

            [[maybe_unused]] const auto circle_a = Circle2<float>{o1, mk8_measurements_[0].distance};
            [[maybe_unused]] const auto circle_b = Circle2<float>{o2, mk8_measurements_[1].distance};


            // [[maybe_unused]] const auto [x,y,z] = vec3;
            // const auto polar = Polar<float>{
            //     .amplitude = measurement.distance,
            //     .phase = measurement.azimuth
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
                // DEBUGGER.scoped("meas")(measurement), 
                // DEBUGGER.scoped("xyz")(x, y, z)
                // DEBUGGER.scoped("xyz")(vec3),
                // DEBUGGER.scoped("abc")(vec3),
                // DEBUGGER.scoped("uvw")(std::ignore)
            );
    });
    }

    #else

    PANIC{"not supported"};

    #endif
}
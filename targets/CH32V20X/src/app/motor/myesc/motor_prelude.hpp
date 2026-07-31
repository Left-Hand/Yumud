#pragma once


#include "motor_leso.hpp"
#include "motor_dsp/dsp_pll.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"


namespace ymd::myesc{

using namespace ymd::digipw;

using namespace ymd::dsp;

//磁结构
//直观理解表贴于内置式的磁路
// https://blog.csdn.net/u010632165/article/details/103637894
enum class [[nodiscard]] MagneticStructure{
    //表贴式永磁同步电机(Surface-Mounted Permanent Magnet Synchronous Motor)
    SurfaceMounted,
    //内嵌式永磁同步电机(Interior Permanent Magnet Synchronous Motor)
    Interior
};



struct alignas(4) TimerTick{
    // int16_t bits;
    uint16_t counter_value;
    bool is_up_counting;
};


//不做温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_25C = 0;

//使用MCU结温进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_DIEJUNC = 1;

//使用外部温度输入1(通常为FET温度)进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_EXT1 = 2;

//使用外部温度输入2(通常为转子温度)进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_EXT2 = 3;


// static constexpr auto MOS_1C840L_500MA_BEST_DEADTIME_NS = 120ns;
// static constexpr auto MOS_1C840L_500MA_BEST_DEADTIME_NS = 400ns;
// static constexpr auto MOS_1C840L_500MA_BEST_DEADTIME_NS = 1000ns;


enum class [[nodiscard]] LoopWiring:uint8_t{
    SeriesPi = 0,
    Mit,
};

enum class [[nodiscard]] HfiMethod:uint8_t{
    Disabled = 0,
    Spin,
    PulseV1,
    PulseV2
};

enum class [[nodiscard]] ElecAngleSource:uint8_t{
    Openloop = 0,
    // Hall,
    Observer,
    Hfi,
    MagEncoder,
    AbzEncoder
};

enum class [[nodiscard]] TrajSmoothMethod:uint8_t{
    Disabled = 0,
    UseX1AndZero,
    UseX1AndX2,
};

struct alignas(4) [[nodiscard]] AlertFlags{
    uint32_t phase_u_disconn:1;
    uint32_t sense_u_noisy:1;
    uint32_t sense_u_levellow:1;
    uint32_t sense_u_levelhigh:1;
    
    uint32_t phase_v_disconn:1;
    uint32_t sense_v_noisy:1;
    uint32_t sense_v_levellow:1;
    uint32_t sense_v_levelhigh:1;

    uint32_t phase_w_disconn:1;
    uint32_t sense_w_noisy:1;
    uint32_t sense_w_levellow:1;
    uint32_t sense_w_levelhigh:1;

    uint32_t busbar_under_voltage:1;
    uint32_t busbar_over_voltage:1;
    uint32_t regen_power_rising:1;
    uint32_t unblance_curr_high:1;

    uint32_t resistance_abnormal_high:1;
    uint32_t resistance_abnormal_low:1;
    uint32_t inductance_abnormal_high:1;
    uint32_t inductance_abnormal_low:1;

    uint32_t rotor_overheat:1;
    uint32_t pcba_overheat:1;
    uint32_t ntc_levellow:1;
    uint32_t ntc_levelhigh:1;

    uint32_t encoder_angle_noisy:1;
    uint32_t encoder_magweak:1;
    uint32_t encoder_highspeed:1;
    uint32_t encoder_frequently_crc_err:1;
    uint32_t encoder_frequently_jitter:1;


    uint32_t distburnce_too_big:1;


};

static_assert(sizeof(AlertFlags) <= 4);

struct alignas(4) [[nodiscard]] OpFlags{
    using Self = OpFlags;


    uint32_t dc_calibrate_unready:1;
    uint32_t initial_encoder_dirtest:1;
    uint32_t measure_resind:1;
    uint32_t measure_flux:1;
    uint32_t sideshaft_calibrate_unready:1;

    constexpr void reset(){
        *this = std::bit_cast<Self>(uint32_t(0));
    }

    constexpr bool any() const {
        return std::bit_cast<uint32_t>(*this) != 0;
    }

    static constexpr Self zero(){
        Self self;
        self.reset();
        return self;
    }

    static constexpr Self from_default(){
        return zero();
    }
};


struct alignas(4) [[nodiscard]] FnSwitches{
    using Self = FnSwitches;

    uint32_t phase_invert_en : 1;


    uint32_t deadtime_compensate_en : 1;
    uint32_t current_harmonic_suppression_en : 1;
    uint32_t encoder_harmonic_suppression_en : 1;
    
    uint32_t cross_decoupling_en : 1;
    uint32_t bemf_decoupling_en : 1;


    uint32_t flux_observer_en : 1;

    uint32_t mtpa_en : 1;
    uint32_t mtpv_en : 1;

    uint32_t sideshaft_compenstate_en : 1;
    uint32_t override_big_position_kp : 1;
    uint32_t damping_forwardfeedback_en : 1;
    uint32_t interia_forwardfeedback_en : 1;

    HfiMethod hfi_method:3;
    ElecAngleSource elec_angle_source : 3;
    LoopWiring loop_wiring : 3;

    TrajSmoothMethod traj_smooth_method : 2;
    uint32_t auto_curve_retrack_en : 1;

    constexpr void reset(){
        *this = std::bit_cast<Self>(uint32_t(0));
    }

    static constexpr Self zero(){
        Self self;
        self.reset();
        return self;
    }

    static constexpr Self from_default(){
        return zero();
    }
};

static_assert(sizeof(FnSwitches) <= 4);


static constexpr auto SIDESHAFT_CALIBRATE_SWITCHES = []{
    FnSwitches switches;
    switches.reset();
    return switches;
}();




struct alignas(4) [[nodiscard]] DcCalibrateState{
    std::array<int32_t, 3> uvw_bvalue_offset_acc;
    std::array<int32_t, 3> uvw_bvalue_offset;

    size_t dc_cal_cnt;
    bool dc_cal_done;

    void reset(){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wclass-memaccess"
        memset(this, 0, sizeof(*this));
        #pragma GCC diagnostic pop
    }
};

struct alignas(4) [[nodiscard]] DebounceState final{
    uint32_t count;

    static constexpr uint32_t MAX_VALUE = 2000;
    constexpr bool is_positive() const {
        return count == MAX_VALUE;
    }

    constexpr void add_sample(const bool s){
        if(s){
            count = std::min(MAX_VALUE, count + 8);
        }else{
            count = uint32_t(std::max(0, int(count - 1)));
        }
    }
};

struct alignas(4) [[nodiscard]] DeadcompState final{
    alignas(4) std::array<int8_t, 3> uvw_sign;
    alignas(4) std::array<bool, 3> uvw_strong;
};

struct Temperature{
    iq16 celsius;

    constexpr iq16 to_celsius() const {
        return celsius;
    }
};

struct alignas(4) [[nodiscard]] HarmonicState final{
    iq20 id6c;
    iq20 iq6c;
    iq20 id6s;
    iq20 iq6s;

    std::array<iq20, 4> integrals;

    iq20 delta_vd6_in;
    iq20 delta_vq6_in;
};



struct alignas(4) [[nodiscard]] TemperatureState final{
    std::array<Temperature, 4> elements;

    auto & die(this auto && self){return self.elements[0];}
    auto & ext1(this auto && self){return self.elements[1];}
    auto & ext2(this auto && self){return self.elements[2];}
    auto & ext3(this auto && self){return self.elements[3];}
};


static constexpr size_t LG2_ENCODER_SIDESHAFT_EPS_TABLE_CAPACITY = 9;
static constexpr size_t ENCODER_SIDESHAFT_EPS_TABLE_CAPACITY = 1u << LG2_ENCODER_SIDESHAFT_EPS_TABLE_CAPACITY;

using EncoderClibrateEps = iq32;

enum class [[nodiscard]] EncoderNonlinearCalibrateStage:uint8_t{
    Start,
    // Idle,
    Ramp,
    // CurrentRise,
    Forward,
    Backward,
    Complete,
    Failed = 0x0f,
};

struct alignas(4) EncoderNonlinearCalibrateCounter{
    uint32_t bits;

    [[nodiscard]] constexpr auto stage(this auto && self) {
        return make_bitfield_proxy<28, 32, EncoderNonlinearCalibrateStage>(&self.bits);
    }

    // [[nodiscard]] constexpr auto specifier(this auto && self) {
    //     return make_bitfield_proxy<24, 28, uint8_t>(&self.bits);
    // }

    [[nodiscard]] constexpr auto count_value(this auto && self) {
        return make_bitfield_proxy<0, 28, uint32_t>(&self.bits);
    }
};


struct alignas(4) [[nodiscard]] EncoderCalibrateState{
    std::array<EncoderClibrateEps, ENCODER_SIDESHAFT_EPS_TABLE_CAPACITY> eps_table;



    uint32_t counter;
    uq32 cmd_mech_turns;
    iq20 torque_curr;
    // uint32_t step_count;

    struct alignas(4) [[nodiscard]] Debug{
        size_t index;
        uq32 frac;
        iq32 mech_eps_before;
        iq32 mech_eps_after;
    };

    Debug debug;
};


struct alignas(4) [[nodiscard]] FluxObserverState final{
    iq20 prev_x1;
    iq20 prev_x2;
    iq20 x1;
    iq20 x2;
    iq20 x1_slowlp;
    iq20 x2_slowlp;
    iq20 x1_slowhp;
    iq20 x2_slowhp;
    iq20 lem1;
    iq20 lem2;
    iq20 flux_err;
    iq20 abs_lem;
};

struct alignas(4) [[nodiscard]] CurveState final{
    iiq32 x1;
    iq20 x2;
    iq16 x3;
    iq16 u;
};

struct alignas(4) [[nodiscard]] TrajState final{
    iiq32 x1;
    iq20 x2;
    iq16 x3;
};


struct alignas(4) [[nodiscard]] HpState2o final{
    iiq32 x1;
    iq20 x2;
};

struct alignas(4) [[nodiscard]] PllState final{
    iiq32 x1;
    iq20 x2;
    iq20 x2_integral;
};


struct alignas(4) [[nodiscard]] ProctiveEncoderAnticoggingState final{
    iiq32 hat_turns;
    iq20 hat_speed;
    iq32 hat_b1;
    iq32 hat_b2;
    iq31 harm_c;
    iq31 harm_s;
    iq32 harm;

    struct Debug{
        uq32 harm_turns;
        iq24 e;
    };

    Debug debug;
};

struct alignas(4) [[nodiscard]] HfiState{
    size_t hfi_idx;
    alignas(4) bool hfi_is_neg_samp;

    iq20 hfi_response;
    Angular<uq16> prev_hfi_lap_angle2x;
    Angular<iq16> hfi_multilap_angle2x;

    iq20 pulsehfi_d_response;
    iq20 pulsehfi_q_response;

    iq20 spinhfi_bin0_real_response_acc;
    iq20 spinhfi_bin0_real_response;

    iq20 spinhfi_bin1_real_response;
    iq20 spinhfi_bin1_imag_response;

    iq20 spinhfi_bin2_real_response_acc;
    iq20 spinhfi_bin2_imag_response_acc;
    iq20 spinhfi_bin2_real_response;
    iq20 spinhfi_bin2_imag_response;
    iq20 spinhfi_bin2_real_response_slowlp;
    iq20 spinhfi_bin2_imag_response_slowlp;
};

struct alignas(4) [[nodiscard]] AllState{
    HpState2o encoder_ltd_state;
    PllState encoder_pll_state;
    CurveState curve_state;
    TrajState traj_smooth_state;
    TrajState traj_state;

    iq20 torque_curr_integral;
    iq20 torque_curr_cmd;
    iq20 torque_curr_x2comp;
    iq20 torque_curr_x3comp;
    iq20 torque_curr_veryslowlp;

    Angular<uq32> openloop_elec_angle;
    iq16 openloop_elec_speed;

    Angular<uq32> elec_angle;
    iq16 elec_speed;

    Angular<uq32> sensed_elec_angle;
    iq16 sensed_elec_speed;

    Angular<uq32> hfi_elec_angle;
    Angular<uq32> observer_elec_angle;

    iiq32 encoder_abs_position64;
    iiq32 encoder_rel_position64;
    uq32 encoder_initial_position_raw;
    uq32 encoder_initial_position;
    struct alignas(4) {
        bool is_encoder_initial_position_recorded;
        uint8_t encoder_correct_method_signature;
    };

    UvwCoord<iq20> uvw_curr_raw;
    UvwCoord<iq20> uvw_curr_ref;
    UvwCoord<iq20> uvw_curr_slowlp;
    UvwCoord<iq20> uvw_curr_fastlp;


    DqCoord<iq20> dq_curr_raw;
    DqCoord<iq20> dq_curr_ref;
    DqCoord<iq20> dq_curr_fastlp;
    
    DqCoord<iq20> dq5_curr_raw;
    DqCoord<iq20> dq7_curr_raw;

    DqCoord<iq20> dq5_curr_lp;
    DqCoord<iq20> dq7_curr_lp;

    HarmonicState harmonic_state;

    iq20 pi_ref_x2;
    iq20 pi_e2;
    iq20 mtpa_d_curr;
    iq20 mtpa_q_curr;
    iq20 backcalc_torque_curr;


    AlphaBetaCoord<iq20> alphabeta_curr_raw;
    AlphaBetaCoord<iq20> alphabeta_curr_ref;
    AlphaBetaCoord<iq20> prev_alphabeta_curr_raw;


    DqCoord<iq20> dq_volt_integral;
    DqCoord<iq20> dq_volt_ctrl;
    DqCoord<iq20> dq_volt_decouple;

    AlphaBetaCoord<iq20> alphabeta_dutycycle_hfi;
    AlphaBetaCoord<iq20> alphabeta_dutycycle_final;
    AlphaBetaCoord<iq20> alphabeta_volt_final;

    UvwCoord<iq16> uvw_dutycycle_deadcomp;
    UvwCoord<iq16> uvw_dutycycle_genout;
    DeadcompState deadcomp_state;

    DcCalibrateState dc_calibrate_state;
    iq20 busbar_curr_raw;
    iq20 busbar_curr_lp;


    iq20 unblance_curr_abs_lp;


    FluxObserverState flux_ob_state;

    HfiState hfi_state;
    dsp::PllState hfi_pll_state;
    dsp::PllState observer_pll_state;
    
    TemperatureState temperature_state;

    DebounceState u_disconn_dbs;
    DebounceState v_disconn_dbs;
    
    
    TimerTick isr_entry_tick;
    TimerTick encoder_get_done_tick;
    TimerTick isr_exit_tick;
    
    ProctiveEncoderAnticoggingState peac_state;
    uint32_t retrack_count;

    struct Debug{
    };

    Debug debug;

    EncoderCalibrateState encoder_calibrate_state;


    void reset(){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wclass-memaccess"
        memset(this, 0, sizeof(*this));
        #pragma GCC diagnostic pop
    }
};


}
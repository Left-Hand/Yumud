#pragma once

#include "drivers/encoder/magnetic/MT6825/mt6825.hpp"
#include "drivers/encoder/magnetic/VCE2755/vce2755.hpp"
#include "motor_leso.hpp"
#include "motor_dsp/dsp_pll.hpp"



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


enum class [[nodiscard]] ElecAngleSource:uint8_t{
    Openloop = 0,
    Hall,
    Hfi,
    MagEncoder,
    AbzEncoder
};

enum class [[nodiscard]] SenlessObserverMethod:uint8_t{
    Lbg,
    Smo,
    NlFlux
};

enum class [[nodiscard]] HfiMethod:uint8_t{
    
};

struct TimerTick{
    int16_t bits;
};


//不做温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_25C = 0;

//使用MCU结温进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_DIEJUNC = 1;

//使用外部温度输入1(通常为FET温度)进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_EXT1 = 2;

//使用外部温度输入2(通常为转子温度)进行温度补偿
static constexpr uint32_t RES_TEMP_COMPENSATE_SOURCE_EXT2 = 3;


static constexpr auto MOS_1C840L_500MA_BEST_DEADTIME_NS = 120ns;
// static constexpr auto MOS_1C840L_500MA_BEST_DEADTIME_NS = 400ns;
// static constexpr auto MOS_1C840L_500MA_BEST_DEADTIME_NS = 1000ns;

struct FnSwitches{
    using Self = FnSwitches;

    uint32_t initial_dc_calibrate_en : 1;
    uint32_t initial_encoder_dirtest_en : 1;
    uint32_t initial_tone_music : 1;
    uint32_t res_temp_compensate_source : 2;
    uint32_t deadtime_compensate_en : 1;
    uint32_t cross_decoupling_en : 1;
    uint32_t bemf_decoupling_en : 1;

    uint32_t hfi_en : 1;
    uint32_t flux_observer_en : 1;

    uint32_t fet_temperature_sensor_equipped : 1;
    uint32_t rotor_temperature_sensor_equipped : 1;
    uint32_t busbar_voltage_sensor_equipped : 1;
    uint32_t busbar_current_sensor_equipped : 1;

    constexpr void reset(){
        *this = std::bit_cast<Self>(uint32_t(0));
    }
};


struct OpFlags{
    using Self = OpFlags;


    uint32_t initial_dc_calibrate:1;
    uint32_t initial_encoder_dirtest:1;
    uint32_t measure_resind:1;
    uint32_t measure_flux:1;
    uint32_t encoder_calibrate:1;

    constexpr void reset(){
        *this = std::bit_cast<Self>(uint32_t(0));
    }

    constexpr bool any() const {
        return std::bit_cast<uint32_t>(*this) != 0;
    }
};



struct alignas(4) [[nodiscard]] DcCalibrateState{
    std::array<uint32_t, 3> uvw_current_bits_offset_acc;
    std::array<uint16_t, 3> uvw_current_bits_offset;

    bool dc_cal_done;
    size_t dc_cal_cnt;

    void reset(){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wclass-memaccess"
        memset(this, 0, sizeof(*this));
        #pragma GCC diagnostic pop
    }
};

struct DebounceState{
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

struct alignas(4) [[nodiscard]] AllState{
    DcCalibrateState dc_calibrate_state;

    SecondOrderState<iq16> track_ref;
    SecondOrderState<iq16> rotor_rotation_state_var;

    uq32 encoder_lap_turns;
    iiq32 encoder_multilap_turns;

    Angular<uq16> hfi_lap_angle;
    Angular<iq16> hfi_multilap_angle;

    Angular<uq32> openloop_elec_angle;
    Angular<uq32> encoder_elec_angle;
    Angular<uq32> hfi_elec_angle;
    Angular<uq32> observer_elec_angle;
    Angular<uq32> hybrid_elec_angle;
    Angular<uq32> selected_elec_angle;


    UvwCoord<iq20> uvw_curr_raw;
    UvwCoord<iq20> uvw_curr_slowlp;
    iq20 unblance_curr_abs_lp;
    DebounceState u_disconn_dbs;
    DebounceState v_disconn_dbs;

    DqCoord<iq20> dq_curr_raw;
    DqCoord<iq20> dq_curr_fastlp;

    AlphaBetaCoord<iq20> alphabeta_curr_raw;
    AlphaBetaCoord<iq20> alphabeta_curr_fastlp;
    AlphaBetaCoord<iq20> spinhfi_alphabeta_volt_gen;

    DqCoord<iq20> dq_volt_integral;
    DqCoord<iq20> dq_volt_gen;
    DqCoord<iq20> dq_volt_ff;

    // AlphaBetaCoord<iq20> hfi_alphabeta_volt;
    AlphaBetaCoord<iq20> alphabeta_volt_gen;
    UvwCoord<iq16> uvw_dutycycle_gen;
    AlphaBetaCoord<iq20> deadtime_comp_alphabeta_dutycycle;


    iq20 busbar_curr_raw;
    iq20 busbar_curr;
    iq20 torque_curr_cmd;


    iq20 hfi_response;
    iq20 hfi_bin0_real_response;
    iq20 hfi_bin1_real_response;
    iq20 hfi_bin1_imag_response;
    iq20 hfi_bin2_real_response;
    iq20 hfi_bin2_imag_response;
    iq20 hfi_bin2_real_response_slowlp;
    iq20 hfi_bin2_imag_response_slowlp;

    dsp::PllState hfi_pll_state;
    dsp::PllState obs_pll_state;



    TimerTick isr_entry_tick;
    TimerTick isr_exit_tick;
    TimerTick isr_elapsed_ticks;


    void reset(){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wclass-memaccess"
        memset(this, 0, sizeof(*this));
        #pragma GCC diagnostic pop
    }
};


}
#pragma once

#include <initializer_list>
#include "hal/sysmisc/nvic/nvic.hpp"
#include "adc_utils.hpp"

#if defined(ADC1_PRESENT) || defined(ADC2_PRESENT)
extern "C"{
__interrupt void ADC1_2_IRQHandler(void);
}
#endif


namespace ymd::hal{

class DmaChannel;



struct Adc_Prelude{
    using Callback = std::function<void(AdcEvent)>;


    using Pga = AdcPga;
    using RegularTrigger = AdcRegularTrigger;
    using InjectedTrigger = AdcInjectedTrigger;
    
    using ChannelSelection = AdcChannelSelection;
    using SampleCycles = AdcSampleCycles;
    using Mode = AdcMode;
    using IT = AdcIT;
};

struct [[nodiscard]] AdcChannelConfig{
    using ChannelSelection = AdcChannelSelection;
    using SampleCycles = AdcSampleCycles;

    ChannelSelection ch_sel;
    SampleCycles sample_cycles;
};

struct AdcIrqHandler{
    static void on_interrupt();
};


class AdcPrimary: public Adc_Prelude{
public:
    explicit AdcPrimary(void * inst):
        p_inst_(inst){;}

    struct Config{
        Mode mode = Mode::Independent;
    };

    void init(
        const std::initializer_list<AdcChannelConfig> & regular_list,
        const std::initializer_list<AdcChannelConfig> & injected_list, 
        const Config & cfg);


    void set_regular_channels(const std::initializer_list<AdcChannelConfig> & regular_list);
    void set_injected_channels(const std::initializer_list<AdcChannelConfig> & injected_list);

    void set_trigger(
        const RegularTrigger r_trigger, 
        const InjectedTrigger j_trigger
    ){
        set_regular_trigger(r_trigger);
        set_injected_trigger(j_trigger);
    }


    template<typename Fn>
    void set_isr_callback(Fn && cb){
        isr_callback_ = std::forward<Fn>(cb);
    }

    void register_nvic(const NvicPriorityCode priority, const Enable en){
        lld::nvic_set_irqn_priority(ADC_IRQn, priority);
        lld::nvic_enable_irqn(ADC_IRQn, en == EN);
    }

    template<IT I>
    void enable_interrupt(const Enable en){
        dyn_enable_interrupt(I, en);
    }


    void set_mode(const Mode mode);

    void set_pga(const Pga pga);

    void enable_continous(const Enable en);

    void enable_auto_inject(const Enable en);


    void enable_right_align(const Enable en);

    void set_regular_trigger(const RegularTrigger trigger);

    void set_injected_trigger(const InjectedTrigger trigger);

    void set_wdt_threshold(const uint16_t low,const uint16_t high);


    void sw_start_regular(const bool force = false);

    void sw_start_injected(const bool force = false);

    void enable_dma(const Enable en);

    uint16_t regular_conv_result();
    uint16_t injected_conv_result(const size_t rank);

protected:
    void * p_inst_;
    Callback isr_callback_;

    bool left_aligned_ = false;

    uint16_t cali_data_;

    [[nodiscard]] uint32_t get_max_value() const noexcept {
        // return ((1 << 12) - 1) << (left_aligned_ ? 4 : 4);
        if(left_aligned_) return 0xFFFF;
        else return 0x0FFF;
    }

    void set_regular_sample_cycles(const ChannelSelection sel, const SampleCycles sample_cycles);
    void enable_singleshot(const Enable en);
    void enable_scan(const Enable en);

    void enable_temp_vref(const Enable en);

    void dyn_enable_interrupt(const AdcIT I, const Enable en);


    #if defined(ADC1_PRESENT) || defined(ADC2_PRESENT)
    friend void ::ADC1_2_IRQHandler(void);
    #endif

    __fast_inline void isr_eoc(){
        if(isr_callback_ == nullptr) return;
        return isr_callback_(AdcEvent::EndOfInjectedConversion);
    }

    __fast_inline void isr_jeoc(){
        if(isr_callback_ == nullptr) return;
        return isr_callback_(AdcEvent::EndOfInjectedConversion);
    }

    __fast_inline void isr_awd(){
        if(isr_callback_ == nullptr) return;
        return isr_callback_(AdcEvent::AnalogWatchdog);
    }

    friend class AdcIrqHandler;
};


struct [[nodiscard]] TemperatureTrimer final{ 
    using Self = TemperatureTrimer;

    static constexpr uintptr_t REFER_VOLT_BASE = 0x1FFFF720;
    static constexpr float COEFF1 = (-3300.0 * 10 / 4096 / 43);
    static constexpr iq16 COEFF1_IQ16 = static_cast<iq16>(COEFF1);
    static constexpr float COEFF2 = (10.0 / 43);

    iq16 b;

    static imconstexpr Self load() {
        const uint32_t compressed_u32 = *reinterpret_cast<const volatile uint32_t*>(REFER_VOLT_BASE);
        const int32_t refer_volt = static_cast<int32_t>((compressed_u32) & 0xffff);
        const int32_t refer_temper = static_cast<int32_t>((compressed_u32) >> 16);
        
        return Self{
            .b = static_cast<iq16>(refer_temper) + static_cast<iq16>(COEFF2) * refer_volt
        };
    }
    
    constexpr iq16 parse_u12(const uint16_t x) const noexcept {
        constexpr uint16_t K = static_cast<uint16_t>(-COEFF1 * 65536);
        return iq16::from_bits(b.to_bits() - (K * x)); 
    }
};

}
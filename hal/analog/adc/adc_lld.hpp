#include "core/constants/enums.hpp"
#include "adc_primitive.hpp"
#include "core/utils/nth.hpp"

namespace ymd::lld{


void adc_set_regular_trigger(void * p_inst, const hal::AdcRegularTrigger trigger);

void adc_set_injected_trigger(void * p_inst, const hal::AdcInjectedTrigger trigger);

void adc_set_regular_quantity(void * p_inst, const uint8_t quantity);

void adc_set_injected_quantity(void * p_inst, const uint8_t quantity);

void adc_configure_regular_channel(void * p_inst, const uint8_t rank, const hal::AdcChannelConfig & cfg);

void adc_configure_injected_channel(void * p_inst, const uint8_t rank, const hal::AdcChannelConfig & cfg);

void adc_enable_auto_inject(void * p_inst, const Enable en);

void adc_enable_dma(void * p_inst, const Enable en);

void adc_enable_singleshot(void * p_inst, const Enable en);

void adc_enable_scan(void * p_inst, const Enable en);

void adc_enable_temp_vref(void * p_inst, const Enable en);


void adc_set_mode(void * p_inst, const hal::AdcMode mode);

void adc_set_pga(void * p_inst, const hal::AdcPga pga);

void adc_enable_continous(void * p_inst, const Enable en);

void adc_enable_right_align(void * p_inst, const Enable en);

void adc_cmd(void * p_inst, Enable en);

void adc_enable_buffer(void * p_inst, Enable en);

void adc_enable_rcc(const Nth adc_nth, Enable en);
}
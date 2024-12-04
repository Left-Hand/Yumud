#pragma once

#include "drivers/device_defs.h"

#ifdef XL2400_DEBUG
#define XL2400_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define XL2400_DEBUG(...)
#endif



namespace ymd::drivers{

class XL2400{
public:
    #include "regs.ipp"

    TopConfigReg top_config_reg;
    AutoAcknowledgeReg auto_ack_reg;
    EnableRxAddressReg enable_rx_address_reg;
    AddressWidthReg address_width_reg;
    AutoRetransmissionReg auto_retransmission_reg;
    RFChannelReg rf_channel_reg;
    RFSettingReg rf_setting_reg;
    StatusReg status_reg;
    TransmissionObservationReg transmission_observation_reg;
    RssiReg rssi_reg;
    RxAddrP0Reg rx_addr_p0_reg;
    RxAddrP1Reg rx_addr_p1_reg;
    RxAddrP2TopsReg rx_addr_p2_tops_reg;
    Bn9ResultReg bn9_result_reg;
    AGCSettingReg agc_setting_reg;
    TxAddrReg tx_addr_reg;
    RxPowerReg rx_power_reg;
    // AnalogConfig0Reg analog_config0_reg;
    // AnalogConfig1Reg analog_config1_reg;
    // AnalogConfig2Reg analog_config2_reg;
    // AnalogConfig3Reg analog_config3_reg;
    FifoStatusReg fifo_status_reg;
    RssiRecoderReg rssi_recoder_reg;
    TxProcessConfigReg tx_process_config_reg;
    RxProcessConfigReg rx_process_config_reg;
    FeatureReg feature_reg;
    PayloadLengthReg payload_length_reg;
    PARampConfigReg pa_ramp_config_reg;

protected:
    SpiDrv spi_drv;
public:
    XL2400(const SpiDrv & _spi_drv):spi_drv(_spi_drv){;}
    XL2400(SpiDrv && _spi_drv):spi_drv(_spi_drv){;}
};

}
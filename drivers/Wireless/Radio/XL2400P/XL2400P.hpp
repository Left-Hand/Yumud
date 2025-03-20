//这个驱动还在推进状态

#pragma once

#include "drivers/device_defs.h"
#include "core/utils/BitField.hpp"

namespace ymd::drivers{

class XL2400P{
public:
    using RegAddress = uint8_t;

    struct R8_TopConfig{
        scexpr RegAddress address = 0x01;

        uint8_t rx_on:1;
        uint8_t power_on:1;
        uint8_t crc_two_bytes:1;
        uint8_t crc_en : 1;

        uint8_t mask_max_rt:1;
        uint8_t mask_tx_ds:1;
        uint8_t mask_rx_dr:1;
        uint8_t loopback_en:1;

        uint8_t bps_gated_clk:1;
        uint8_t bps_idle_rst:1;
        uint8_t if_2m_sel:1;
        uint8_t addr_tx_opt:1;

        uint8_t whiten_opt:1;
        uint8_t dac_comp_out:1;
        uint8_t ce_soft:1;
        uint8_t ce_sel:1;

        uint8_t ce_pd:1;
        uint8_t adjust_freq_on_ack:1;
        uint8_t hardware_rc_cali_en:1;
        uint8_t pmu_en:1;
    };

    class R8_AutoAcknowledge:public Reg8<>{
        uint8_t data;
    public:
        auto p(const size_t n){return make_bfarray<0, 6, 6>(data);}
    };

    struct R8_EnableRxAddress:public Reg8<>{
        uint8_t data;
    public:
        scexpr RegAddress address = 0x02;
        auto p(const size_t n){return make_bfarray<0, 6, 6>(data);}
    };

    struct R8_AddressWidth:public Reg8<>{
        scexpr RegAddress address = 0x03;

        uint8_t pipex_address_width:2;
        uint8_t tx_address_width:2;
        uint8_t pillon_lock_time:4;
    };

    struct R8_AutoRetransmission{

        uint8_t retrans_times:4;

        uint8_t retrans_delay:4;
        uint8_t tx_time2_delay:4;
        uint8_t tx_time3_delay:4;
        uint8_t diag_sel:8;
        uint8_t diag_test_oe:1;
        uint8_t txframe_bypass:1;
        uint8_t redemod_bypass:1;
        uint8_t gpio_i_inv:1;
        uint8_t gpio_config:2;
    };

    struct R8_RFChannel{
        uint16_t rx_channel:14;

        uint8_t __resv__ :2;
        uint8_t rf_power:6;
    };

    struct R8_RFSetting{
        uint8_t cyclix_pattern_tx_enable:1;
        uint8_t __resv__ :2;
        uint8_t rf_datarate_high_bit:1;
        uint8_t tx_attenuation_level:1;
        uint8_t rf_datarate_low_bit:1;
        uint8_t pn9_bitstream_en:1;
        uint8_t cont_carrier_en:1;
        uint8_t tx_pattern:8;
    };

    struct R8_Status{
        uint8_t tx_full:1;
        uint8_t rx_pipe_number:2;
        uint8_t max_rt:1;
        uint8_t tx_ds:1;
        uint8_t rx_dr:1;
        uint8_t __resv__ :1;
    };

    struct R8_TransmissionObservation{
        uint8_t arc_cnt:4;
        uint8_t plos_cnt:4;
        uint8_t dc_offset_i:8;
        uint8_t dc_offset_q:8;
        uint8_t freq_offset:8;
    };

    struct R8_Rssi{
        uint8_t rssi1:1;
        uint8_t rssi2:1;
        uint8_t __resv1__:2;
        uint8_t rssi_en:1;
        uint8_t __resv2__:3;
        uint8_t rssidb_offset:6;
    };

    using R8_RxAddrP0 = uint64_t;
    using R8_RxAddrP1 = uint64_t;

    struct R8_RxAddrP2Tops{//0x0c
        uint8_t rx_addr_p2:8;
        uint8_t rx_addr_p3:8;
        uint8_t rx_addr_p4:8;
        uint8_t rx_addr_p5:8;
    };

    struct R8_Bn9Result{
        uint32_t total_bit_counter:32;
        uint32_t err_bit_counter:32;
    };

    struct R8_AGCSetting{
        uint8_t agc_gain_1:7;
        uint8_t agc_gain_2:7;
        uint8_t agc_gain_3:7;
        uint8_t agc_gain_4:7;
        uint8_t agc_gain_5:7;
    };

    using R8_TxAddr = uint64_t;

    struct R8_RxPower{
        uint8_t rx_power_p0:6;
        uint8_t __resv1__:2;
        uint8_t rx_power_p1:6;
        uint8_t __resv2__:2;
        uint8_t rx_power_p2:6;
        uint8_t __resv3__:3;
        uint8_t rx_power_p3:6;
        uint8_t __resv4__:2;
        uint8_t rx_power_p4:6;
        uint8_t __resv5__:2;
        uint8_t rx_power_p5:6;
        uint8_t __resv6__:2;
    };

    // using R8_AnalogConfig0 = uint128;
    // using R8_AnalogConfig1 = uint128_t;
    // using R8_AnalogConfig2 = __uint128_t;
    // using R8_AnalogConfig3 = __uint128_t;


    struct R8_FifoStatus{
        uint8_t rx_empty:1;
        uint8_t rx_full:1;
        uint8_t pend_rxfrm_num_l:2;
        uint8_t tx_empty:1;
        uint8_t tx_full:1;
        uint8_t txreuse:1;
        uint8_t pend_rxfrm_num_h:2;
        uint8_t ad_rccal_ctrim:5;
        uint8_t ad_rccal_finish:1;
        uint8_t hw_rc_cali_done;
        uint8_t R8_bb_ana3_7t3:5;
    };

    struct R8_RssiRecoder{
        uint8_t rssirec1:8;
        uint8_t rssirec2:8;
        uint8_t rssi1x_vref_sel:3;
        uint8_t rssi2_vref_sel:3;
        uint8_t rssi_dmb:8;
        uint8_t __resv__:2;
    };


    struct R8_TxProcessConfig{
        uint8_t     freq_dev:8;
        uint8_t        gasflt_bt_sel:1;
        uint8_t        gasflt_bps:1;
        uint16_t    kmod_set:9;
        uint8_t        kmod_bps:1;
    };

    struct R8_RxProcessConfig {
        uint8_t rx_iq_swap : 1;        // IF ADC data IQ swap
        uint8_t adc_sample_pha : 1;    // IF ADC data sample edge select; 1: invert
        uint8_t pre_dc_manu : 1;    // Freq offset manual setting enable
        uint8_t ipl_bps : 1;         // Bypass IPLS
        uint8_t chan_flt_bps : 1;    // Bypass channel filter
        uint8_t sync_wind_cfg : 6;   // Syncword search windows
        uint8_t pre_dc_wind : 2;     // Freq offset calculate windows
        uint8_t max_freq : 8;        // Max Freq set for IPLS
        uint8_t pre_dc_set : 8;      // Freq offset manual set
        uint32_t rx_dem_start_dl : 8; // RX Start Delay counter at 16MHz
        uint8_t rx_dem_start_cf : 1;    // Enable RX Start Delay
    };

    struct R8_Feature {
        uint8_t en_dyn_ack : 1;          // Set 1 enables the W_TX_PAYLOAD_NOACK command
        uint8_t en_ack_pay : 1;          // Set 1 enables payload on ACK
        uint8_t en_dpl : 1;              // Set 1 enables dynamic payload length
        uint8_t en_whiten : 1;           // Set 1 enable whithen feature
        uint8_t en_fec : 1;              // Set 1 enable FEC & Interleave feature
        uint8_t en_long_pld : 1;         // Set 1 enables long payload feature max length is 128Byte
        uint8_t stat_setup : 2;       // Adjust the output of SDO during command input
    };

    struct R8_PayloadLength{
        uint8_t dpl_p0:1;
        uint8_t dpl_p1:1;
        uint8_t dpl_p2:1;
        uint8_t dpl_p3:1;
        uint8_t dpl_p4:1;
        uint8_t dpl_p5:1;
        uint8_t __resv__:2;
    };


    struct R8_PARampConfig{
        uint8_t ramp_time:3;
        uint8_t ramp_1:5;
        uint8_t ramp_2:5;
        uint8_t ramp_3:5;
        uint8_t ramp_4:5;
        uint8_t ramp_5:5;
        uint8_t ramp_6:5;
        uint8_t ramp_7:5;
        uint8_t ramp_8:5;
        uint8_t ramp_9:5;
        uint8_t ramp_10:5;
        uint8_t ramp_11:5;
        uint8_t ramp_12:5;
        uint8_t ramp_13:5;
        uint8_t ramp_14:5;
    };

    DEF_R8(R8_TopConfig, top_config_reg)
    DEF_R8(R8_AutoAcknowledge, auto_ack_reg)
    DEF_R8(R8_EnableRxAddress, enable_rx_address_reg)
    DEF_R8(R8_AddressWidth, address_width_reg)
    DEF_R8(R8_AutoRetransmission, auto_retransmission_reg)
    DEF_R8(R8_RFChannel, rf_channel_reg)
    DEF_R8(R8_RFSetting, rf_setting_reg)
    DEF_R8(R8_Status, status_reg)
    DEF_R8(R8_TransmissionObservation, transmission_observation_reg)
    DEF_R8(R8_Rssi, rssi_reg)
    DEF_R8(R8_RxAddrP0, rx_addr_p0_reg)
    DEF_R8(R8_RxAddrP1, rx_addr_p1_reg)
    DEF_R8(R8_RxAddrP2Tops, rx_addr_p2_tops_reg)
    DEF_R8(R8_Bn9Result, bn9_result_reg)
    DEF_R8(R8_AGCSetting, agc_setting_reg)
    DEF_R8(R8_TxAddr, tx_addr_reg)
    DEF_R8(R8_RxPower, rx_power_reg)
    DEF_R8(R8_FifoStatus, fifo_status_reg)
    DEF_R8(R8_RssiRecoder, rssi_recoder_reg)
    DEF_R8(R8_TxProcessConfig, tx_process_config_reg)
    DEF_R8(R8_RxProcessConfig, rx_process_config_reg)
    DEF_R8(R8_Feature, feature_reg)
    DEF_R8(R8_PayloadLength, payload_length_reg)
    DEF_R8(R8_PARampConfig, pa_ramp_config_reg)

protected:
    hal::SpiDrv spi_drv;
public:
    XL2400P(const hal::SpiDrv & _spi_drv):spi_drv(_spi_drv){;}
    XL2400P(hal::SpiDrv && _spi_drv):spi_drv(_spi_drv){;}
};

}
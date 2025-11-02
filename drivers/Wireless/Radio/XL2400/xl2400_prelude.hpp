#pragma once

#include "core/io/regs.hpp"
#include "core/utils/strong_type_gradation.hpp"
#include "core/stream/ostream.hpp"

namespace ymd::drivers{
struct XL2400_Address{
    using Self = XL2400_Address;

    std::array<uint8_t, 5> bytes;

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        auto guard = os.create_guard();
        os << std::hex << 
            self.bytes[0] << ':' << 
            self.bytes[1] << ':' << 
            self.bytes[2] << ':' << 
            self.bytes[3] << ':' << 
            self.bytes[4];
    }
};

struct XL2400_Prelude{


    /**
     * REGISTER TABLE
     */
    /****************** SPI  REGISTER  ********************/
    static constexpr  uint8_t XL2400_CMD_R_REGISTER    = 0x00; // [000A AAAA] Register read
    static constexpr  uint8_t XL2400_CMD_W_REGISTER    = 0x20; // [001A AAAA] Register write
    static constexpr  uint8_t XL2400_CMD_R_RX_PAYLOAD  = 0x61; // Read RX payload
    static constexpr  uint8_t XL2400_CMD_W_TX_PAYLOAD  = 0xA0; // Write TX payload
    static constexpr  uint8_t XL2400_CMD_FLUSH_TX      = 0xE1; // Flush TX FIFO
    static constexpr  uint8_t XL2400_CMD_FLUSH_RX      = 0xE2; // Flush RX FIFO
    static constexpr  uint8_t XL2400_CMD_REUSE_TX_PL   = 0xE3; // Reuse TX Payload
    static constexpr  uint8_t XL2400_CMD_ACTIVATE      = 0x50; // ACTIVATE
    static constexpr  uint8_t XL2400_CMD_DEACTIVATE    = 0x50; // DEACTIVATE
    static constexpr  uint8_t XL2400_CMD_RST_FSPI      = 0x53; // RESET
    static constexpr  uint8_t XL2400_CMD_R_RX_PL_WID   = 0x60; // Read width of RX data 
    static constexpr  uint8_t XL2400_CMD_W_ACK_PAYLOAD = 0xA8; // Data with ACK
    static constexpr  uint8_t XL2400_CMD_W_TX_PAYLOAD_NOACK = 0xB0; // TX Payload no ACK Request
    static constexpr  uint8_t XL2400_CMD_NOP           = 0xFF; // No operation (used for reading status register)

    /******************CONTROL  REGISTER ******************/
    static constexpr  uint8_t XL2400_REG_CFG_TOP       = 0x00; // Configuration register, 20 bits
    static constexpr  uint8_t XL2400_REG_EN_AA         = 0x01; // Enable "Auto acknowledgment"
    static constexpr  uint8_t XL2400_REG_EN_RXADDR     = 0x02; // Enable RX addresses
    static constexpr  uint8_t XL2400_REG_SETUP_AW      = 0x03; // Setup of address widths
    static constexpr  uint8_t XL2400_REG_SETUP_RETR    = 0x04; // Setup of automatic re-transmit, 30 bits
    static constexpr  uint8_t XL2400_REG_RF_CH         = 0x05; // RF channel, 22 bits
    static constexpr  uint8_t XL2400_REG_RF_SETUP      = 0x06; // RF setup, 16 bits
    static constexpr  uint8_t XL2400_REG_STATUS        = 0x07; // Status
    static constexpr  uint8_t XL2400_REG_OBSERVE_TX    = 0x08; // Transmit observe register, 32 bits
    static constexpr  uint8_t XL2400_REG_RSSI          = 0x09; // Data output and RSSI, 14 bits
    static constexpr  uint8_t XL2400_REG_RX_ADDR_P0    = 0x0A; // Receive address data pipe 0, 40 bits
    static constexpr  uint8_t XL2400_REG_RX_ADDR_P1    = 0x0B; // Receive address data pipe 1, 40 bits
    static constexpr  uint8_t XL2400_REG_RX_ADDR_P2_P5 = 0x0C; // Receive address data pipe 2~5, 32 bits
    static constexpr  uint8_t XL2400_REG_BER_RESULT    = 0x0D; // BER(PN9) test result, 64 bits
    static constexpr  uint8_t XL2400_REG_AGC_SETTING   = 0x0E; // AGC settings, 32 bits
    static constexpr  uint8_t XL2400_REG_PGA_SETTING   = 0x0F; // PGA settings, 39 bits
    static constexpr  uint8_t XL2400_REG_TX_ADDR       = 0x10; // Transmit address, 40 bits
    static constexpr  uint8_t XL2400_REG_RX_PW_PX      = 0x11; // Number of bytes in RX payload in data pipe 0 ~ pipe 5, 48 bits
    static constexpr  uint8_t XL2400_REG_ANALOG_CFG0   = 0x12; // Analog config 0, 128 bits
    static constexpr  uint8_t XL2400_REG_ANALOG_CFG1   = 0x13; // Analog config 1, 128 bits
    static constexpr  uint8_t XL2400_REG_ANALOG_CFG2   = 0x14; // Analog config 2, 128 bits
    static constexpr  uint8_t XL2400_REG_ANALOG_CFG3   = 0x15; // Analog config 3, 128 bits
    static constexpr  uint8_t XL2400_REG_FIFO_STATUS   = 0x17; // FIFO status, 20 bits
    static constexpr  uint8_t XL2400_REG_RSSIREC       = 0x18; // RSSI recorder feature, 32 bits
    static constexpr  uint8_t XL2400_REG_TXPROC_CFG    = 0x19; // TX Process configuration, 29 bits
    static constexpr  uint8_t XL2400_REG_RXPROC_CFG    = 0x1A; // RX Process configuration, 40 bits
    static constexpr  uint8_t XL2400_REG_DYNPD         = 0x1C; // Enable dynamic payload length
    static constexpr  uint8_t XL2400_REG_FEATURE       = 0x1D; // Feature config
    static constexpr  uint8_t XL2400_REG_RAMP_CFG      = 0x1E; // PA Ramp Configuration, 88 bits


    /**************************** CONFIGs ************************************/

    static constexpr  uint8_t XL2400_PL_WIDTH_MAX      = 64;   // Max payload width
    static constexpr  uint8_t XL2400_RF_10DB           = 0x3F;
    static constexpr  uint8_t XL2400_RF_9DB            = 0x38;
    static constexpr  uint8_t XL2400_RF_8DB            = 0x34;
    static constexpr  uint8_t XL2400_RF_7DB            = 0x30;
    static constexpr  uint8_t XL2400_RF_6DB            = 0x2C; // 250Kbps Maximum
    static constexpr  uint8_t XL2400_RF_5DB            = 0x28;
    static constexpr  uint8_t XL2400_RF_4DB            = 0x24;
    static constexpr  uint8_t XL2400_RF_3DB            = 0x20;
    static constexpr  uint8_t XL2400_RF_2DB            = 0x14;
    static constexpr  uint8_t XL2400_RF_0DB            = 0x10; // 1Mbps Maximum
    static constexpr  uint8_t XL2400_RF__2DB           = 0x0C;
    static constexpr  uint8_t XL2400_RF__6DB           = 0x08;
    static constexpr  uint8_t XL2400_RF__12DB          = 0x04;
    static constexpr  uint8_t XL2400_RF__18DB          = 0x02;
    static constexpr  uint8_t XL2400_RF__24DB          = 0x01;
    static constexpr  uint8_t XL2400_RF_DR_2M          = 0x08; // 2Mbps
    static constexpr  uint8_t XL2400_RF_DR_1M          = 0x00; // 1Mbps
    static constexpr  uint8_t XL2400_RF_DR_250K        = 0x20; // 250Kbps
    static constexpr  uint8_t XL2400_RF_DR_125K        = 0x28; // 125Kbps

    static constexpr  uint8_t XL2400_FLAG_RX_DR        = 0X40;   // Data ready
    static constexpr  uint8_t XL2400_FLAG_TX_DS        = 0X20;   // Data sent
    static constexpr  uint8_t XL2400_FLAG_RX_TX_CMP    = 0X60;   // Data sent & acked
    static constexpr  uint8_t XL2400_FLAG_MAX_RT       = 0X10;   // Max retried
    static constexpr  uint8_t XL2400_FLAG_TX_FULL      = 0x01; // 1:TX FIFO full
};
struct XL2400_Regset:public XL2400_Prelude{
    using R8_RxAddrP0 = uint64_t;
    using R8_RxAddrP1 = uint64_t;
    using R8_TxAddr = uint64_t;
    struct R32_TopConfig{
        uint32_t rx_on:1;
        uint32_t power_on:1;

        // CRC scheme
        // 0: 1 byte, 1: 2 bytes
        uint32_t crc_two_bytes:1;

        // Enable CRC.Forced high if any of
        // the bits in EN_AA is high
        uint32_t crc_en : 1;

        // Mask interrupt caused by MAX_RT; 
        // 0: interrupt not reflected on IRQ pin;
        // 1: reflect MAX_RT as active low interrupt
        // on IRQ pin
        uint32_t mask_max_rt:1;
        uint32_t mask_tx_ds:1;
        uint32_t mask_rx_dr:1;
        uint32_t loopback_en:1;

        uint32_t bps_gated_clk:1;
        uint32_t bps_idle_rst:1;
        uint32_t if_2m_sel:1;
        uint32_t addr_tx_opt:1;

        uint32_t whiten_opt:1;
        uint32_t dac_comp_out:1;
        uint32_t ce_soft:1;
        uint32_t ce_sel:1;

        uint32_t ce_pd:1;
        uint32_t adjust_freq_on_ack:1;
        uint32_t hardware_rc_cali_en:1;
        uint32_t pmu_en:1;
    }DEF_R32(top_config_reg)

    struct R8_AutoAcknowledge:public Reg8<>{
        uint8_t p0:1;
        uint8_t p1:1;
        uint8_t p2:1;
        uint8_t p3:1;
        uint8_t p4:1;
        uint8_t p5:1;
        uint8_t __resv__ :2;
    }DEF_R8(auto_acknowledge_reg)

    struct R8_EnableRxAddress:public Reg8<>{
        uint8_t p0:1;
        uint8_t p1:1;
        uint8_t p2:1;
        uint8_t p3:1;
        uint8_t p4:1;
        uint8_t p5:1;
        uint8_t __resv__ :2;
    }DEF_R8(enable_rx_address_reg)

    enum class PrxAddressWidth:uint8_t{
        _3B = 1,
        _4B = 2,
        _5B = 3 
    };

    enum class PtxAddressWidth:uint8_t{
        _3B = 1,
        _4B = 2,
        _5B = 3 
    };

    

    struct R8_AddressWidth:public Reg8<>{
        PrxAddressWidth pipex_address_width:2;
        PrxAddressWidth tx_address_width:2;
        uint8_t pillon_lock_time:4;
    };

    struct R32_AutoRetransmission{
        // 0000: disabled
        // 0001: up to 1 re-transmit on fail of AA
        // ...
        // 1111: up to 15 re-transmits on fail
        // of AA
        uint32_t retrans_times:4;

        // 0000: wait 250uS
        // 0001: wait 500uS
        // ...
        // 1111: wait 4000uS
        uint32_t retrans_delay:4;


        uint32_t tx_time2_delay:4;
        uint32_t tx_time3_delay:4;

        uint32_t diag_sel:8;
        uint32_t diag_test_oe:1;
        uint32_t txframe_bypass:1;
        uint32_t redemod_bypass:1;
        uint32_t gpio_i_inv:1;
        uint32_t gpio_config:2;
    };

    struct R32_RFChannel{
        // Set frequency channel in 1 MHz
        // increment, 0x962 is 2402MHz
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

    struct R32_TransmissionObservation{
        uint8_t arc_cnt:4;
        uint8_t plos_cnt:4;
        uint8_t dc_offset_i;
        uint8_t dc_offset_q;
        uint8_t freq_offset;
    };

    struct R8_Rssi{
        uint8_t rssi1:1;
        uint8_t rssi2:1;
        uint8_t __resv1__:2;
        uint8_t rssi_en:1;
        uint8_t __resv2__:3;
        uint8_t rssidb_offset:6;
    };



    struct R32_RxAddrP2Tops{//0x0c
        uint8_t rx_addr_p2;
        uint8_t rx_addr_p3;
        uint8_t rx_addr_p4;
        uint8_t rx_addr_p5;
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

    // DEF_R8(R32_TopConfig, top_config_reg)
    // DEF_R8(R8_AutoAcknowledge, auto_ack_reg)
    // DEF_R8(R8_EnableRxAddress, enable_rx_address_reg)
    // DEF_R8(R8_AddressWidth, address_width_reg)
    // DEF_R8(R8_AutoRetransmission, auto_retransmission_reg)
    // DEF_R8(R8_RFChannel, rf_channel_reg)
    // DEF_R8(R8_RFSetting, rf_setting_reg)
    // DEF_R8(R8_Status, status_reg)
    // DEF_R8(R8_TransmissionObservation, transmission_observation_reg)
    // DEF_R8(R8_Rssi, rssi_reg)
    // DEF_R8(R8_RxAddrP0, rx_addr_p0_reg)
    // DEF_R8(R8_RxAddrP1, rx_addr_p1_reg)
    // DEF_R8(R8_RxAddrP2Tops, rx_addr_p2_tops_reg)
    // DEF_R8(R8_Bn9Result, bn9_result_reg)
    // DEF_R8(R8_AGCSetting, agc_setting_reg)
    // DEF_R8(R8_TxAddr, tx_addr_reg)
    // DEF_R8(R8_RxPower, rx_power_reg)
    // DEF_R8(R8_AnalogConfig0, analog_config0_reg)
    // DEF_R8(R8_AnalogConfig1, analog_config1_reg)
    // DEF_R8(R8_AnalogConfig2, analog_config2_reg)
    // DEF_R8(R8_AnalogConfig3, analog_config3_reg)
    // DEF_R8(R8_FifoStatus, fifo_status_reg)
    // DEF_R8(R8_RssiRecoder, rssi_recoder_reg)
    // DEF_R8(R8_TxProcessConfig, tx_process_config_reg)
    // DEF_R8(R8_RxProcessConfig, rx_process_config_reg)
    // DEF_R8(R8_Feature, feature_reg)
    // DEF_R8(R8_PayloadLength, payload_length_reg)
    // DEF_R8(R8_PARampConfig, pa_ramp_config_reg)

};


}
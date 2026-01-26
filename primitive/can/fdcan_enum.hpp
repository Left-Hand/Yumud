#pragma once

namespace ymd::hal{

#if 0
struct FdCanConfig final {
    // Nominal Bit Timings
    CanNominalBitTimming nbtr;
    // (Variable) Data Bit Timings
    DataBitTiming dbtr;
    // Enables or disables automatic retransmission of messages

    // If this is enabled, the CAN peripheral will automatically try to retransmit each frame util it can be sent. Otherwise, it will try only once to send each frame.

    // Automatic retransmission is enabled by default.
    Enable automatic_retransmit;

    // The transmit pause feature is intended for use in CAN systems where the CAN message 
    // identifiers are permanently specified to specific values and cannot easily be changed.

    // These message identifiers can have a higher CAN arbitration priority than other defined messages, 
    // while in a specific application their relative arbitration priority must be inverse.

    // This may lead to a case where one ECU sends a burst of CAN messages that cause another ECU CAN 
    // messages to be delayed because that other messages have a lower CAN arbitration priori
    Enable transmit_pause;

    // Enabled or disables the pausing between transmissions
    // This feature looses up burst transmissions coming from a single node and it protects against 
    // “babbling idiot” scenarios where the application program erroneously requests too many transmissions.
    FrameTransmissionConfig frame_transmit;

    // Non Isoe Mode If this is set, the FDCAN uses the CAN FD frame format as specified by the 
    // Bosch CAN FD Specification V1.0.
    Enable non_iso_mode;

    // Edge Filtering: Two consecutive dominant tq required to detect an edge for hard synchronization
    Enable edge_filtering;

    // Enables protocol Error handling
    Enable protocol_Error_handling;

    // Sets the general clock divider for this FdCAN instance
    ClockDivider clock_divider;

    // Sets the timestamp source
    TimestampSource timestamp_source;

    // Configures the Global Filter
    GlobalFilter global_filter;

    // TX buffer mode (FIFO or priority queue)
    TxBufferMode tx_buffer_mode;
};
#endif
}
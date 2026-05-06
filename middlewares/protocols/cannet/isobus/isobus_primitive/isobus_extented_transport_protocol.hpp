#pragma once

#include <cstdint>


// https://github.com/Open-Agriculture/AgIsoStack-plus-plus/blob/main/isobus/include/isobus/isobus/can_extended_transport_protocol.hpp

namespace ymd::isobus
{
/// @brief The states that a ETP session could be in. Used for the internal state machine.
enum class StateMachineState
{
    None, ///< Protocol session is not in progress
    SendRequestToSend, ///< We are sending the request to send message
    WaitForClearToSend, ///< We are waiting for a clear to send message
    SendClearToSend, ///< We are sending clear to send message
    WaitForDataPacketOffset, ///< We are waiting for a data packet offset message
    SendDataPacketOffset, ///< We are sending a data packet offset message
    WaitForDataTransferPacket, ///< We are waiting for data transfer packets
    SendDataTransferPackets, ///< A Tx data session is in progress
    WaitForEndOfMessageAcknowledge, ///< We are waiting for an end of message acknowledgement
};

/// @brief A list of all defined abort reasons in ISO11783
enum class ConnectionAbortReason : std::uint8_t
{
    Reserved = 0, ///< Reserved, not to be used, but should be tolerated
    AlreadyInCMSession = 1, ///< We are already in a connection mode session and can't support another
    SystemResourcesNeeded = 2, ///< Session must be aborted because the system needs resources
    Timeout = 3, ///< General timeout
    ClearToSendReceivedWhileTransferInProgress = 4, ///< A CTS was received while already processing the last CTS
    MaximumRetransmitRequestLimitReached = 5, ///< Maximum retries for the data has been reached
    UnexpectedDataTransferPacketReceived = 6, ///< A data packet was received outside the proper state
    BadSequenceNumber = 7, ///< Incorrect sequence number was received and cannot be recovered
    DuplicateSequenceNumber = 8, ///< Re-received a sequence number we've already processed
    UnexpectedDataPacketOffsetReceived = 9, // Received a data packet offset outside the proper state
    UnexpectedDataPacketOffsetPGN = 10, ///< Received a data packet offset with an unexpected PGN
    DataPacketOffsetExceedsClearToSend = 11, ///< Received a number of packets in EDPO greater than CTS
    BadDataPacketOffset = 12, ///< Received a data packet offset that is incorrect
    UnexpectedClearToSendPGN = 14, ///< Received a CTS with an unexpected PGN
    NumberOfClearToSendPacketsExceedsMessage = 15, ///< Received a CTS with a number of packets greater than the message
    AnyOtherError = 250 ///< Any reason not defined in the standard
};

} // namespace isobus

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

//参考资料：
// (BSD License)
// https://github.com/adafruit/Adafruit-PN532/tree/master

// Apache License
// https://github.com/WMT-GmbH/pn532


namespace ymd::drivers{

struct PN532_Prelude{

enum class [[nodiscard]] ErrorCode:uint8_t {
    /// Time Out, the target has not answered
    Timeout = 0x01,
    /// A CRC error has been detected by the CIU
    CrcError = 0x02,
    /// A Parity error has been detected by the CIU
    ParityError = 0x03,
    /// During an anti-collision/select operation (ISO/IEC14443-3
    /// Type A and ISO/IEC18092 106 kbps passive mode), an
    /// erroneous Bit Count has been detected
    WrongBitCountDuringAntiCollision = 0x04,
    /// Framing error during Mifare operation
    FramingError = 0x05,
    /// An abnormal bit-collision has been detected during bit wise
    /// anti-collision at 106 kbps
    AbnormalBitCollision = 0x06,
    /// Communication buffer size insufficient
    InsufficientCommunicationBuffer = 0x07,
    /// RF Buffer overflow has been detected by the CIU (bit
    /// BufferOvfl of the register CIU_Error)
    RfBufferOverflow = 0x09,
    /// In active communication mode, the RF field has not been
    /// switched on in time by the counterpart (as defined in NFCIP-1
    /// standard)
    RfFieldHasNotBeenSwitchedOn = 0x0A,
    /// RF Protocol error (cf. Error! Reference source not found.,
    /// description of the CIU_Error register)
    RfProtocolError = 0x0B,
    /// Temperature error: the internal temperature sensor has
    /// detected overheating, and therefore has automatically
    /// switched off the antenna drivers
    Overheating = 0x0D,
    /// Internal buffer overflow
    InternalBufferOverflow = 0x0E,
    /// Invalid parameter (range, format, …)
    InvalidParameter = 0x10,
    /// DEP Protocol: The PN532 configured in target mode does not
    /// support the command received from the initiator (the
    /// command received is not one of the following: ATR_REQ,
    /// WUP_REQ, PSL_REQ, DEP_REQ, DSL_REQ, RLS_REQ
    /// Error! Reference source not found.).
    CommandNotSupported = 0x12,
    /// DEP Protocol, Mifare or ISO/IEC14443-4: The data format
    /// does not match to the specification.
    /// Depending on the RF protocol used, it can be:
    /// • Bad length of RF received frame,
    /// • Incorrect value of PCB or PFB,
    /// • Invalid or unexpected RF received frame,
    /// • NAD or DID incoherence.
    WrongDataFormat = 0x13,
    /// Mifare: Authentication error
    AuthenticationError = 0x14,
    /// ISO/IEC14443-3: UID Check byte is wrong
    WrongUidCheckByte = 0x23,
    /// DEP Protocol: Invalid device state, the system is in a state
    /// which does not allow the operation
    InvalidDeviceState = 0x25,
    /// Operation not allowed in this configuration (host controller
    /// interface)
    OperationNotAllowed = 0x26,
    /// This command is not acceptable due to the current context of
    /// the PN532 (Initiator vs. Target, unknown target number,
    /// Target not in the good state, …)
    CommandNotAcceptable = 0x27,
    /// The PN532 configured as target has been released by its
    /// initiator
    TargetHasBeenReleased = 0x29,
    /// PN532 and ISO/IEC14443-3B only: the ID of the card does
    /// not match, meaning that the expected card has been
    /// exchanged with another one.
    CardHasBeenExchanged = 0x2A,
    /// PN532 and ISO/IEC14443-3B only: the card previously
    /// activated has disappeared.
    CardHasDisappeared = 0x2B,
    /// Mismatch between the NFCID3 initiator and the NFCID3
    /// target in DEP 212/424 kbps passive.
    NfcId3InitiatorTargetMismatch = 0x2C,
    /// An over-current event has been detected
    OverCurrent = 0x2D,
    /// NAD missing in DEP frame
    NadMissing = 0x2E,
};


/// Commands supported by the Pn532
///
/// These commands are fully described in the section 7 of the User Manual:
/// <https://www.nxp.com/docs/en/user-guide/141520.pdf>
enum class [[nodiscard]] Command : uint8_t {
    /// This command is used for self-diagnosis. Processing time of this command varies depending
    /// on the content of the processing.
    ///
    /// For more information, see 7.2.1 Diagnose
    Diagnose = 0x00,
    /// This command is used to get the version of the embedded firmware from PN532.
    ///
    /// For more information, see 7.2.2 GetFirmwareVersion
    GetFirmwareVersion = 0x02,
    /// This command allows the host controller to know at a given moment the complete situation of
    /// the PN532.
    ///
    /// For more information, see 7.2.3 GetGeneralStatus
    GetGeneralStatus = 0x04,
    /// This command is used to read the content of one or several internal registers of the PN532.
    ///
    /// For more information, see 7.2.4 ReadRegister
    ReadRegister = 0x06,
    /// This command is used to overwrite the content of one or several internal registers of the
    /// PN532.
    ///
    /// For more information, see 7.2.5 WriteRegister
    WriteRegister = 0x08,
    /// Tells the PN532 to read the value for each GPIO port and return the information to the host
    /// controller.
    ///
    /// For more information, see 7.2.6 ReadGPIO
    ReadGPIO = 0x0C,
    /// Tells the PN532 to apply the value for each port specified by the host controller.
    ///
    /// For more information, see 7.2.7 WriteGPIO
    WriteGPIO = 0x0E,
    /// Selects the baud rate on the serial link between the host controller and the PN532.
    ///
    /// For more information, see 7.2.8 SetSerialBaudRate
    SetSerialBaudRate = 0x10,
    /// This command is used to set internal parameters of the PN532, and then to configure its
    /// behavior regarding different cases.
    ///
    /// For more information, see 7.2.9 SetParameters
    SetParameters = 0x12,
    /// This command is used to select the data flow path by configuring the internal serial data
    /// switch.
    ///
    /// For more information, see 7.2.10 SAMConfiguration
    SAMConfiguration = 0x14,
    /// This command can be used to put the PN532 into Power Down mode in order to save power
    /// consumption.
    ///
    /// For more information, see 7.2.11 PowerDown
    PowerDown = 0x16,
    /// This command is used to configure the different settings of the PN532.
    ///
    /// For more information, see 7.3.1 RFConfiguration
    RFConfiguration = 0x32,
    /// This command is used for radio regulation test.
    ///
    /// For more information, see 7.3.2 RFRegulationTest
    RFRegulationTest = 0x58,
    /// This command is used by a host controller to activate a target using either active or
    /// passive communication mode during communication over DEP protocol.
    ///
    /// For more information, see 7.3.3 InJumpForDEP
    InJumpForDEP = 0x56,
    /// This command is used by a host controller to activate a target using either active or
    /// passive communication mode during communication over PSL or DEP protocols.
    ///
    /// For more information, see 7.3.4 InJumpForPSL
    InJumpForPSL = 0x46,
    /// This command tells PN532 to detect as many targets as possible in passive mode.
    ///
    /// For more information, see 7.3.5 InListPassiveTarget
    InListPassiveTarget = 0x4A,
    /// This command is used by a host controller to launch an activation of a target in case of
    /// passive mode.
    ///
    /// For more information, see 7.3.6 InATR
    InATR = 0x50,
    /// This command is used by a host controller to change the defined bit rates either with a TPE
    /// target or with a ISO/IEC14443-4 target.
    ///
    /// For more information, see 7.3.7 InPSL
    InPSL = 0x4E,
    /// This command is used to support protocol data exchanges between the PN532 as initiator and
    /// a target.
    ///
    /// For more information, see 7.3.8 InDataExchange
    InDataExchange = 0x40,
    /// This command is used to support basic data exchanges between the PN532 and a target.
    ///
    /// For more information, see 7.3.9 InCommunicateThru
    InCommunicateThru = 0x42,
    /// Command to deselect specified targets(s).
    ///
    /// For more information, see 7.3.10 InDeselect
    InDeselect = 0x44,
    /// Command to release the specified target(s).
    ///
    /// For more information, see 7.3.11 InRelease
    InRelease = 0x52,
    /// Command to select the specified target.
    ///
    /// For more information, see 7.3.12 InSelect
    InSelect = 0x54,
    /// This command is used to poll card(s) / target(s) of specified Type present in the RF field.
    ///
    /// For more information, see 7.3.13 InAutoPoll
    InAutoPoll = 0x60,
    /// The host controller uses this command to configure the PN532 as target.
    ///
    /// For more information, see 7.3.14 TgInitAsTarget
    TgInitAsTarget = 0x8C,
    /// This command is used to give General Bytes to the PN532.
    ///
    /// For more information, see 7.3.15 TgSetGeneralBytes
    TgSetGeneralBytes = 0x92,
    /// This command allows the host controller to get back the data received by the PN532 from its
    /// initiator.
    ///
    /// For more information, see 7.3.16 TgGetData
    TgGetData = 0x86,
    /// This command allows the host controller to spully PN532 with teh data that it wants to send
    /// back to teh initiator.
    ///
    /// For more information, see 7.3.17 TgSetData
    TgSetData = 0x8E,
    /// This command is used if the overall amount of data to be sent cannot be transmitted in one
    /// frame.
    ///
    /// For more information, see 7.3.18 TgSetMetaData
    TgSetMetaData = 0x94,
    /// This command is used to get a packet of data from an initiator and to send it back to the
    /// host controller.
    ///
    /// For more information, see 7.3.19 TgGetInitiatorCommand
    TgGetInitiatorCommand = 0x88,
    /// This command is used to send a response packet of data to an initiator.
    ///
    /// For more information, see 7.3.20 TgResponseToInitiator
    TgResponseToInitiator = 0x90,
    /// This command is used by the host controller to know what the current state of the PN532 is.
    ///
    /// For more information, see 7.3.21 TgGetTargetStatus
    TgGetTargetStatus = 0x8A,
};


// Apache License
// https://github.com/WMT-GmbH/pn532
#if 0
/// SAM mode parameter to be used in [`Command::SAMConfiguration`]
#[derive(Copy, Clone, Eq, PartialEq, Debug)]
pub enum SAMMode {
    /// The SAM is not used; this is the default mode
    Normal,
    /// The couple PN532+SAM is seen as only one contactless SAM card
    /// from the external world
    VirtualCard {
        /// In multiples of 50ms
        timeout: uint8_t,
    },
    /// The host controller can access to the SAM with standard PCD commands
    /// (InListPassiveTarget, InDataExchange, ...)
    WiredCard,
    /// Both the PN532 and the SAM are visible from the external world
    /// as two separated targets
    DualCard,
}
#endif


/// Card type parameter to be used in [`Command::InListPassiveTarget`]
enum class CardType:uint8_t {
    /// 106 kbps type A (ISO/IEC14443 Type A)
    IsoTypeA = 0x00,
    /// 212 kbps (FeliCa polling)
    FeliCa212kbps = 0x01,
    /// 424 kbps (FeliCa polling)
    FeliCa424kbps = 0x02,
    /// 106 kbps type B (ISO/IEC14443-3B)
    IsoTypeB = 0x03,
    /// 106 kbps Innovision Jewel tag
    Jewel = 0x04,
};

/// Bitrate to be used in [`Command::RFRegulationTest`]
enum class TxSpeed:uint8_t {
    /// 106 kbps
    Tx106kbps = 0b0000'0000,
    /// 212 kbps
    Tx212kbps = 0b0001'0000,
    /// 424 kbps
    Tx424kbps = 0b0010'0000,
    /// 848 kbps
    Tx848kbps = 0b0011'0000,
};

/// Type of modulation to be used in [`Command::RFRegulationTest`]
enum class TxFraming:uint8_t {
    Mifare = 0b0000'0000,
    FeliCa = 0b0000'0010,
};

enum class NTAGCommand:uint8_t {
    GetVersion = 0x60,
    Read = 0x30,
    FastRead = 0x3A,
    Write = 0xA2,
    CompWrite = 0xA0,
    ReadCnt = 0x39,
    PwdAuth = 0x1B,
    ReadSig = 0x3C,
};

enum class MifareCommand:uint8_t {
    AuthenticationWithKeyA = 0x60,
    AuthenticationWithKeyB = 0x61,
    PersonalizeUIDUsage = 0x40,
    SetModType = 0x43,
    Read = 0x30,
    Write = 0xA0,
    Decrement = 0xC0,
    Increment = 0xC1,
    Restore = 0xC2,
    Transfer = 0xB0,
};

static constexpr size_t MAX_REQUEST_PAYLOAD_LENGTH = 8;


struct Request{
    Command command;
    std::array<uint8_t, MAX_REQUEST_PAYLOAD_LENGTH> payload;
};

#if 0
struct CommandFactory{
    static constexpr auto GET_FIRMWARE_VERSION = Request(Command::GetFirmwareVersion, {});
    static constexpr auto INLIST_ONE_ISO_A_TARGET =
        Request(Command::InListPassiveTarget, {1, static_cast<uint8_t>(CardType::IsoTypeA)});

    static constexpr auto SELECT_TAG_1 = Request(Command::InSelect, {1});
    static constexpr auto SELECT_TAG_2 = Request(Command::InSelect, {2});
    static constexpr auto DESELECT_TAG_1 = Request(Command::InDeselect, {1});
    static constexpr auto DESELECT_TAG_2 = Request(Command::InDeselect, {2});
    static constexpr auto RELEASE_TAG_1 = Request(Command::InRelease, {1});
    static constexpr auto RELEASE_TAG_2 = Request(Command::InRelease, {2});

    static constexpr auto rf_regulation_test(TxSpeed tx_speed, TxFraming tx_framing) {
        return Request(
            Command::RFRegulationTest,
            {static_cast<uint8_t>(tx_speed)| static_cast<uint8_t>(tx_framing)}
        );
    }
};
#endif

};
}
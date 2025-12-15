#pragma once

#include <cstdint>

// MIT license
// https://github.com/orcahand/orca_core/blob/main/orca_core/hardware/dynamixel_client.py

namespace ymd::robots::dynamixel{


//The following addresses assume XH motors.
//see https://emanual.robotis.com/docs/en/dxl/x/xc330-t288/ for control table

static constexpr uint8_t ADDR_OPERATING_MODE = 11;
static constexpr uint8_t ADDR_TORQUE_ENABLE = 64;
static constexpr uint8_t ADDR_GOAL_POSITION = 116;
static constexpr uint8_t ADDR_GOAL_PWM = 100;
static constexpr uint8_t ADDR_GOAL_CURRENT = 102;
static constexpr uint8_t ADDR_PROFILE_VELOCITY = 112;
static constexpr uint8_t ADDR_PRESENT_POSITION = 132;
static constexpr uint8_t ADDR_PRESENT_VELOCITY = 128;
static constexpr uint8_t ADDR_PRESENT_CURRENT = 126;
static constexpr uint8_t ADDR_PRESENT_POS_VEL_CUR = 126;
static constexpr uint8_t ADDR_MOVING_STATUS = 123;
static constexpr uint8_t ADDR_PRESENT_TEMPERATURE = 146;

//Data Byte Length
static constexpr size_t LEN_OPERATING_MODE = 1;
static constexpr size_t LEN_PRESENT_POSITION = 4;
static constexpr size_t LEN_PRESENT_VELOCITY = 4;
static constexpr size_t LEN_PRESENT_CURRENT = 2;
static constexpr size_t LEN_PRESENT_POS_VEL_CUR = 10;
static constexpr size_t LEN_GOAL_POSITION = 4;
static constexpr size_t LEN_GOAL_PWM = 2;
static constexpr size_t LEN_GOAL_CURRENT = 2;
static constexpr size_t LEN_PROFILE_VELOCITY = 4;
static constexpr size_t LEN_MOVING_STATUS = 1;
static constexpr size_t LEN_PRESENT_TEMPERATURE = 1;


enum class UartBaudrate:uint8_t{
    // 2.4.4 Baud Rate(8)
    // The Baud Rate(8) determines serial communication speed between a controller and DYNAMIXEL.

    // Value	    Baud Rate	    Margin of Error
    // 6	        4M [bps]	    0.000 [%]
    // 5	        3M [bps]	    0.000 [%]
    // 4	        2M [bps]	    0.000 [%]
    // 3	        1M [bps]	    0.000 [%]
    // 2	        115,200 [bps]	0.0064 [%]
    // 1(Default)	57,600 [bps]	0.0016 [%]
    // 0	        9,600 [bps]	    0.000 [%]

    _9600 = 0b000,
    _57600 = 0b001,
    _115200 = 0b010,
    _1000000 = 0b011,
    _2000000 = 0b100,
    _3000000 = 0b101,
    _4000000 = 0b110
};

enum class CtrlMode:uint8_t{
    // see https://emanual.robotis.com/docs/en/dxl/x/xc330-t288/#operating-mode11
    // 0	Current Control Mode	
    // DYNAMIXEL only controls current(torque) regardless of speed and position. 
    // This mode is ideal for a gripper or a system that only uses current(torque) control 
    // or a system that has additional velocity/position controllers.

    // 1	Velocity Control Mode	
    // This mode controls velocity. This mode is identical to the Wheel Mode(endless) from 
    // existing DYNAMIXEL. This mode is ideal for wheel-type robots.

    
    // 3(Default)	Position Control Mode	
    // This mode controls position. This mode is identical to the Joint Mode from existing DYNAMIXEL. 
    // Operating position range is limited by the Max Position Limit(48) and the Min Position Limit(52). 
    // This mode is ideal for articulated robots that each joint rotates less than 360 degrees.

    // 4	Extended Position Control Mode(Multi-turn)	
    // This mode controls position. This mode is identical to the Multi-turn Position Control 
    // from existing DYNAMIXEL. 512 turns are supported(-256[rev] ~ 256[rev]). 
    // This mode is ideal for multi-turn wrists or conveyer systems or a system that 
    // requires an additional reduction gear. Note that Max Position Limit(48), Min Position Limit(52) 
    // are not used on Extended Position Control Mode.

    // 5	Current-based Position Control Mode	This mode controls both position and current(torque). 
    // Up to 512 turns are supported(-256[rev] ~ 256[rev]). This mode is ideal for a system that 
    // requires both position and current control such as articulated robots or grippers.

    // 16	PWM Control Mode (Voltage Control Mode)	This mode directly controls PWM output. (Voltage Control Mode)
    Current = 0b000,
    Velocity = 0b001,
    Position = 0b011,
    MultiTurn = 0b101,
    CurrentBasedPosition = 0b101,
    Voltage = 16
};

#if 0
DEFAULT_POS_SCALE = 2.0 * np.pi / 4096  //0.088 degrees
//See http://emanual.robotis.com/docs/en/dxl/x/xh430-v210/#goal-velocity
DEFAULT_VEL_SCALE = 0.229 * 2.0 * np.pi / 60.0  //0.229 rpm
DEFAULT_CUR_SCALE = 1.34
#endif


namespace details{

#if 0

def signed_to_unsigned(value: int, size: int) -> int:
    """Converts the given value to its unsigned representation."""
    if value < 0:
        bit_size = 8 * size
        max_value = (1 << bit_size) - 1
        value = max_value + value
    return value


def unsigned_to_signed(value: int, size: int) -> int:
    """Converts the given value from its unsigned representation."""
    bit_size = 8 * size
    if (value & (1 << (bit_size - 1))) != 0:
        value = -((1 << bit_size) - value)
    return value
#endif



}

//dxl servo serialize error
enum class DxlDeError:uint8_t{
    H1, //H1 is not 0xff
    H2, //H2 is not 0xff
    H3, //H3 is not 0xfd
    RSRV, // reserved is not 0x00,
    CRC // CRC error
};

enum class ErrorKind:uint8_t{
    // 0x01	Result Fail	Failed to process the sent Instruction Packet
    FailedToProcess = 0x01,

    // 0x02	Instruction Error	An undefined Instruction has been used
    // Action has been used without Reg Write
    UndefinedInstruction = 0x02,

    // 0x03	CRC Error	The CRC of the sent Packet does not match the expected value
    CrcError = 0x03,

    // 0x04	Data Range Error	Data to be written to the specified Address is outside the range of the minimum/maximum value
    DataRangeError = 0x04,

    // 0x05	Data Length Error	Attempted to write Data that is shorter than the required data length of the specified Address
    // (ex: when you attempt to only use 2 bytes of a register that has been defined as 4 bytes)
    DataLengthError = 0x05,

    // 0x06	Data Limit Error	Data to be written to the specified Address is outside of the configured Limit value
    DataLimitError = 0x06,

    // 0x07	Access Error	Attempted to write a value to an Address that is Read Only or has not been defined
    // Attempted to read a value from an Address that is Write Only or has not been defined
    // Attempted to write a value to an EEPROM register while Torque was Enabled.
    Access = 0x07
};

}
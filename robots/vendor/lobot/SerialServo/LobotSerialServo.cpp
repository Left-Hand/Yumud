#include "LobotSerialServo.hpp"



static constexpr uint8_t LOBOT_SERVO_FRAME_HEADER         = 0x55;
static constexpr uint8_t LOBOT_SERVO_MOVE_TIME_WRITE      =  1;
static constexpr uint8_t LOBOT_SERVO_MOVE_TIME_READ       =  2;
static constexpr uint8_t LOBOT_SERVO_MOVE_TIME_WAIT_WRITE =  7;
static constexpr uint8_t LOBOT_SERVO_MOVE_TIME_WAIT_READ  =  8;
static constexpr uint8_t LOBOT_SERVO_MOVE_START            = 11;
static constexpr uint8_t LOBOT_SERVO_MOVE_STOP             = 12;
static constexpr uint8_t LOBOT_SERVO_ID_WRITE              = 13;
static constexpr uint8_t LOBOT_SERVO_ID_READ               = 14;
static constexpr uint8_t LOBOT_SERVO_ANGLE_OFFSET_ADJUST   = 17;
static constexpr uint8_t LOBOT_SERVO_ANGLE_OFFSET_WRITE    = 18;
static constexpr uint8_t LOBOT_SERVO_ANGLE_OFFSET_READ     = 19;
static constexpr uint8_t LOBOT_SERVO_ANGLE_LIMIT_WRITE     = 20;
static constexpr uint8_t LOBOT_SERVO_ANGLE_LIMIT_READ      = 21;
static constexpr uint8_t LOBOT_SERVO_VIN_LIMIT_WRITE       = 22;
static constexpr uint8_t LOBOT_SERVO_VIN_LIMIT_READ        = 23;
static constexpr uint8_t LOBOT_SERVO_TEMP_MAX_LIMIT_WRITE  = 24;
static constexpr uint8_t LOBOT_SERVO_TEMP_MAX_LIMIT_READ   = 25;
static constexpr uint8_t LOBOT_SERVO_TEMP_READ             = 26;
static constexpr uint8_t LOBOT_SERVO_VIN_READ              = 27;
static constexpr uint8_t LOBOT_SERVO_POS_READ              = 28;
static constexpr uint8_t LOBOT_SERVO_OR_MOTOR_MODE_WRITE   = 29;
static constexpr uint8_t LOBOT_SERVO_OR_MOTOR_MODE_READ    = 30;
static constexpr uint8_t LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE  = 31;
static constexpr uint8_t LOBOT_SERVO_LOAD_OR_UNLOAD_READ   = 32;
static constexpr uint8_t LOBOT_SERVO_LED_CTRL_WRITE        = 33;
static constexpr uint8_t LOBOT_SERVO_LED_CTRL_READ         = 34;
static constexpr uint8_t LOBOT_SERVO_LED_ERROR_WRITE       = 35;
static constexpr uint8_t LOBOT_SERVO_LED_ERROR_READ        = 36;



#pragma pack(push, 1)

template<typename T>
struct FrameWapper_t{
protected:

    template<typename U>
    struct FrameBody_t{
        const uint8_t length = sizeof(FrameBody_t<U>);
        const uint8_t command;
        const U content;
        const uint8_t checksum;
        
        constexpr FrameBody_t(uint8_t _command, U && _content) :
                command(_command), content(std::forward<U>(_content)), 
                checksum(CalculateCheckSum(reinterpret_cast<const uint8_t *>(this), length)) {;}

        scexpr uint8_t CalculateCheckSum(const uint8_t * buf, const size_t len) {
            uint16_t temp = 0;
            
            for (size_t i = 0; i < len; i++) {
                temp += buf[i];
            }
            return ~temp;
        }
    };


    const uint8_t header[2] = {
        LOBOT_SERVO_FRAME_HEADER, LOBOT_SERVO_FRAME_HEADER
    };

    const uint8_t id;
    
    const FrameBody_t<T> body;
    
public:
    constexpr FrameWapper_t(const uint8_t _id, uint8_t _command, T && _content) :
            id(_id), body(_command, std::forward<T>(_content)) {;}

    constexpr FrameWapper_t(const FrameWapper_t & other) = delete;
    constexpr FrameWapper_t(FrameWapper_t && other) = delete;
    

    constexpr uint8_t operator[](const size_t idx) const {
        return reinterpret_cast<const uint8_t *>(this)[idx];
    }

    consteval size_t size() const {
        return sizeof(FrameWapper_t<T>);
    }

    constexpr const uint8_t * begin() const {
        return reinterpret_cast<const uint8_t *>(this);
    }

    constexpr const uint8_t * end() const{
        return begin() + size();
    }

    auto to_arr() const {
        return std::array<uint8_t, size()>{this->begin(), this->end()};
    }
};

#pragma pack(pop)


using namespace ymd::drivers;

#define CREATE_FRAME(command, ...) FrameWapper_t(this->id_, command, std::make_tuple(__VA_ARGS__))
#define WRITE_FRAME(command, ...) this->write_frame(CREATE_FRAME(command, __VA_ARGS__));


void LobotSerialServo::move(const int16_t position,const uint16_t time) {
    WRITE_FRAME(LOBOT_SERVO_MOVE_TIME_WRITE, time, CLAMP2(position, 1000));
}

void LobotSerialServo::set_id(const uint8_t new_id) {
    WRITE_FRAME(LOBOT_SERVO_ID_WRITE, new_id);
    id_ = new_id;
}


void LobotSerialServo::unload() {
    WRITE_FRAME(LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE, 0);
}

void LobotSerialServo::load() {
    WRITE_FRAME(LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE, 1);
}
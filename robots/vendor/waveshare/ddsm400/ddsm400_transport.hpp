#include "ddsm400_primitive.hpp"


namespace ymd::robots::waveshare::ddsm400::transport{
using namespace primitive;
template<typename T>
static constexpr std::array<uint8_t, NUM_PACKET_BYTES> 
serialize_request(const MotorId motor_id, const T & msg){
    std::array<uint8_t, NUM_PACKET_BYTES> buf;
    buf[0] = motor_id.to_u8();
    buf[1] = (std::bit_cast<uint8_t>(T::COMMAND));
    msg.fill_bytes(std::span<uint8_t, NUM_PAYLOAD_BYTES>(&buf[2], NUM_PAYLOAD_BYTES));
    const auto crc8 = calc_crc8(std::span<const uint8_t>(&buf[0], NUM_PACKET_BYTES - 1));
    buf[NUM_PACKET_BYTES - 1] = crc8;
    return buf;
}


// 传输层解析器
struct RecvTransportParser final{
    enum class PacketRecvState:uint8_t {
        MotorId = 0,
        Command = 1,
        Payload = 2,
        Crc = 3
    };

    DEF_FRIEND_DERIVE_DEBUG(PacketRecvState)

    template<typename Fn>
    explicit RecvTransportParser(Fn && callback):
        callback_(std::forward<Fn>(callback))
    {
        reset();
    }

    void reset(){
        payload_bytes_cnt_ = 0;
        state_ = PacketRecvState::MotorId;
    }

    void push_byte(const uint8_t byte){
        switch(state_){
            case PacketRecvState::MotorId:{
                request_packet_.motor_id = MotorId::from_u8(byte);
                state_ = PacketRecvState::Command;
                break;
            }
            case PacketRecvState::Command:{
                const auto may_req_command = try_into_req_command(byte);
                if(may_req_command.is_none()){
                    reset();
                    return;
                }
                request_packet_.req_command = may_req_command.unwrap();
                state_ = PacketRecvState::Payload;
                break;
            }
            case PacketRecvState::Payload:{
                request_packet_.payload[payload_bytes_cnt_] = byte;
                payload_bytes_cnt_++;
                if(payload_bytes_cnt_ >= NUM_PAYLOAD_BYTES){
                    state_ = PacketRecvState::Crc;
                }
                break;
            }
            case PacketRecvState::Crc:{
                
                const uint16_t crc8 = request_packet_.calc_crc();
                
                if(crc8 != byte){
                    on_crc_failed(crc8, byte);
                }else{
                    handle_flat_request(request_packet_);
                }
                reset();
                break;
            }
            default:
                reset();
                break;
        }
    }


    void handle_flat_request(const FlatPacket & request){
        if(callback_ == nullptr){
            __builtin_trap();
        }

        callback_(request);
    }

    void on_crc_failed(const uint8_t calc, const uint8_t given){
        //do nothing
    }
private:
    using Callback = std::function<void(const FlatPacket &)>;

    Callback callback_;
    FlatPacket request_packet_;
    uint8_t payload_bytes_cnt_ = 0;
    PacketRecvState state_ = PacketRecvState::MotorId;
};
}
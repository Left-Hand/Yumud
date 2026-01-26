#include "stl06n.hpp"
#include <span>

using namespace ymd;
using namespace ymd::drivers::stl06n;

using Self = STL06N_ParseReceiver;


#define ASSUME(expr) ({\
    const bool is_right = bool(expr);\
    if(!is_right) __builtin_trap();\
})\


void Self::push_byte(const uint8_t byte){
    switch(fsm_state_){
        case FsmState::AwaitingHeader:
            if(byte != HEADER_TOKEN){
                reset();
                return;
            }
            fsm_state_ = FsmState::AwaitingVerlen;
            return;

        case FsmState::AwaitingVerlen:
            may_command_ = Command::try_from_u8(byte);

            if(may_command_.is_none()){
                reset();
                return;

            }
            fsm_state_ = FsmState::Payload;

            return;

        case FsmState::Payload:
            bytes_[bytes_count_] = byte;
            bytes_count_++;

            if(bytes_count_ > may_command_.unwrap().payload_length()){
                flush();
                reset();
            }
            return;

        case FsmState::Emitting:
            PANIC{"racing condition is happening!!!"};
            return;

    }
    PANIC{"unreachable"};
    reset();
}

void Self::push_bytes(const std::span<const uint8_t> bytes){
    for(const auto byte: bytes){
        push_byte(byte);
    }
}



STL06N_ParseReceiver::STL06N_ParseReceiver(Callback && callback):
    callback_(std::move(callback))
{
    if(callback_ == nullptr)
        PANIC{"callback cannot be nullptr"};

    reset();
}

void Self::flush(){
    fsm_state_ = FsmState::Emitting;

    const auto command = may_command_.unwrap();
    const size_t num_context_bytes = command.payload_length();

    const auto context = std::span(bytes_.data(), num_context_bytes);

    //尾元素本身指向crc8校验 并不构成越界
    const uint8_t actual_crc = context[num_context_bytes];
    const uint8_t expected_crc = [&] -> uint8_t{
        return Crc8Calculator()
            .push_byte(HEADER_TOKEN)
            .push_byte(command.to_u8())
            .push_bytes(context)
            .get();
    }();

    if(expected_crc != actual_crc) [[unlikely]]{
        const auto event = Event(Event::InvalidCrc{
            .command = command,
            .expected = expected_crc,
            .actual = actual_crc
        });
        callback_(event);
        return;
    }

    switch(command.kind()){
        case Command::Sector:{
            const auto & sector = *reinterpret_cast<const LidarSectorPacket *>(context.data());
            const auto event = Event(Event::DataReady{.sector = sector});
            callback_(event);
        }
        return;
        case Command::Start:
            callback_(Event(Event::Start{}));
        return;
        case Command::Stop:
            callback_(Event(Event::Stop{}));
        return;
        case Command::SetSpeed:
            callback_(Event(Event::SetSpeed{}));
        return;
        case Command::GetSpeed:{
            //context[0] 为数据长度字段 固定为4
            const auto speed = LidarSpinSpeedCode::from_bytes(
                context[1], context[2]
            );
            callback_(Event(Event::GetSpeed{.speed = speed}));
            return;
        }
    }
    //unreachable
    __builtin_trap();
}

void Self::reset(){
    bytes_count_ = 0;
    fsm_state_ = FsmState::AwaitingHeader;
    may_command_ = None;
}

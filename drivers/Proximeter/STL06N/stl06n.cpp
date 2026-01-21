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
            fsm_state_ = FsmState::Remaining;

            return;

        case FsmState::Remaining:
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
    const auto num_bytes = command.payload_length();

    const uint8_t * buffer_begin = bytes_.data();

    //尾元素本身指向crc8校验 并不构成越界
    const uint8_t actual_crc = buffer_begin[num_bytes];

    const uint8_t expected_crc = [&]{
        Crc8Calculator calc;
        calc = calc.push_byte(HEADER_TOKEN);
        calc = calc.push_byte(command.to_u8());
        calc = calc.push_bytes(std::span(buffer_begin, num_bytes));
        return calc.get();
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
            const auto & sector = *reinterpret_cast<const LidarSectorPacket *>(buffer_begin);
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
            //buffer_begin[0] 为数据长度字段 固定为4
            const auto speed = LidarSpinSpeedCode::from_bytes(
                buffer_begin[1], buffer_begin[2]
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

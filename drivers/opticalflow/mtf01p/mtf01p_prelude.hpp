#pragma once


#include <cstdint>
#include "core/utils/Option.hpp"

// https://micoair.cn/docs/Micolink-xie-yi-ding-yi-yu-jie-xi

// 帧头	    设备 ID	    系统 ID	    消息 ID	    包序列	    负载长度	数据负载	帧校验
// 0xEF	    0X0F	    0x00	    0x51	    0-0xFF	    0x14	见下方说明	前面所有数据之和

namespace ymd::drivers::mtf01p{
    

struct [[nodiscard]] DistanceCode final{
    uint32_t bits;

    bool is_invalid() const {
        return bits == 0;
    }

    Option<uint32_t> to_millimeters() const {
        if(is_invalid()) return None;
        return Some(bits);
    }
};

struct [[nodiscard]] SpeedCode final{
    int16_t bits;//实际速度(cm/s) = 光流速度 * 高度(m)
};

struct alignas(4) [[nodiscard]] Packet final{
    uint32_t systime_ms;
    DistanceCode distance_code;
    uint8_t signal_strength;
    uint8_t precision;
    bool is_range_valid;
    uint8_t __resv__;

    SpeedCode opticalflow_x_speed_code;
    SpeedCode opticalflow_y_speed_code;

    uint8_t opticalflow_quality;
    uint8_t opticalflow_status;
    uint16_t __resv2__;
};

static_assert(sizeof(Packet) == 20);
static constexpr size_t MAX_PAYLOAD_SIZE = 20;

struct [[nodiscard]] ChecksumBuilder final{
    uint8_t checksum;

    static constexpr ChecksumBuilder from_default(){
        return ChecksumBuilder{.checksum = 0};
    }


    constexpr ChecksumBuilder push_byte(const uint8_t byte) const noexcept {
        ChecksumBuilder self = *this;
        self.checksum = static_cast<uint8_t>(self.checksum + byte);
        return self;
    }

    constexpr ChecksumBuilder push_bytes(std::span<const uint8_t> bytes) const noexcept {
        ChecksumBuilder self = *this;
        for(size_t i = 0; i < bytes.size(); i++){
            self = self.push_byte(bytes[i]);
        }
        return self;
    }

    [[nodiscard]] uint8_t finalize() const noexcept {
        return checksum;
    }
};

using Callback = std::function<void(Packet)>;

class AlxAoa_ParseReceiver final{
public:
    enum class FsmState:uint8_t{
        AwaitHeader,
        AwaitDeviceId,
        AwaitSysId,
        AwaitMsgId,
        AwaitPacketSeq,
        AwaitLen,
        AwaitPayload,
        AwaitChecksum,
    };

    static constexpr uint8_t HEADER_TOKEN = 0xef;
    static constexpr uint8_t DEVICE_ID = 0x0f;
    static constexpr uint8_t SYS_ID = 0x00;
    static constexpr uint8_t MSG_ID = 0x51;

    explicit AlxAoa_ParseReceiver(Callback callback):
        callback_(callback)
    {
        reset();
    }

    void push_bytes(const std::span<const uint8_t> bytes){
        for(const auto byte : bytes){
            push_byte(byte);
        }
    }

    void push_byte(const uint8_t byte){

        auto sw_fsm_state = [&](const FsmState state){ 
            fsm_state_ = state; 
        };
        
        switch(fsm_state_){
            case FsmState::AwaitHeader:{
                if(byte != HEADER_TOKEN){reset(); return;}
                checksum_bd_ = checksum_bd_.push_byte(byte);
                sw_fsm_state(FsmState::AwaitDeviceId);
                break;
            }
            case FsmState::AwaitDeviceId:{
                if(byte != DEVICE_ID){reset(); return;}
                checksum_bd_ = checksum_bd_.push_byte(byte);
                sw_fsm_state(FsmState::AwaitSysId);
                break;
            }
            case FsmState::AwaitSysId:{
                if(byte != SYS_ID){reset(); return;}
                checksum_bd_ = checksum_bd_.push_byte(byte);
                sw_fsm_state(FsmState::AwaitMsgId);
                break;
            }
            case FsmState::AwaitMsgId:{
                if(byte != MSG_ID){reset(); return;}
                checksum_bd_ = checksum_bd_.push_byte(byte);
                sw_fsm_state(FsmState::AwaitPacketSeq);
                break;
            }
            case FsmState::AwaitPacketSeq:{
                //TODO: use packet seq id

                checksum_bd_ = checksum_bd_.push_byte(byte);
                sw_fsm_state(FsmState::AwaitLen);
                break;
            }
            case FsmState::AwaitLen:{
                if(byte != sizeof(Packet)){reset(); return;}

                checksum_bd_ = checksum_bd_.push_byte(byte);
                sw_fsm_state(FsmState::AwaitPayload);
                break;
            }
            case FsmState::AwaitPayload:{
                if(payload_len_ >= sizeof(Packet)){
                    sw_fsm_state(FsmState::AwaitChecksum);
                }else{
                    payload_bytes[payload_len_] = byte;
                    checksum_bd_ = checksum_bd_.push_byte(byte);
                    payload_len_ ++;
                }
                break;
            }
            case FsmState::AwaitChecksum:{
                if(byte == checksum_bd_.finalize()){
                    flush();
                }
                reset();
                break;
            }
        }
    }


    void flush(){
        if(callback_ == nullptr) return;
        callback_(packet);
    }

    void reset(){
        fsm_state_ = FsmState::AwaitHeader;
        payload_len_ = 0;
        checksum_bd_ = ChecksumBuilder::from_default();
    }
private:

    union{
        std::array<uint8_t, MAX_PAYLOAD_SIZE> payload_bytes;
        Packet packet;
    };

    Callback callback_;
    FsmState fsm_state_;
    uint8_t payload_len_;
    ChecksumBuilder checksum_bd_;

};

}
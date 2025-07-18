#pragma once
#include "Encoder.hpp"


namespace ymd::drivers{


class MagStatus{
public:
    static constexpr MagStatus High(const uint16_t value = 0){
        return MagStatus{Kind::MagnetHigh, value};
    }

    static constexpr MagStatus Low(const uint16_t value = 0){
        return MagStatus{Kind::MagnetLow, value};
    }

    static constexpr MagStatus Proper(const uint16_t value = 0){
        return MagStatus{Kind::MagnetProper, 0};
    }

    static constexpr MagStatus Invalid(){
        return MagStatus{Kind::Invalid, 0};
    }

    constexpr bool is_high() const{
        return kind_ == Kind::MagnetHigh;
    }

    constexpr bool is_low() const{
        return kind_ == Kind::MagnetLow;
    }

    constexpr bool is_proper() const{
        return kind_ == Kind::MagnetProper;
    }

    constexpr uint16_t value() const{
        return value_;
    }

    constexpr Option<EncoderError> map_to_error() const{
        switch(kind_){
            case Kind::MagnetProper: return None;
            case Kind::MagnetHigh: return Some(EncoderError::MagnetHigh);
            case Kind::MagnetLow: return Some(EncoderError::MagnetLow);
            case Kind::Invalid: return Some(EncoderError::MagnetInvalid);
            default: __builtin_unreachable();
        }
    }

private:
    enum class Kind:uint8_t{
        MagnetHigh,
        MagnetLow,
        MagnetProper,
        Invalid
    };
    Kind kind_;
    uint16_t value_;


    explicit constexpr MagStatus(const Kind kind, const uint16_t value):
        kind_(kind), value_(value){;}
};

class MagEncoderIntf: public EncoderIntf{
public:
    virtual Result<MagStatus, EncoderError> get_mag_status() = 0;
};


}

#pragma once

#include "types/rect2/rect2.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace etk{

using namespace ymd::drivers;

struct EyeInfo{
    Vector2_t<real_t> pos;
};

struct EyelidInfo{
    Range_t<real_t> range;
};

class Eyes;

class EyesPhy{
protected:

    struct EyePhy{
        PwmRadianServo yaw_;
        PwmRadianServo pitch_;
        EyeInfo info_;
        auto & operator = (const EyeInfo & info){
            info_.pos = (info.pos + info_.pos * 7) / 8;
            yaw_.set_radian(info_.pos[0] * real_t(0.6) + real_t(1.8));
            pitch_.set_radian(info_.pos[1] * real_t(-0.4) + real_t(1.8));
            return *this;
        }
    };

    struct EyelidPhy{
        PwmRadianServo lower_;
        PwmRadianServo upper_;

        EyelidPhy & operator = (const EyelidInfo & info){
            lower_.set_radian(info.range[0]);
            upper_.set_radian(info.range[1]);
            return *this;
        }
    };

    EyePhy eye_phy_;
    std::array<EyelidPhy,2> eyelid_phys_;

    struct Refs{
        hal::PwmIntf & yaw;
        hal::PwmIntf & pitch; 
        hal::PwmIntf & upper_l;
        hal::PwmIntf & lower_l;
        hal::PwmIntf & upper_r;
        hal::PwmIntf & lower_r;
    };

    friend class Eyes;

public:
    EyesPhy(const Refs & refs)
    : eye_phy_{PwmRadianServo{refs.yaw}, PwmRadianServo{refs.pitch}}
    , eyelid_phys_{
        EyelidPhy{PwmRadianServo{refs.upper_l}, PwmRadianServo{refs.lower_l}},
        EyelidPhy{PwmRadianServo{refs.upper_r}, PwmRadianServo{refs.lower_r}}
    }{}

    void move(const EyeInfo & eye_info, const std::array<EyelidInfo, 2> & eyelids_info){
        eye_phy_ = eye_info;
        eyelid_phys_[0] = eyelids_info[0];
        eyelid_phys_[1] = eyelids_info[1];
    }
};

class Eyes{
public:
    struct Config{
        Vector2i l_center;
        Vector2i r_center;

        size_t eye_radius;
        size_t iris_radius;
        size_t pupil_radius;
    };

protected:
    const Config & config_;

    EyesPhy eyes_phy_;
    EyeInfo eye_info_;
    std::array<EyelidInfo,2> eyelids_info_;
public:
    Eyes(const Config & config, const EyesPhy::Refs & refs):
            config_(config),
            eyes_phy_{refs}{}

    void solve(const Vector2i & raw_lpos, const Vector2i & raw_rpos){

    }

    void update(const EyeInfo & eye_info, const std::array<EyelidInfo, 2> & eyelids_info){
        eye_info_ = std::move(eye_info);
        eyelids_info_ = std::move(eyelids_info);
    }

    void move(){
        eyes_phy_.move(eye_info_, eyelids_info_);
    }

    auto & eyeInfo(){return eye_info_;}
    auto & eyelidsInfo(){return eyelids_info_;}

    const auto & eyeInfo() const {return eye_info_;}
    const auto & eyelidsInfo() const {return eyelids_info_;}


    void render(PainterConcept & painter) {
        auto render_eye = [&](const LR side){

            auto center = (side == LR::LEFT) ? config_.l_center : config_.r_center;
            auto center_p = center + eye_info_.pos * config_.eye_radius * real_t(0.5);

            painter.setColor(ColorEnum::WHITE);
            painter.drawFilledCircle(center, config_.eye_radius).unwrap();

            painter.setColor(ColorEnum::BROWN);
            painter.drawFilledCircle(center_p, config_.iris_radius).unwrap();

            painter.setColor(ColorEnum::BLACK);
            painter.drawFilledCircle(center_p, config_.pupil_radius).unwrap();
        };

        render_eye(LR::LEFT);
        render_eye(LR::RIGHT);
    }
};


}
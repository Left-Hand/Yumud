#pragma once

#include "types/rect2/rect2_t.hpp"

namespace etk{

struct EyeInfo{
    Vector2 pos_;
};

struct EyelidInfo{
    Range range_;
};

class Eyes;

class EyesPhy{
protected:

    struct EyePhy{
        PwmChannel & yaw_;
        PwmChannel & pitch_;

        auto & operator = (const EyeInfo & info){
            yaw_ = info.pos_[0];
            pitch_ = info.pos_[1];
            return *this;
        }
    };

    struct EyelidPhy{
        PwmChannel & lower_;
        PwmChannel & upper_;

        EyelidPhy & operator = (const EyelidInfo & info){
            lower_ = info.range_[0];
            upper_ = info.range_[1];
            return *this;
        }
    };

    EyePhy eye_phy_;
    std::array<EyelidPhy,2> eyelid_phys_;

    struct Refs{
        PwmChannel & yaw;
        PwmChannel & pitch; 
        PwmChannel & upper_l;
        PwmChannel & lower_l;
        PwmChannel & upper_r;
        PwmChannel & lower_r;
    };

    friend class Eyes;

public:
    EyesPhy(const Refs & refs)
    : eye_phy_{refs.yaw, refs.pitch}
    , eyelid_phys_{
        EyelidPhy{refs.upper_l, refs.lower_l},
        EyelidPhy{refs.upper_r, refs.lower_r}
    }{}

    void update(const EyeInfo & eye_info, const std::array<EyelidInfo, 2> & eyelids_info){
        eye_phy_ = eye_info;
        eyelid_phys_[0] = eyelids_info[0];
        eyelid_phys_[1] = eyelids_info[1];
    }
};

class Eyes:public CanvasItem{
public:
    struct Config{
        Vector2i l_center;
        Vector2i r_center;

        size_t eye_radius;
        size_t iris_radius;
        size_t pupil_radius;
    };

    EyeInfo eye_info_;
    std::array<EyelidInfo,2> eyelids_info_;
protected:
    const Config & config_;

    EyesPhy eyes_phy;
public:
    Eyes(const Theme & theme, const Config & config, const EyesPhy::Refs & refs):
            CanvasItem(theme), 
            config_(config),
            eyes_phy{refs}{}

    void solve(const Vector2i & raw_lpos, const Vector2i & raw_rpos){

    }

    void update(const EyeInfo & eye_info, const std::array<EyelidInfo, 2> & eyelids_info){
        eye_info_ = std::move(eye_info);
        eyelids_info_ = std::move(eyelids_info);
    }


    void render(PainterConcept & painter) override{
        auto render_eye = [&](const LR side){

            auto center = (side == LR::LEFT) ? config_.l_center : config_.r_center;
            auto center_p = center + eye_info_.pos_ * config_.eye_radius * real_t(0.5);

            painter.setColor(ColorEnum::WHITE);
            painter.drawFilledCircle(center, config_.eye_radius);

            painter.setColor(ColorEnum::BROWN);
            painter.drawFilledCircle(center_p, config_.iris_radius);

            painter.setColor(ColorEnum::BLACK);
            painter.drawFilledCircle(center_p, config_.pupil_radius);
        };

        render_eye(LR::LEFT);
        render_eye(LR::RIGHT);
    }
};


}
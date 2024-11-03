#include "mp6540.hpp"


using namespace yumud;
using namespace yumud::drivers;

static void error(){
    MP6540_DEBUG("terminated!!!");
    CREATE_FAULT;
}

MP6540::MP6540(PWM3_WP && pwms, AIN3_WP && ains)
        : pwms_{
            &pwms[0].get(),
            &pwms[1].get(),
            &pwms[2].get()
        },ains_{
            &ains[0].get(),
            &ains[1].get(),
            &ains[2].get()
        }
    
    {

}



void MP6540::init(){
    for(size_t i = 0; i < 3; i++){
        const auto & item = pwms_[i];
        if(item == nullptr){
            MP6540_DEBUG("pwm[", i, "]: is null");
            error();
        }else{
            // item->init();
        } 
    }

    for(size_t i = 0; i < 3; i++){
        const auto & item = ains_[i]; 
        if(item == nullptr){
            MP6540_DEBUG("ain[", i, "]: is null");
            error();
        }else{

        }
    }
    
    return;
}

void MP6540::enable(const bool en){
    bool has_en = false;
    for(size_t i = 0; i < 3; i++){
        const auto & item = ens_[i]; 
        if(item == nullptr){
        }else{
            item->write(en);
        }
    }

    if(has_en == false){
        MP6540_DEBUG("No enable channel found!");
        error();
    }
}

void MP6540::setSoRes(const real_t so_res_ohms){
    scexpr real_t curr_mirror_ratio = real_t(9200.0);
    volt_to_curr_ratio = curr_mirror_ratio / so_res_ohms;
}

MP6540::MP6540CurrentChannel & MP6540::ch(const size_t index){
    if(index == 0 or index > 3){
        MP6540_DEBUG("Channel index out of range:", index);
        CREATE_FAULT;
    }
    return chs[index - 1];
}

MP6540 & MP6540::operator= (const UVW_Duty & duty){
    std::tie(*pwms_[0], *pwms_[1], *pwms_[2]) = duty; 
    return *this;
}

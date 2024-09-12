#include "mp6540.hpp"



MP6540::MP6540(PWM3 && pwms, AIN3 && ains){

}



void MP6540::init(){
    for(size_t i = 0; i < 3; i++){
        const auto & item = pwms_[i];
        if(item == nullptr){
            MP6540_DEBUG("pwm[", i, "]: is null");
            goto error;
        }else{
            item->init();
        } 
    }

    for(size_t i = 0; i < 3; i++){
        const auto & item = ains_[i]; 
        if(item == nullptr){
            MP6540_DEBUG("ain[", i, "]: is null");
            goto error;
        }else{

        }
    }
    return;

error:
    MP6540_DEBUG("terminated!!!");
    CREATE_FAULT;
}

void MP6540::setSoRes(const real_t so_res_k_ohms){
    static constexpr real_t curr_mirror_ratio = real_t(9800.0 / 1000);
    volt_to_curr_ratio = so_res_k_ohms * curr_mirror_ratio;
}

MP6540::MP6540CurrentChannel & MP6540::ch(const size_t index){
    if(index > 2){
        MP6540_DEBUG("Channel index out of range:", index);
        CREATE_FAULT;
    }
    return chs[index];
}

MP6540 & MP6540::operator= (const UVW_Duty & duty){
    std::tie(*pwms_[0], *pwms_[1], *pwms_[2]) = duty; 
    return *this;
}

#include "CanFilter.hpp"
#include "sys/core/platform.h"

using namespace ymd;

void CanFilter::list(const std::initializer_list<CanID16> & list){
    switch(list.size()){
        default:
            HALT;
        case 2:
            id16[0] =       *std::next(list.begin(), 0);
            id16[1] =       *std::next(list.begin(), 1);
            mask16[0] =     *std::next(list.begin(), 2);
            mask16[1] =     *std::next(list.begin(), 3);

            break;
    }
    is32 = false;
    islist = true;
    inited = true;

    apply();
}

void CanFilter::list(const std::initializer_list<CanID32> & list){
    switch(list.size()){
        default:
            HALT;
        case 2:
            id32 =      *list.begin();
            mask32 =    *std::next(list.begin());
            break;
    }
    is32 = true;
    islist = true;
    inited = true;

    apply();
}

void CanFilter::mask(const CanID16 & id1, const CanID16 & mask1, const CanID16 & id2, const CanID16 & mask2){
    id16[0] = id1;
    id16[1] = id2;
    mask16[0] = mask1;
    mask16[1] = mask2;

    is32 = false;
    islist = false;
    inited = true;

    apply();
}

void CanFilter::mask(const CanID32 & id, const CanID32 & mask){
    id32 = id;
    mask32 = mask;

    is32 = true;
    islist = false;
    inited = true;

    apply();
}


void CanFilter::apply(){
    if(!inited){
        // HALT;
        return;
    }

    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {
        .CAN_FilterIdHigh = id16[1],
        .CAN_FilterIdLow = id16[0],
        .CAN_FilterMaskIdHigh = mask16[1],
        .CAN_FilterMaskIdLow = mask16[0],
        .CAN_FilterFIFOAssignment = CAN_FIFO0,
        .CAN_FilterNumber = idx,
        .CAN_FilterMode = islist ? CAN_FilterMode_IdList : CAN_FilterMode_IdMask,
        .CAN_FilterScale = is32 ? CAN_FilterScale_32bit : CAN_FilterScale_16bit,
        .CAN_FilterActivation = ENABLE,
    };

    CAN_FilterInit(&CAN_FilterInitSturcture);

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
    //register both fifo
    CAN_FilterInit(&CAN_FilterInitSturcture);

}

void CanFilter::deinit(){
    HALT;
    //TODO
}
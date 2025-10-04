#include "can_filter.hpp"

#include "core/platform.hpp"
#include "core/sdk.hpp"
#include "core/string/string_view.hpp"

using namespace ymd;
using namespace ymd::hal;


void CanFilter::apply(const CanFilterConfig & cfg){

    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {
        .CAN_FilterIdHigh = cfg.id16[1],
        .CAN_FilterIdLow = cfg.id16[0],
        .CAN_FilterMaskIdHigh = cfg.mask16[1],
        .CAN_FilterMaskIdLow = cfg.mask16[0],
        .CAN_FilterFIFOAssignment = CAN_FIFO0,
        .CAN_FilterNumber = nth_count_,
        .CAN_FilterMode = cfg.islist_ ? CAN_FilterMode_IdList : CAN_FilterMode_IdMask,
        .CAN_FilterScale = cfg.is32_ ? CAN_FilterScale_32bit : CAN_FilterScale_16bit,
        .CAN_FilterActivation = ENABLE,
    };

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInit(&CAN_FilterInitSturcture);

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_FIFO1;
    CAN_FilterInit(&CAN_FilterInitSturcture);

}

void CanFilter::deinit(){
    // HALT;
    // TODO();
}


#include "focmotor.hpp"

uint8_t FOCMotorConcept::getDefaultNodeId(){
    auto chip_id = Sys::Chip::getChipIdCrc();
    switch(chip_id){
        case 3273134334:
            return 3;
        case 341554774:
            return 2;
        case 4079188777:
            return 1;
        case 0x551C4DEA:
            return  3;
        case 0x8E268D66:
            return 1;
        default:
            return 0;
    }
}
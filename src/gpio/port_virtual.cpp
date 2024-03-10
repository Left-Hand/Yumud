#include "port_virtual.hpp"


void PortVirtual::writeByIndex(const int8_t index, const bool data){
    if(index < 0 || pin_ptrs[index] == nullptr) return;
    pin_ptrs[index]->write(data);
}

bool PortVirtual::readByIndex(const int8_t index){
    if(index < 0 || pin_ptrs[index] == nullptr) return false;
    return bool(*(pin_ptrs[index]));
}
void PortVirtual::write(const uint16_t & data){
    for(uint8_t i = 0; i < 16; i++){
        writeByIndex(i, bool(data & (1 << i)));
    }
};

void PortVirtual::set(const Pin & pin){
    pin_ptrs[CTZ((uint16_t)pin)]->set();
}

void PortVirtual::clr(const Pin & pin){
    pin_ptrs[CTZ((uint16_t)pin)]->clr();
}

void PortVirtual::setBits(const uint16_t & data){
    for(uint8_t i = 0; i < 16; i++){
        if(data & (1 << i)) pin_ptrs[i]->set();
    }
}

void PortVirtual::clrBits(const uint16_t & data){
    for(uint8_t i = 0; i < 16; i++){
        if(data & (1 << i)) pin_ptrs[i]->clr();
    }
}

const uint16_t PortVirtual::read(){
    uint16_t data = 0;
    for(uint8_t i = 0; i < 16; i++){
        data |= uint16_t(pin_ptrs[i]->read() << i);
    }
    return data;
}

// void PortVirtual::bindPin(Gpio & gpio, const uint8_t index){
//     GpioVirtual::WriteCallback write_callback;
//     GpioVirtual::ReadCallback read_callback;
//     GpioVirtual::ModeCallback mode_callback;

//     switch((uint32_t)gpio.instance){
//         #ifdef HAVE_GPIOA
//         case GPIOA_BASE:
//             write_callback = std::bind(&Port::writeByIndex, &portA, std::placeholders::_1, std::placeholders::_2);
//             read_callback = std::bind(&Port::readByIndex, &portA, std::placeholders::_1);
//             mode_callback = std::bind(&Port::setModeByIndex, &portA, std::placeholders::_1, std::placeholders::_2);
//             break;
//         #endif
//         #ifdef HAVE_GPIOB
//         case GPIOB_BASE:
//             write_callback = std::bind(&Port::writeByIndex, &portB, std::placeholders::_1, std::placeholders::_2);
//             read_callback = std::bind(&Port::readByIndex, &portB, std::placeholders::_1);
//             mode_callback = std::bind(&Port::setModeByIndex, &portB, std::placeholders::_1, std::placeholders::_2);
//             break;
//         #endif
//         #ifdef HAVE_GPIOC
//         case GPIOC_BASE:
//             write_callback = std::bind(&Port::writeByIndex, &portC, std::placeholders::_1, std::placeholders::_2);
//             read_callback = std::bind(&Port::readByIndex, &portC, std::placeholders::_1);
//             mode_callback = std::bind(&Port::setModeByIndex, &portC, std::placeholders::_1, std::placeholders::_2);
//             break;
//         #endif
//         #ifdef HAVE_GPIOD
//         case GPIOD_BASE:
//             write_callback = std::bind(&Port::writeByIndex, &portD, std::placeholders::_1, std::placeholders::_2);
//             read_callback = std::bind(&Port::readByIndex, &portD, std::placeholders::_1);
//             mode_callback = std::bind(&Port::setModeByIndex, &portD, std::placeholders::_1, std::placeholders::_2);
//             break;
//         #endif
//         #ifdef HAVE_GPIOE
//         case GPIOE_BASE:
//             write_callback = std::bind(&Port::writeByIndex, &portE, std::placeholders::_1, std::placeholders::_2);
//             read_callback = std::bind(&Port::readByIndex, &portE, std::placeholders::_1);
//             mode_callback = std::bind(&Port::setModeByIndex, &portE, std::placeholders::_1, std::placeholders::_2);
//             break;
//         #endif
//         default:
//             break;
//     }

//     pin_ptrs[index] = std::make_unique<GpioVirtual>(GpioVirtual(gpio.pin_index, write_callback, read_callback, mode_callback));
// }

void PortVirtual::setModeByIndex(const int8_t & index, const PinMode & mode){
    if(index < 0 || pin_ptrs[index] == nullptr) return;
    pin_ptrs[index]->setMode(mode);
}

void PortVirtual::bindPin(GpioVirtual & gpio, const uint8_t index){
    if(index < 0) return;
    pin_ptrs[index] = std::make_unique<GpioVirtual>(gpio);
}

void PortVirtual::bindPin(Gpio & gpio, const uint8_t index){
    if(index < 0) return;
    pin_ptrs[index] = std::make_unique<GpioVirtual>(GpioVirtual(gpio));
}
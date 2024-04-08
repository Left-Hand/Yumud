#include "gpio.hpp"
#include "port.hpp"
// static void GpioNativeWriteCallback(const int8_t & index, const bool & data){
//     if(data){
//         GPIOC->BSHR = (1 << index);
//     }else{
//         GPIOC->BCR = (1 << index);
//     }
// }

// static bool GpioNativeReadCallback(const int8_t & index){
//     return bool(GPIOC->INDR & index);
// }

PortConcept * GpioVirtual::form_gpiotypedef_to_port(volatile GPIO_TypeDef * _instance){
    switch((uint32_t)_instance){
        default:
        #ifdef HAVE_GPIOA
        case GPIOA_BASE:
            return &portA;
        #endif
        #ifdef HAVE_GPIOB
        case GPIOB_BASE:
            return &portB;
        #endif
        #ifdef HAVE_GPIOC
        case GPIOC_BASE:
            return &portC;
        #endif
        #ifdef HAVE_GPIOD
        case GPIOD_BASE:
            return &portD;
        #endif
        #ifdef HAVE_GPIOE
        case GPIOE_BASE:
            return &portE;
        #endif
    }
}

// static 
// GpioVirtual::WriteCallback GpioVirtual::getNativeWriteCallback(Gpio & gpio){
//     switch((uint32_t)gpio.instance){
//         default:
//         #ifdef HAVE_GPIOA
//         case GPIOA_BASE:
//             return std::bind(&Port::writeByIndex, &portA, std::placeholders::_1, std::placeholders::_2);
//         #endif
//         #ifdef HAVE_GPIOB
//         case GPIOB_BASE:
//             return std::bind(&Port::writeByIndex, &portB, std::placeholders::_1, std::placeholders::_2);
//         #endif
//         #ifdef HAVE_GPIOC
//         case GPIOC_BASE:
//             // return std::bind(&Port::writeByIndex, &portC, std::placeholders::_1, std::placeholders::_2);
//             return GpioNativeWriteCallback;
//         #endif
//         #ifdef HAVE_GPIOD
//         case GPIOD_BASE:
//             return std::bind(&Port::writeByIndex, &portD, std::placeholders::_1, std::placeholders::_2);
//         #endif
//         #ifdef HAVE_GPIOE
//         case GPIOE_BASE:
//             return std::bind(&Port::writeByIndex, &portE, std::placeholders::_1, std::placeholders::_2);
//         #endif
//     }
// }

// GpioVirtual::ReadCallback GpioVirtual::getNativeReadCallback(Gpio & gpio){
//     switch((uint32_t)gpio.instance){
//         default:
//         #ifdef HAVE_GPIOA
//         case GPIOA_BASE:
//             return std::bind(&Port::readByIndex, &portA, std::placeholders::_1);
//         #endif
//         #ifdef HAVE_GPIOB
//         case GPIOB_BASE:
//             return std::bind(&Port::readByIndex, &portB, std::placeholders::_1);
//         #endif
//         #ifdef HAVE_GPIOC
//         case GPIOC_BASE:
//             // return std::bind(&Port::readByIndex, &portC, std::placeholders::_1);
//             return GpioNativeReadCallback;
//         #endif
//         #ifdef HAVE_GPIOD
//         case GPIOD_BASE:
//             return std::bind(&Port::readByIndex, &portD, std::placeholders::_1);
//         #endif
//         #ifdef HAVE_GPIOE
//         case GPIOE_BASE:
//             return std::bind(&Port::readByIndex, &portE, std::placeholders::_1);
//         #endif
//     }
// }

// GpioVirtual::ModeCallback GpioVirtual::getNativeModeCallback(Gpio & gpio){
//     switch((uint32_t)(gpio.instance)){
//         default:
//         #ifdef HAVE_GPIOA
//         case GPIOA_BASE:
//             return std::bind(&Port::setModeByIndex, &portA, std::placeholders::_1, std::placeholders::_2);
//         #endif
//         #ifdef HAVE_GPIOB
//         case GPIOB_BASE:
//             return std::bind(&Port::setModeByIndex, &portB, std::placeholders::_1, std::placeholders::_2);
//         #endif
//         #ifdef HAVE_GPIOC
//         case GPIOC_BASE:
//             return std::bind(&Port::setModeByIndex, &portC, std::placeholders::_1, std::placeholders::_2);
//         #endif
//         #ifdef HAVE_GPIOD
//         case GPIOD_BASE:
//             return std::bind(&Port::setModeByIndex, &portD, std::placeholders::_1, std::placeholders::_2);
//         #endif
//         #ifdef HAVE_GPIOE
//         case GPIOE_BASE:
//             return std::bind(&Port::setModeByIndex, &portE, std::placeholders::_1, std::placeholders::_2);
//         #endif
//     }

// }


// GpioVirtual::GpioVirtual(Gpio & gpio){
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

//     *this = GpioVirtual(gpio.pin_index, write_callback, read_callback, mode_callback);
// }
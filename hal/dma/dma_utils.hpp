#pragma once

#include <cstdint>

namespace ymd{
class OutputStream;
}

namespace ymd::hal{


struct [[nodiscard]] DmaMode{
public:
    enum class Kind:uint8_t{
        ToMemory = 0b000,
        ToPeriph = 0b001,
        Synergy = 0b010,
        Distribute = 0b011,

        ToMemCircular = 0b100,
        ToPeriphCircular = 0b101,
        SynergyCircular = 0b110,
        DistributeCircular = 0b111,
    };

    using enum Kind;

    constexpr DmaMode(Kind kind):kind_(kind){}

    [[nodiscard]] constexpr Kind kind()const{return kind_;}

    [[nodiscard]] constexpr bool is_circular()const{
        return (std::bit_cast<uint8_t>(kind()) & 0b100) != 0;
    }

    [[nodiscard]] constexpr bool dst_is_periph() const {
        switch(kind()){
            case Kind::ToPeriph:
            case Kind::ToPeriphCircular:
                return true;
            default:
                return false;
        }
    }

    friend OutputStream& operator<<(OutputStream& os, DmaMode::Kind kind);
    friend OutputStream& operator<<(OutputStream& os, DmaMode mode){
        return os << mode.kind();}
private:
    Kind kind_;
};

struct [[nodiscard]] DmaDirection{
public:
    enum class Kind:uint8_t{
        ToMemory = 0,
        ToPeriph,
        Synergy,
        Distribute
    };

    using enum Kind;
    constexpr DmaDirection(Kind kind):kind_(kind){}

    constexpr DmaDirection(DmaMode mode):kind_(
        Kind{static_cast<uint8_t>(std::bit_cast<uint8_t>(mode.kind()) & 0b011)}){}

    [[nodiscard]] constexpr Kind kind()const{return kind_;}

    [[nodiscard]] constexpr DmaMode with_circular()const{
        return std::bit_cast<DmaMode>(static_cast<uint8_t>(std::bit_cast<uint8_t>(kind()) | 0b100));
    }

    [[nodiscard]] constexpr DmaMode without_circular()const{
        return std::bit_cast<DmaMode>(static_cast<uint8_t>(std::bit_cast<uint8_t>(kind()) & 0b011));
    }

    friend OutputStream& operator<<(OutputStream& os, DmaDirection::Kind kind);
    friend OutputStream& operator<<(OutputStream& os, DmaDirection direction){
        return os << direction.kind();}
private:
    Kind kind_;
};


enum class DmaPriority:uint16_t{
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3
};


namespace dma_details{
static constexpr uint8_t calculate_dma_index(const void * inst){
    #ifdef ENABLE_DMA2
    return inst < DMA2_Channel1 ? 1 : 2;
    #else
    return 1;
    #endif
}

static constexpr uint8_t calculate_channel_index(const void * inst){
    uint8_t dma_index = calculate_dma_index(inst);
    switch(dma_index){
        #ifdef ENABLE_DMA1
        case 1:
            return (reinterpret_cast<uint32_t>(inst) - DMA1_Channel1_BASE) / 
                (DMA1_Channel2_BASE - DMA1_Channel1_BASE) + 1;
        #endif

        #ifdef ENABLE_DMA2
        case 2:
            if(reinterpret_cast<uint32_t>(inst) < DMA2_Channel7_BASE){ 
                return ((reinterpret_cast<uint32_t>(inst) - DMA2_Channel1_BASE) / 
                    (DMA2_Channel2_BASE - DMA2_Channel1_BASE)) + 1;
            }else{
                return ((reinterpret_cast<uint32_t>(inst) - DMA2_Channel7_BASE) / 
                    (DMA2_Channel8_BASE - DMA2_Channel7_BASE)) + 7;
            }
        #endif
        default:
            __builtin_trap();
    }
}


static constexpr uint32_t calculate_done_mask(const void * inst){
    uint8_t dma_index = calculate_dma_index(inst);
    uint8_t channel_index = calculate_channel_index(inst);
    switch(dma_index){
        #ifdef ENABLE_DMA1
        case 1:
            return (DMA1_IT_TC1 << ((CTZ(DMA1_IT_TC2) - CTZ(DMA1_IT_TC1)) * (channel_index - 1)));
        #endif
        #ifdef ENABLE_DMA2
        case 2:
            if(reinterpret_cast<uint32_t>(inst) <= DMA2_Channel7_BASE){ 
                return ((uint32_t)(DMA2_IT_TC1 & 0xff) << ((CTZ(DMA2_IT_TC2) - CTZ(DMA2_IT_TC1)) * (channel_index - 1))) | (uint32_t)(0x10000000);
            }else{
                return ((uint32_t)(DMA2_IT_TC8 & 0xff) << ((CTZ(DMA2_IT_TC9) - CTZ(DMA2_IT_TC8)) * (channel_index - 8))) | (uint32_t)(0x20000000);
            }
        #endif
        default:
            break;
    }
    return 0;
}


static constexpr uint32_t calculate_half_mask(const void * inst){
    uint8_t dma_index = calculate_dma_index(inst);
    uint8_t channel_index = calculate_channel_index(inst);
    switch(dma_index){
        #ifdef ENABLE_DMA1
        case 1:
            return (DMA1_IT_HT1 << ((CTZ(DMA1_IT_HT2) - CTZ(DMA1_IT_HT1)) * (channel_index - 1)));
        #endif
        #ifdef ENABLE_DMA2
        case 2:
            if(reinterpret_cast<uint32_t>(inst) <= DMA2_Channel7_BASE){ 
                return ((uint32_t)(DMA2_IT_HT1 & 0xff) << ((CTZ(DMA2_IT_HT2) - CTZ(DMA2_IT_HT1)) * (channel_index - 1))) | (uint32_t)(0x10000000);
            }else{
                return ((uint32_t)(DMA2_IT_HT8 & 0xff) << ((CTZ(DMA2_IT_HT9) - CTZ(DMA2_IT_HT8)) * (channel_index - 8))) | (uint32_t)(0x20000000);
            }
        #endif
        default:
            break;
    }
    return 0;
}
}
}
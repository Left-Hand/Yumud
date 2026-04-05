#pragma once


#include "core/utils/Result.hpp"
#include "primitive/can/can_id.hpp"


// 参考链接
// https://github.com/waas-rent/vesc_can_sdk/blob/main/CAN_PROTOCOL.md


namespace ymd::robots::vesc::can{

enum class PacketId:uint16_t{
	SetDuty						        = 0,
	SetCurrent					        = 1,
	SetCurrentBrake			        = 2,
	SetRpm						        = 3,
	SetPos						        = 4,
	FillRxBuffer				        = 5,
	FillRxBufferLong			        = 6,
	ProcessRxBuffer			        = 7,
	ProcessShortBuffer			        = 8,
	Status1						        = 9,
	SetCurrentRel				        = 10,
	SetCurrentBrakeRel		        = 11,
	SetCurrentHandbrake		        = 12,
	SetCurrentHandbrakeRel	        = 13,
	Status2						        = 14,
	Status3						        = 15,
	Status4						        = 16,
	Ping							        = 17,
	Pong							        = 18,
	DetectApplyAllFoc			        = 19,
	DetectApplyAllFocRes		        = 20,
	ConfCurrentLimits			        = 21,
	ConfStoreCurrentLimits	        = 22,
	ConfCurrentLimitsIn		        = 23,
	ConfStoreCurrentLimitsIn	        = 24,
	ConfFocErpms				        = 25,
	ConfStoreFocErpms			        = 26,
	Status5						        = 27,
	PollTS5700N8501Status		        = 28,
	ConfBatteryCut				        = 29,
	ConfStoreBatteryCut		        = 30,
	Shutdown						        = 31,
	IoBoardAdc1To4			        = 32,
	IoBoardAdc5To8			        = 33,
	IoBoardAdc9To12			        = 34,
	IoBoardDigitalIn			        = 35,
	IoBoardSetOutputDigital	        = 36,
	IoBoardSetOutputPwm		        = 37,
	BmsVTot					        = 38,
	BmsI						        = 39,
	BmsAhWh					        = 40,
	BmsVCell					        = 41,
	BmsBal						        = 42,
	BmsTemps					        = 43,
	BmsHum						        = 44,
	BmsSocSohTempStat		        = 45,
	PswStat						        = 46,
	PswSwitch					        = 47,
	BmsHwData1				        = 48,
	BmsHwData2				        = 49,
	BmsHwData3				        = 50,
	BmsHwData4				        = 51,
	BmsHwData5				        = 52,
	BmsAhWhChgTotal			        = 53,
	BmsAhWhDisTotal			        = 54,
	UpdatePidPosOffset		        = 55,
	PollRotorPos				        = 56,
	NotifyBoot					        = 57,
	Status6						        = 58,
	GnssTime					        = 59,
	GnssLat						        = 60,
	GnssLon						        = 61,
	GnssAltSpeedHdop			        = 62,
	UpdateBaud			        = 63,
};


static constexpr hal::CanExtId encode_can_id(const PacketId packet_id, const uint8_t controller_id){
    const uint32_t id_u29 = static_cast<uint32_t>(packet_id) << 8 | controller_id;
    return hal::CanExtId::from_u29(id_u29);
}

template<typename T, size_t RATIO>
struct alignas(sizeof(T)) [[nodiscard]] Scaled final{
    using Self = Scaled;
    static constexpr bool IS_SIGNED = std::is_signed_v<T>;

    static constexpr float MAX_F = static_cast<long double>(std::numeric_limits<T>::max()) / RATIO;
    static constexpr float MIN_F = static_cast<long double>(std::numeric_limits<T>::min()) / RATIO;

    T bits;

    static constexpr Result<Self, std::strong_ordering> try_from_float(const float x){
        if(x > MAX_F) return Err(std::strong_ordering::greater);
        if(x < MIN_F) return Err(std::strong_ordering::less);
        return Ok(Self{static_cast<T>(x * RATIO)});
    }

    static constexpr Self from_float(const float x){
        if(x > MAX_F) return Err(std::strong_ordering::greater);
        if(x < MIN_F) return Err(std::strong_ordering::less);
        return Ok(Self{static_cast<T>(x * RATIO)});
    }

    constexpr float to_float() const {
        return static_cast<float>(bits) / RATIO;
    }
};

template<typename T, size_t RATIO>
struct alignas(sizeof(T)) [[nodiscard]] InvScaled final{
    T bits;
};

struct [[nodiscard]] alignas(8) U64Bitset final{
    uint64_t bits;

    constexpr void set_high(const size_t index){
        if(index > 63) __builtin_trap();
        bits |= (1ull << index);
    }

    constexpr void set_low(const size_t index){
        if(index > 63) __builtin_trap();
        const uint64_t mask = ~(1ull << index);
        bits &= mask;
    }
};

struct [[nodiscard]] alignas(4) TS5700N8501Status final{
    uint8_t sf;
    uint8_t abs0;
    uint8_t abs1;
    uint8_t abs2;
    uint8_t abm0;
    uint8_t abm1;
    uint8_t abm2;
    uint8_t almc;

    [[nodiscard]] constexpr std::span<const uint8_t, 8> as_bytes() const {
        return std::span<const uint8_t, 8>(&this->sf, 8);
    }

    [[nodiscard]] constexpr std::span<uint8_t, 8> as_bytes_mut() {
        return std::span<uint8_t, 8>(&this->sf, 8);
    }
};


}
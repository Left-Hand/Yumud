#pragma once

#include <cstdint>
#include <bit>

namespace ymd::dsp{
class [[nodiscard]] DebounceFilter final{
public:

	struct [[nodiscard]] Config final{
		uint8_t flip_threshold;
		uint8_t pipe_length;

		static constexpr Config from_default(){
			return Config{
				.flip_threshold = 2,
				.pipe_length = 8,
			};
		}
	};

	constexpr explicit DebounceFilter(const Config & cfg):
		flip_threshold(cfg.flip_threshold),
		pipe_length(cfg.pipe_length){
	}

	constexpr void update(const bool input){
		bool is_match = (input == true);
		bool shift_out = bool(history & (1 << (pipe_length - 1)));

		match_cnts += is_match - shift_out;
		uint8_t invalid_cnts = pipe_length - match_cnts;
		history = (history << 1) | is_match;

		switch(state){
		case State::Low:
			if(match_cnts >= flip_threshold) state = State::MayHigh;
			break;
		case State::High:
			if(invalid_cnts >= flip_threshold) state = State::MayLow;
			break;
		case State::MayLow:
		case State::MayHigh:
			if(invalid_cnts == 0) state = State::High;
			else if(match_cnts == 0) state = State::Low;
			break;
		default:
			break;
		}
		// resu = (uint8_t)state & 0x01;
	}

	[[nodiscard]] constexpr bool is_active(){
		return std::bit_cast<uint8_t>(state) & 0x01;
	}

private:


	uint32_t history = 0;

	enum class State:uint8_t{
		Low, High, MayLow, MayHigh
	};

	uint8_t flip_threshold;
	uint8_t pipe_length;

	State state = State::Low;

	uint8_t match_cnts = 0;
};

}
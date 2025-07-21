#pragma once

#include <cstdint>
#include <bit>

namespace ymd::dsp{
class DebounceFilter{
public:

	struct Config{
		const uint8_t pipe_length = 8;
		const uint8_t threshold = 2;
		const bool polarity = true;
	};

	constexpr explicit DebounceFilter(const Config & cfg):
		pipe_length(cfg.pipe_length), 
		threshold(cfg.threshold), 
		polarity(cfg.polarity){
	}

	constexpr void update(const bool input){
		bool valid = (input == polarity);
		bool shift_out = bool(data & (1 << (pipe_length - 1)));

		valid_cnts += valid - shift_out;
		uint8_t invalid_cnts = pipe_length - valid_cnts;
		data = (data << 1) | valid;

		switch(state){
		case State::Low:
			if(valid_cnts >= threshold) state = State::MayHigh;
			break;
		case State::High:
			if(invalid_cnts >= threshold) state = State::MayLow;
			break;
		case State::MayLow:
		case State::MayHigh:
			if(invalid_cnts == 0) state = State::High;
			else if(valid_cnts == 0) state = State::Low;
		default:
			break;
		}
		// resu = (uint8_t)state & 0x01;
	}

	constexpr bool is_high(){
		return std::bit_cast<uint8_t>(state) & 0x01;
	}

private:
	uint32_t data = 0;
	uint8_t pipe_length;
	uint8_t threshold;

	enum class State:uint8_t{
		Low, High, MayLow, MayHigh
	};
	State state = State::Low;
	bool polarity;

	uint8_t valid_cnts = 0;
};

}
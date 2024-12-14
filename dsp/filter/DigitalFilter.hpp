#pragma once

#include <cstdint>

class DigitalFilter{
protected:
	uint32_t data = 0;
	uint8_t size;
	uint8_t threshold;

	enum class State:uint8_t{
		Low, High, MayLow, MayHigh
	};
	State state = State::Low;
	bool polarity;

	uint8_t valid_cnts = 0;

public:

	DigitalFilter(const uint8_t _size = 8, const uint8_t _threshold = 2,
			const bool _polarity = true):size(_size), threshold(_threshold), polarity(_polarity){
	}

	bool update(const bool input){
		bool valid = (input == polarity);
		bool shift_out = bool(data & (1 << (size - 1)));

		valid_cnts += valid - shift_out;
		uint8_t invalid_cnts = size - valid_cnts;
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
		bool ret = (uint8_t)state & 0x01;
		return ret;
	}
};
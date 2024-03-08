#ifndef __NEC_ENCODER_HPP__
#define __NEC_ENCODER_HPP__

#include <cstdint>
#include <functional>

class NecEncoder{
protected:
	uint8_t bit_prog;
	uint8_t byte_prog;
    std::function<void(const bool)> setter;

	struct NecCode{
		uint8_t total_cnt;
		uint8_t valid_cnt;
	};

	const NecCode codes[4] = {
		{.total_cnt = 24, .valid_cnt = 16},
		{.total_cnt = 2, .valid_cnt = 1},
		{.total_cnt = 4, .valid_cnt = 1},
		{.total_cnt = 2, .valid_cnt = 1}
	};

	enum class BitType:uint8_t{
		Leader, Zero, One, Stop
	};

	enum class EncodeProg:uint8_t{
		Idle, Lead, Address, invAddress, Command, invCommand, Stop
	};

	bool writeBit(const BitType & bit){
		bit_prog ++;
		const NecCode & code = codes[(uint8_t)bit];

		setter(bit_prog <= code.valid_cnt);

		bool ret = false;
		if(bit_prog >= code.total_cnt){
			bit_prog = 0;
			ret = true;
		}
		return ret;
	}
	bool writeByte(const uint8_t & byte){

		if(writeBit((byte & (0x01 << byte_prog)) ? BitType::One : BitType::Zero))
			byte_prog ++;

		bool ret = false;

		if(byte_prog >= 8){
			byte_prog = 0;
			ret = true;
		}

		return ret;
	};
public:
	uint8_t address = 0xAA;
	uint8_t command = 0xCC;
	EncodeProg encode_prog;

    NecEncoder(std::function<void(const bool)> _setter)setter(_setter){;}
	bool tick(){
		switch(encode_prog){
		case EncodeProg::Lead:
			if(writeBit(BitType::Leader)){
				encode_prog = EncodeProg::Address;
			}
			break;

		case EncodeProg::Address:
			if(writeByte(address)){
				encode_prog = EncodeProg::invAddress;
			}
			break;
		case EncodeProg::invAddress:
			if(writeByte(~address)){
				encode_prog = EncodeProg::Command;
			}
			break;
		case EncodeProg::Command:
			if(writeByte(command)){
				encode_prog = EncodeProg::invCommand;
			}
			break;
		case EncodeProg::invCommand:
			if(writeByte(~command)){
				encode_prog = EncodeProg::Stop;
			}
			break;
		case EncodeProg::Stop:
			if(writeBit(BitType::Stop)){
				encode_prog = EncodeProg::Idle;
			}
            // setIrState(false);
			break;
		default:
			break;
		}

		return (encode_prog == EncodeProg::Idle);
	}

	void emit(const uint8_t & _address, const uint8_t & _command){
		address = _address;
		command = _command;

		encode_prog = EncodeProg::Lead;
	}
};

#endif
#pragma once

// https://github.com/matrixcascade/PainterEngine/blob/master/core/AES.h

#include "core/utils/Result.hpp"

namespace ymd::encrypt::aes{

static constexpr size_t  BLOCK_BYTES_SIZE		= 16;
static constexpr size_t  SEED_KEY_SIZE			= 32;
static constexpr size_t  KEYSCHEDULE_MAX_ROW	    = 60;
static constexpr size_t  KEYSCHEDULE_MAX_COLUMN =  4;
static constexpr size_t  STATE_MATRIX_SIZE	= 	4;

namespace details{

///////////////////////////////////////////////////////////////////////////
//				GF(2^8) Operation....
//////////////////////////////////////////////////////////////////////////
static constexpr uint8_t gfmultby01(uint8_t b){
    return b;
}

static constexpr uint8_t gfmultby02(uint8_t b){
    if (b < 0x80)
        return (uint8_t)(int)(b <<1);
    else
        return (uint8_t)( (int)(b << 1) ^ (int)(0x1b) );
}

static constexpr uint8_t gfmultby03(uint8_t b){
    return (uint8_t) ( (int)gfmultby02(b) ^ (int)b );
}

static constexpr uint8_t gfmultby09(uint8_t b){
    return (uint8_t)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
        (int)b );
}

static constexpr uint8_t gfmultby0b(uint8_t b){
    return (uint8_t)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
        (int)gfmultby02(b) ^
        (int)b );
}

static constexpr uint8_t gfmultby0d(uint8_t b){
    return (uint8_t)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
        (int)gfmultby02(gfmultby02(b)) ^
        (int)(b) );
}

static constexpr uint8_t gfmultby0e(uint8_t b){
    return (uint8_t)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
        (int)gfmultby02(gfmultby02(b)) ^
        (int)gfmultby02(b) );
}




static constexpr uint8_t Sbox[][16]={  // populate the Sbox matrix
	/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
	/*0*/  {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
	/*1*/  {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
	/*2*/  {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
	/*3*/  {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
	/*4*/  {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
	/*5*/  {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
	/*6*/  {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
	/*7*/  {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
	/*8*/  {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
	/*9*/  {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
	/*a*/  {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
	/*b*/  {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
	/*c*/  {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
	/*d*/  {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
	/*e*/  {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
	/*f*/  {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}, 
};

static constexpr uint8_t iSbox[][16]= {  // populate the iSbox matrix
	/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
	/*0*/  {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
	/*1*/  {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
	/*2*/  {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
	/*3*/  {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
	/*4*/  {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
	/*5*/  {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
	/*6*/  {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
	/*7*/  {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
	/*8*/  {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
	/*9*/  {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
	/*a*/  {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
	/*b*/  {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
	/*c*/  {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
	/*d*/  {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
	/*e*/  {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
	/*f*/  {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d} 
};

const uint8_t Rcon[][4]= 
{ 
	{0x00, 0x00, 0x00, 0x00},  
	{0x01, 0x00, 0x00, 0x00},
	{0x02, 0x00, 0x00, 0x00},
	{0x04, 0x00, 0x00, 0x00},
	{0x08, 0x00, 0x00, 0x00},
	{0x10, 0x00, 0x00, 0x00},
	{0x20, 0x00, 0x00, 0x00},
	{0x40, 0x00, 0x00, 0x00},
	{0x80, 0x00, 0x00, 0x00},
	{0x1b, 0x00, 0x00, 0x00},
	{0x36, 0x00, 0x00, 0x00} 
};


}

enum class KeySize:uint8_t{
	_128Bits, _192Bits, _256Bits
};



struct AES{
    using State = std::array<std::array<uint8_t, STATE_MATRIX_SIZE>, STATE_MATRIX_SIZE>;

    enum class Error:uint8_t{
        BufferIsNotAlignedTo16,
        BufferSizeNotMatching
    };

    friend OutputStream & operator <<(OutputStream & os, Error error){
        switch(error){
            case Error::BufferIsNotAlignedTo16:
                return os << "BufferIsNotAlignedTo16";
            case Error::BufferSizeNotMatching:
                return os << "BufferSizeNotMatching";
            }
        __builtin_unreachable();
    } 

    [[nodiscard]] static constexpr AES from_128bits(std::span<const uint8_t, 16> key_buffer){
        AES ret;
        ret.compute(KeySize::_128Bits, key_buffer.data());
        return ret;
    }

    [[nodiscard]] static constexpr AES from_192bits(std::span<const uint8_t, 24> key_buffer){
        AES ret;
        ret.compute(KeySize::_192Bits, key_buffer.data());
        return ret;
    }

    [[nodiscard]] static constexpr AES from_256bits(std::span<const uint8_t, 32> key_buffer){
        AES ret;
        ret.compute(KeySize::_256Bits, key_buffer.data());
        return ret;
    }

    constexpr Result<void, Error> cipher(std::span<uint8_t> output,std::span<const uint8_t> input){
        const size_t in_size = input.size();
        const size_t out_size = output.size();
        if(in_size != out_size) return Err(Error::BufferSizeNotMatching);
        if(in_size & 0b1111) return Err(Error::BufferIsNotAlignedTo16);

        for (size_t i=0;i<in_size/16;i++){
            cipher_block(input.data()+i*16,output.data()+i*16);
        }
        return Ok();
    }



    constexpr Result<void, Error> inv_cipher(std::span<uint8_t> output,std::span<const uint8_t> input){
        const size_t in_size = input.size();
        const size_t out_size = output.size();
        if(in_size != out_size) return Err(Error::BufferSizeNotMatching);
        if(in_size & 0b1111) return Err(Error::BufferIsNotAlignedTo16);

        for (size_t i = 0;i < in_size/16;i++){
            inv_cipher_block(input.data() + i*16,output.data() + i*16);
        }
        return Ok();
    }
private:

	uint8_t				w[KEYSCHEDULE_MAX_ROW][KEYSCHEDULE_MAX_COLUMN];						// key_buffer schedule array. 
	size_t					Nk;																			// key_buffer size in 32-bit words.  4, 6, 8.  (128, 192, 256 bits).
	size_t					Nr;																			// number of rounds. 10, 12, 14.

    constexpr void compute(KeySize key_size, const uint8_t * key_buffer){
        using namespace details;
        
        std::tie(Nk, Nr) = keysize_to_nk_and_nr(key_size);

        std::array<uint8_t, 4> temp;

        for (size_t row = 0; row < this->Nk; ++row)
        {
            this->w[row][0] = key_buffer[4*row];
            this->w[row][1] = key_buffer[4*row+1];
            this->w[row][2] = key_buffer[4*row+2];
            this->w[row][3] = key_buffer[4*row+3];
        }

        for (size_t row = this->Nk; row < 4 * (this->Nr+1); ++row)
        {
            temp[0] = this->w[row-1][0]; 
            temp[1] = this->w[row-1][1];
            temp[2] = this->w[row-1][2]; 
            temp[3] = this->w[row-1][3];

            if (row % this->Nk == 0)  
            {
                temp = SubWord(RotWord(temp));

                temp[0] = (uint8_t)( (int)temp[0] ^ (int)Rcon[row/this->Nk][0] );
                temp[1] = (uint8_t)( (int)temp[1] ^ (int)Rcon[row/this->Nk][1] );
                temp[2] = (uint8_t)( (int)temp[2] ^ (int)Rcon[row/this->Nk][2] );
                temp[3] = (uint8_t)( (int)temp[3] ^ (int)Rcon[row/this->Nk][3] );
            }
            else if ( this->Nk > 6 && (row % this->Nk == 4) )  
            {
                temp = SubWord(temp);
            }

            // w[row] = w[row-Nk] xor temp
            this->w[row][0] = (uint8_t) ( (int)this->w[row-this->Nk][0] ^ (int)temp[0] );
            this->w[row][1] = (uint8_t) ( (int)this->w[row-this->Nk][1] ^ (int)temp[1] );
            this->w[row][2] = (uint8_t) ( (int)this->w[row-this->Nk][2] ^ (int)temp[2] );
            this->w[row][3] = (uint8_t) ( (int)this->w[row-this->Nk][3] ^ (int)temp[3] );

        }  // for loop
    }

    void cipher_block(const uint8_t input[16], uint8_t output[16]){
        i_Cipher(input,output);
    }


    void inv_cipher_block(const uint8_t input[16], uint8_t output[16]){
        i_InvCipher(input,output);
    }

    [[nodiscard]] static constexpr std::tuple<size_t, size_t> keysize_to_nk_and_nr(const KeySize key_size){
        switch (key_size){
            case KeySize::_128Bits:
                return {4, 10};
            case KeySize::_192Bits:
                return {6, 12};
            case KeySize::_256Bits:
                return {8, 14};
        default:
            __builtin_unreachable();
        }
    }


    constexpr void i_Cipher(const uint8_t input[BLOCK_BYTES_SIZE], uint8_t output[BLOCK_BYTES_SIZE]) const {
        State state;
        for (size_t i = 0; i < (4 * 4); ++i){
            state[i % 4][i / 4] = input[i];
        }

        state = AddRoundKey(state, 0);

        for (size_t round = 1; round <= (this->Nr - 1); ++round)  // main round loop
        {
            state = SubBytes(state); 
            state = ShiftRows(state);  
            state = MixColumns(state); 
            state = AddRoundKey(state, round);
        }  // main round loop

        state = SubBytes(state);
        state = ShiftRows(state);
        state = AddRoundKey(state, this->Nr);

        // output = state
        for (size_t i = 0; i < (4 * 4); ++i){
            output[i] = state[i % 4][i / 4];
        }
    }  // cipher_block()




    constexpr void i_InvCipher(const uint8_t input[BLOCK_BYTES_SIZE], uint8_t output[BLOCK_BYTES_SIZE]) const {
        State state;
        for (size_t i = 0; i < (4 * 4); ++i)
        {
            state[i % 4][i / 4] = input[i];
        }

        state = AddRoundKey(state, Nr);

        for (size_t round = this->Nr-1; round >= 1; --round)  // main round loop
        {
            state = InvShiftRows(state);
            state = InvSubBytes(state);
            state = AddRoundKey(state, round);
            state = InvMixColumns(state);
        }  // end main round loop for inv_cipher_block

        state = InvShiftRows(state);
        state = InvSubBytes(state);
        state = AddRoundKey(state, 0);

        // output = state
        for (size_t i = 0; i < (4 * 4); ++i){
            output[i] = state[i % 4][i / 4];
        }

    }  // inv_cipher_block()



    [[nodiscard]] __inline static constexpr 
    State InvSubBytes(const State & state){
        using namespace details;
        State ret;
        for (size_t r = 0; r < 4; ++r){
            for (size_t c = 0; c < 4; ++c){
                ret[r][c] = iSbox[ (state[r][c] >> 4)][(state[r][c] & 0x0f) ];
            }
        }
        return ret;
    }  // InvSubBytes


    [[nodiscard]] __inline static constexpr  
    State InvMixColumns(const State & state){
        using namespace details;

        State ret;

        for (size_t c = 0; c < 4; ++c)
        {
            ret[0][c] = (uint8_t) ( (int)gfmultby0e(state[0][c]) ^ (int)gfmultby0b(state[1][c]) ^
                (int)gfmultby0d(state[2][c]) ^ (int)gfmultby09(state[3][c]) );
            ret[1][c] = (uint8_t) ( (int)gfmultby09(state[0][c]) ^ (int)gfmultby0e(state[1][c]) ^
                (int)gfmultby0b(state[2][c]) ^ (int)gfmultby0d(state[3][c]) );
            ret[2][c] = (uint8_t) ( (int)gfmultby0d(state[0][c]) ^ (int)gfmultby09(state[1][c]) ^
                (int)gfmultby0e(state[2][c]) ^ (int)gfmultby0b(state[3][c]) );
            ret[3][c] = (uint8_t) ( (int)gfmultby0b(state[0][c]) ^ (int)gfmultby0d(state[1][c]) ^
                (int)gfmultby09(state[2][c]) ^ (int)gfmultby0e(state[3][c]) );
        }
        return ret;
    }  // InvMixColumns


    [[nodiscard]] __inline static constexpr  
    State MixColumns(const State & state){
        using namespace details;
        State ret;
        for (size_t c = 0; c < 4; ++c)
        {
            ret[0][c] = (uint8_t) ( (int)gfmultby02(state[0][c]) ^ (int)gfmultby03(state[1][c]) ^
                (int)gfmultby01(state[2][c]) ^ (int)gfmultby01(state[3][c]) );
            ret[1][c] = (uint8_t) ( (int)gfmultby01(state[0][c]) ^ (int)gfmultby02(state[1][c]) ^
                (int)gfmultby03(state[2][c]) ^ (int)gfmultby01(state[3][c]) );
            ret[2][c] = (uint8_t) ( (int)gfmultby01(state[0][c]) ^ (int)gfmultby01(state[1][c]) ^
                (int)gfmultby02(state[2][c]) ^ (int)gfmultby03(state[3][c]) );
            ret[3][c] = (uint8_t) ( (int)gfmultby03(state[0][c]) ^ (int)gfmultby01(state[1][c]) ^
                (int)gfmultby01(state[2][c]) ^ (int)gfmultby02(state[3][c]) );
        }

        return ret;
    }  // MixColumns


    [[nodiscard]] __inline static constexpr  
    State SubBytes(const State& state){
        State ret;
        using namespace details;
        for (size_t r = 0; r < 4; ++r){
            for (size_t c = 0; c < 4; ++c){
                ret[r][c] = Sbox[ (state[r][c] >> 4)][(state[r][c] & 0x0f) ];
            }
        }
        return ret;
    }

    [[nodiscard]] __inline static constexpr 
    State ShiftRows(const State& state){
        State ret;
        ret[0] = state[0]; // 第0行不移位
        ret[1] = rotate_row_left<1>(state[1]); // 第1行左移1位
        ret[2] = rotate_row_left<2>(state[2]); // 第2行左移2位
        ret[3] = rotate_row_left<3>(state[3]); // 第3行左移3位
        return ret;
    }

    [[nodiscard]] __inline static constexpr 
    State InvShiftRows(const State& state){
        State ret;
        ret[0] = state[0]; // 第0行不移位
        ret[1] = rotate_row_right<1>(state[1]); // 第1行右移1位
        ret[2] = rotate_row_right<2>(state[2]); // 第2行右移2位
        ret[3] = rotate_row_right<3>(state[3]); // 第3行右移3位
        return ret;
    }

    [[nodiscard]] static constexpr __inline 
    std::array<uint8_t, 4> SubWord(const std::array<uint8_t, 4> word){
        using namespace details;
        std::array<uint8_t, 4> result;
        result[0] = Sbox[ word[0] >> 4][word[0] & 0x0f ];
        result[1] = Sbox[ word[1] >> 4][word[1] & 0x0f ];
        result[2] = Sbox[ word[2] >> 4][word[2] & 0x0f ];
        result[3] = Sbox[ word[3] >> 4][word[3] & 0x0f ];
        return result;
    }

    [[nodiscard]] static constexpr __inline 
    std::array<uint8_t, 4> RotWord(const std::array<uint8_t, 4> word){
        std::array<uint8_t, 4> result;
        result[0] = word[1];
        result[1] = word[2];
        result[2] = word[3];
        result[3] = word[0];
        return result;
    }

    [[nodiscard]] constexpr __inline 
    State AddRoundKey(State state, size_t round) const {
        State ret;
        for (size_t r = 0; r < 4; ++r){
            for (size_t c = 0; c < 4; ++c){
                ret[r][c] = (uint8_t) ( (int)state[r][c] ^ (int)this->w[(round*4)+c][r] );
            }
        }
        return ret;
    }

    template<int I>
    [[nodiscard]] __fast_inline static constexpr 
    std::array<uint8_t, 4> rotate_row_left(const std::array<uint8_t, 4> src){
        static_assert(I < 4, "Row index must be less than 4");
        return {
            src[(0 + I) % 4],
            src[(1 + I) % 4],
            src[(2 + I) % 4],
            src[(3 + I) % 4]
        };
    }

    // 需要添加右旋函数：
    template<int I>
    [[nodiscard]] __fast_inline static constexpr 
    std::array<uint8_t, 4> rotate_row_right(const std::array<uint8_t, 4> src){
        static_assert(I < 4, "Shift amount must be less than 4");
        return {
            src[(4 - I) % 4],
            src[(5 - I) % 4], 
            src[(6 - I) % 4],
            src[(7 - I) % 4]
        };
    }
};


}
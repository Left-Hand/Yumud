#pragma once


// MIT LICENISE
// https://github.com/matrixcascade/PainterEngine/blob/master/core/PX_Sha256.c

//  Implementation of SHA256 hash function.
//  Original author: Tom St Denis, tomstdenis@gmail.com, http://libtom.org
//  Modified by WaterJuice retaining Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org


#include <cstdint>
#include <span>
#include <algorithm>

namespace ymd::encrypt::sha256{

namespace details{

static constexpr void STORE32H(uint32_t x, std::span<uint8_t, 4> y){
    y[0] = (uint8_t)(((x)>>24) & 0xff);
    y[1] = (uint8_t)(((x)>>16) & 0xff);
    y[2] = (uint8_t)(((x)>>8) & 0xff);
    y[3] = (uint8_t)((x) & 0xff);
}

static constexpr uint32_t LOAD32H(std::span<const uint8_t, 4> y){
    return (((uint32_t)((y)[0] & 0xff)<<24) |
            ((uint32_t)((y)[1] & 0xff)<<16) |
            ((uint32_t)((y)[2] & 0xff)<<8)  |
            ((uint32_t)((y)[3] & 0xff)));

}

static constexpr void STORE64H(uint64_t x, std::span<uint8_t, 8> y){ 
    STORE32H(static_cast<uint32_t>(x >> 32), y.subspan<0, 4>());
    STORE32H(static_cast<uint32_t>(x), y.subspan<4, 4>());
}

// The K array
static constexpr std::array<uint32_t, 64> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

template<size_t n>
static constexpr uint32_t ror(uint32_t x) {
    return (x >> n) | (x << (32 - n));
}

// Various logical functions
static constexpr uint32_t Ch(uint32_t x,uint32_t y,uint32_t z ){return (z ^ (x & (y ^ z)));}
static constexpr uint32_t Maj(uint32_t x,uint32_t y,uint32_t z ){return (((x | y) & z) | (x & y));}

template<size_t n>  
static constexpr uint32_t S(uint32_t x){return ror<n>(x);}
template<size_t n>  
static constexpr uint32_t R(uint32_t x){return (((x)&0xFFFFFFFF)>>(n));}

static constexpr uint32_t Sigma0(uint32_t x ){return (S<2>(x) ^ S<13>(x) ^ S<22>(x));}
static constexpr uint32_t Sigma1(uint32_t x ){return (S<6>(x) ^ S<11>(x) ^ S<25>(x));}

static constexpr uint32_t Gamma0(uint32_t x ){return (S<7>(x) ^ S<18>(x) ^ R<3>(x));}
static constexpr uint32_t Gamma1(uint32_t x ){return (S<17>(x) ^ S<19>(x) ^ R<10>(x));}


}

static constexpr size_t SHA256_BYTES = ( 256 / 8 );
static constexpr size_t BLOCK_SIZE = 64;

struct Sha256Context{
    static constexpr std::array<uint32_t, 8> INITIAL_STATE = {
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
        0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
    };

    constexpr void update(const std::span<const uint8_t> pbuf){
        uint32_t n;

        if( curlen > BLOCK_SIZE ) return;

        auto Buffer = pbuf.data();
        auto BufferSize = static_cast<int32_t>(pbuf.size());

        while( BufferSize > 0 ){
            if((curlen == 0) and (static_cast<size_t>(BufferSize) >= BLOCK_SIZE)){
                transform(std::span<const uint8_t, BLOCK_SIZE>(Buffer, BLOCK_SIZE));
                length += BLOCK_SIZE * 8;
                Buffer = Buffer + BLOCK_SIZE;
                BufferSize -= BLOCK_SIZE;
            }else{
                n = std::min<int32_t>( BufferSize, (BLOCK_SIZE - curlen) );
                // memcpy( buf + curlen, (void *)Buffer, (int)n );
                std::copy( Buffer, Buffer + n, buf.data() + curlen );

                curlen += n;
                Buffer = Buffer + n;
                BufferSize -= n;
                if( curlen == BLOCK_SIZE )
                {
                    transform(std::span(buf));
                    length += 8*BLOCK_SIZE;
                    curlen = 0;
                }
            }
        }
    }

    constexpr void deinit(std::span<uint8_t, SHA256_BYTES> digest){

        if( curlen >= BLOCK_SIZE ) return;

        // Increase the length of the message
        length += curlen * 8;

        // Append the '1' bit
        buf[curlen++] = 0x80;

        // if the length is currently above 56 bytes we append zeros
        // then compress.  Then we can fall back to padding zeros and length
        // encoding like normal.
        if( curlen > 56 ){
            while( curlen < BLOCK_SIZE )
            {
                buf[curlen++] = 0;
            }
            transform(std::span(buf));
            curlen = 0;
        }

        // Pad up to 56 bytes of zeroes
        while( curlen < 56 ){
            buf[curlen++] = 0;
        }

        // Store length
        details::STORE64H( length, std::span<uint8_t, 8>(buf.data()+56, 8));
        transform(std::span(buf));

        // Copy output
        for(size_t i=0; i<8; i++ ){
            details::STORE32H(state[i], std::span<uint8_t, 4>(digest.data() + (4*i), 4) );
        }
    }

private:
    uint64_t    length = 0;
    std::array<uint32_t, 8>    state = INITIAL_STATE;
    uint32_t    curlen = 0;
    std::array<uint8_t, BLOCK_SIZE>     buf = {};

    constexpr void transform(std::span<const uint8_t, BLOCK_SIZE> block){
        std::array<uint32_t, 8> S = state;
        const auto W = [&]()-> std::array<uint32_t, BLOCK_SIZE> {
            std::array<uint32_t, BLOCK_SIZE> ret;

            // Copy the state into 512-bits into W[0..15]
            for(size_t i=0; i < 16; i++ ){
                ret[i] = details::LOAD32H(std::span<const uint8_t, 4>(block.data() + (4*i), 4) );
            }

            // Fill W[16..63]
            for(size_t i=16; i < BLOCK_SIZE; i++ ){
                ret[i] = details::Gamma1( ret[i-2]) + ret[i-7] + details::Gamma0( ret[i-15] ) + ret[i-16];
            }

            return ret;
        }();

        auto ring_shift_S = [&]{
            const uint32_t t = S[7];
            S[7] = S[6];
            S[6] = S[5];
            S[5] = S[4];
            S[4] = S[3];
            S[3] = S[2];
            S[2] = S[1];
            S[1] = S[0];
            S[0] = t;
        };


        // Compress
        for (size_t i = 0; i < BLOCK_SIZE; i++) {
            const uint32_t t0 = S[7] + details::Sigma1(S[4]) 
                + details::Ch(S[4], S[5], S[6]) 
                + details::K[i] + W[i]; 
            const uint32_t t1 = details::Sigma0(S[0]) 
                + details::Maj(S[0], S[1], S[2]); 
            S[3] += t0; 
            S[7] = t0 + t1;

            ring_shift_S();
        }

        // Feedback
        for (size_t i = 0; i < 8; i++) {
            state[i] += S[i];
        }
    }
};

constexpr void calculate_sha256(
    std::span<uint8_t, SHA256_BYTES> digest,          // [in]
    std::span<const uint8_t> pbuf
){
    Sha256Context context;

    context.update(pbuf);
    context.deinit(digest);
}

static constexpr std::array<uint8_t, SHA256_BYTES> calculate_sha256(std::span<const uint8_t> pbuf) {
    std::array<uint8_t, SHA256_BYTES> digest{};
    calculate_sha256(digest, pbuf);
    return digest;
}



namespace test{

#if 0
static constexpr void test_sha256(){ 
    constexpr auto input = std::array<uint8_t, 5>{0x61, 0x62, 0x63, 0x64, 0x65};
    constexpr auto digest = calculate_sha256(std::span(input));
    // static_assert(digest[0] == 0);
}
#endif

}
}
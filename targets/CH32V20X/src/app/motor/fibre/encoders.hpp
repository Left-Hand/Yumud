#pragma once

#include "protocol.hpp"
#include "crc.hpp"
#include "utils.hpp"
#include <utility>

namespace ymd::fibre{
struct Request
{
    endpoint_id_t endpoint_id;
    size_t length;
};

/* Base classes --------------------------------------------------------------*/

// @brief Base class for all stream encoders
// A stream based encoder is an encoder that generates arbitrary length data blocks.
class StreamEncoder : public StreamSource
{
public:
    // @brief Returns 0 if no error ocurred, otherwise a non-zero error code.
    // Once get_bytes returned an error, subsequent calls to get_status must return the same error.
    // If the encoder is in an error state, the behavior of get_available_bytes and get_bytes is undefined.
    virtual int get_status() = 0;

    // @brief Returns the minimum number of bytes that will still be generated by this encoder.
    // If 0, the encoder is considered complete and any subsequent call to get_bytes must generate
    // exactly 0 bytes.
    // get_bytes() must always generate as many bytes as requested unless the encoder generates no more bytes
    // afterwards
    virtual size_t get_available_bytes() = 0;
};

// @brief Base class for an encoder that is fed in a block-wise fashion.
// This base class is provided for convenience when implementing certain types of encoders.
// A StreamEncoder can be obtained from a BlockEncoder by using StreamEncoder_from_BlockEncoder.
template<unsigned BLOCKSIZE>
class BlockEncoder
{
public:
    typedef ::std::integral_constant<size_t, BLOCKSIZE> block_size;

    virtual int get_status() = 0;

    virtual size_t get_available_blocks() = 0;

    virtual int get_block(uint8_t block[BLOCKSIZE]) = 0;

private:
};

// @brief Base class for an encoder that is fed in a byte-wise fashion
// This base class is provided for convenience when implementing certain types of encoders.
// A StreamEncoder can be obtained from a ByteEncoder by using StreamEncoder_from_ByteEncoder.
class ByteEncoder
{
public:
    virtual int get_status() = 0;

    virtual size_t get_available_bytes() = 0;

    virtual int get_byte(uint8_t *output_byte) = 0;
};

/* Converter classes ---------------------------------------------------------*/

// @brief Encapsulates a BlockEncoder to make it look like a StreamEncoder
// @tparam T The encapsulated BlockEncoder type.
//           Must inherit from to BlockEncoder.
template<typename T, ENABLE_IF(TypeChecker<T>::template all_are<BlockEncoder<T::block_size::value>>()) >
class StreamEncoder_from_BlockEncoder : public StreamEncoder
{
public:
    // @brief Imitates the constructor signature of the encapsulated type.
    template<typename ... Args, ENABLE_IF(
            TypeChecker<Args...>::template first_is_not<StreamEncoder_from_BlockEncoder>()) >
    explicit StreamEncoder_from_BlockEncoder(Args &&... args)
            : block_encoder_(::std::forward<Args>(args)...)
    {
        EXPECT_TYPE(T, BlockEncoder<T::block_size::value>);
    }

    inline int get_status() final
    {
        return buffered_bytes_ ? 0 : block_encoder_.get_status();
    }

    inline size_t get_available_bytes() final
    {
        size_t available_bytes = block_encoder_.get_available_blocks() * T::block_size::value;
        return available_bytes + buffered_bytes_;
    }

    inline int get_bytes(uint8_t *buffer, size_t length, size_t *generated_bytes) final
    {
        while (!get_status() && get_available_bytes() && length)
        {
            // if the buffer is empty, retrieve a new block from the encode
            if (!buffered_bytes_)
            {
                block_encoder_.get_block(buffer_);
                buffered_bytes_ = T::block_size::value;
            }

            // hand the buffered bytes to the encoder
            size_t n_copy = ::std::min(buffered_bytes_, length);
            memcpy(buffer, buffer_ + T::block_size::value - n_copy, n_copy);
            length -= n_copy;
            buffer += n_copy;
            if (generated_bytes) (*generated_bytes) += n_copy;
            buffered_bytes_ -= n_copy;
        }
        return get_status();
    }

private:
    T block_encoder_;
    size_t buffered_bytes_ = 0;
    uint8_t buffer_[T::block_size::value];
};

// @brief Encapsulates a ByteEncoder to make it look like a BlockEncoder
// @tparam T The encapsulated ByteEncoder type.
//           Must inherit from ByteEncoder.
template<typename T, ENABLE_IF(TypeChecker<T>::template all_are<ByteEncoder>()) >
class BlockEncoder_from_ByteEncoder : public BlockEncoder<1>
{
public:
    // @brief Imitates the constructor signature of the encapsulated type.
    template<typename ... Args, ENABLE_IF(
            TypeChecker<Args...>::template first_is_not<BlockEncoder_from_ByteEncoder>()) >
    BlockEncoder_from_ByteEncoder(Args &&... args)
            : byte_encoder_(::std::forward<Args>(args)...)
    {
        EXPECT_TYPE(T, ByteEncoder);
    }

    inline int get_status() final
    {
        return byte_encoder_.get_status();
    }

    inline size_t get_available_blocks() final
    {
        return byte_encoder_.get_available_bytes();
    }

    inline int get_block(uint8_t block[1]) final
    {
        int status = byte_encoder_.get_byte(*block);
        return status;
    }

private:
    T byte_encoder_;
};

// @brief Encapsulates a ByteEncoder to make it look like a StreamEncoder
// @tparam T The encapsulated ByteEncoder type.
//           Must inherit from ByteEncoder.
template<typename T, ENABLE_IF(TypeChecker<T>::template all_are<ByteEncoder>()) >
class StreamEncoder_from_ByteEncoder : public StreamEncoder
{
public:
    // @brief Imitates the constructor signature of the encapsulated type.
    template<typename ... Args, ENABLE_IF(
            TypeChecker<Args...>::template first_is_not<StreamEncoder_from_ByteEncoder>()) >
    StreamEncoder_from_ByteEncoder(Args &&... args)
            : byte_encoder_(::std::forward<Args>(args)...)
    {
        EXPECT_TYPE(T, ByteEncoder);
    }

    inline int get_status() final
    {
        return byte_encoder_.get_status();
    }

    inline size_t get_available_bytes() final
    {
        return byte_encoder_.get_available_bytes();
    }

    inline int get_bytes(uint8_t *buffer, size_t length, size_t *generated_bytes) final
    {
        while (!byte_encoder_.get_status() && byte_encoder_.get_available_bytes() && length)
        {
            length--;
            if (generated_bytes) (*generated_bytes)++;
            byte_encoder_.get_byte(buffer++);
        }
        return byte_encoder_.get_status();
    }

private:
    T byte_encoder_;
};

/* Encoder implementations ---------------------------------------------------*/

template<typename T>
class VarintByteEncoder : public ByteEncoder
{
public:
    static constexpr T BIT_WIDTH = (CHAR_BIT * sizeof(T));

    VarintByteEncoder(const T &state_variable) :
            state_variable_(state_variable)
    {}

    size_t get_available_bytes() final
    {
        return done_ ? 0 : 1;
    }

    int get_status() final
    {
        return 0;
    }

    int get_byte(uint8_t *output_byte) final
    {
        if (bit_pos_ == 0)
            LOG_FIBRE("start encoding varint, from pos %d\n", bit_pos_);
        *output_byte = (state_variable_ >> bit_pos_) & 0x7f;
        bit_pos_ += 7;
        if (bit_pos_ < BIT_WIDTH && (state_variable_ >> bit_pos_))
        {
            LOG_FIBRE("remainder: %x\n", state_variable_ >> bit_pos_);
            *output_byte |= 0x80;
        } else
            done_ = true;
        return 0;
    }

private:
    const T &state_variable_;
    size_t bit_pos_ = 0; // bit position
    int status_ = 0;
    bool done_ = false;
};

template<typename T>
using VarintStreamEncoder = StreamEncoder_from_ByteEncoder<VarintByteEncoder<T>>;

template<typename T>
VarintStreamEncoder<T> make_varint_encoder(const T &variable)
{
    return VarintStreamEncoder<T>(variable);
}

VarintStreamEncoder<GET_TYPE_OF(&Request::endpoint_id)> make_endpoint_id_encoder(const Request &request)
{
    return make_varint_encoder(request.endpoint_id);
}

VarintStreamEncoder<GET_TYPE_OF(&Request::length)> make_length_encoder(const Request &request)
{
    return make_varint_encoder(request.length);
}

template<uint8_t INIT, uint8_t POLYNOMIAL, typename TEncoder,
        ENABLE_IF(TypeChecker<TEncoder>::template all_are<StreamEncoder>()) >
class CRC8BlockEncoder : public BlockEncoder<CRC8_BLOCKSIZE>
{
public:
    CRC8BlockEncoder(TEncoder &&inner_encoder)
            : inner_encoder_(::std::forward<TEncoder>(inner_encoder))
    {}

    int get_status() final
    {
        return status_;
    }

    size_t get_available_blocks() final
    {
        return (inner_encoder_.get_available_bytes() + CRC8_BLOCKSIZE - 2) / (CRC8_BLOCKSIZE - 1);
    }

    int get_block(uint8_t block[4]) final
    {
        size_t generated_bytes = 0;
        status_ = inner_encoder_.get_bytes(block, CRC8_BLOCKSIZE - 1, &generated_bytes);
        if (status_)
            return status_;

        // zero out unused end of the block
        while (generated_bytes < CRC8_BLOCKSIZE)
            block[generated_bytes++] = 0;

        block[CRC8_BLOCKSIZE - 1] = current_crc_ = calc_crc8<POLYNOMIAL>(current_crc_, block, CRC8_BLOCKSIZE - 1);
        return 0;
    }

private:
    TEncoder inner_encoder_;
    int status_ = 0;
    uint8_t current_crc_ = INIT;
};

template<unsigned INIT, unsigned POLYNOMIAL, typename TEncoder>
using CRC8StreamEncoder = StreamEncoder_from_BlockEncoder<CRC8BlockEncoder<INIT, POLYNOMIAL, TEncoder>>;

template<unsigned INIT, unsigned POLYNOMIAL, typename TEncoder>
CRC8StreamEncoder<INIT, POLYNOMIAL, TEncoder> make_crc8_encoder(TEncoder &&encoder)
{
    return CRC8StreamEncoder<INIT, POLYNOMIAL, TEncoder>(::std::forward<TEncoder>(encoder));
}

template<typename ... TEncoders>
class EncoderChain;

template<>
class EncoderChain<> : public StreamEncoder
{
public:
    size_t get_available_bytes() final
    { return 0; }

    int get_status() final
    { return 0; }

    int get_bytes(uint8_t *output, size_t length, size_t *generated_bytes) final
    { return 0; }
};

template<typename TEncoder, typename ... TEncoders>
class EncoderChain<TEncoder, TEncoders...> : public StreamEncoder
{
public:
    EncoderChain(TEncoder &&this_encoder, TEncoders &&... subsequent_encoders) :
            this_encoder_(::std::forward<TEncoder>(this_encoder)),
            subsequent_encoders_(::std::forward<TEncoders>(subsequent_encoders)...)
    {
        EXPECT_TYPE(TEncoder, StreamEncoder);
    }

    size_t get_available_bytes() final
    {
        return this_encoder_.get_available_bytes() + subsequent_encoders_.get_available_bytes();
    }

    int get_status() final
    {
        // If this encoder or any of the subsequent encoders failed, return error code.
        int this_status = this_encoder_.get_status();
        int subsequent_status = subsequent_encoders_.get_status();
        if (this_status)
            return this_status;
        else if (subsequent_status)
            return subsequent_status;
        else
            return 0;
    }

    int get_bytes(uint8_t *output, size_t length, size_t *generated_bytes) final
    {
        if (this_encoder_.get_available_bytes()){
            size_t chunk = 0;
            int status = this_encoder_.get_bytes(output, length, &chunk);
            if (status)
                return status;
            output += chunk;
            length -= chunk;
            if (generated_bytes) *generated_bytes += chunk;
            if (!length)
                return 0;
        }
        return subsequent_encoders_.get_bytes(output, length, generated_bytes);
    }

private:
    TEncoder this_encoder_;
    EncoderChain<TEncoders...> subsequent_encoders_;
};

template<typename ... TEncoders>
EncoderChain<TEncoders...> make_encoder_chain(TEncoders &&... encoders)
{
    return EncoderChain<TEncoders...>(::std::forward<TEncoders>(encoders)...);
}

}
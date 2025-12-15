#pragma once

#include <span>
#include <cstdint>


namespace ymd::mavlink{

template<typename T>
class Sink;

template<typename T>
class Source;

template<>
class Sink<uint8_t>{
public:
    size_t write(std::span<const uint8_t> pbuf);
    size_t pending() const;
};

using BytesSink = Sink<uint8_t>;

template<>
class Source<uint8_t>{
public:
    size_t read(std::span<uint8_t> pbuf);
    size_t available() const;
};

using BytesSource = Source<uint8_t>;


template<typename TProtocol>
class Sender;

template<typename TProtocol>
class Receiver;


}
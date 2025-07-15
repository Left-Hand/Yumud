#pragma once 

#include "st7789_prelude.hpp"


namespace ymd{

struct BitsQueue{
    constexpr explicit BitsQueue() = default;
    constexpr bool pop_bit(){
        const auto ret = buf_ & 0x01;
        length_ -= 1;
        buf_ = buf_ >> 1;
        return ret;
    }

    template<size_t N>
    constexpr uint32_t pop_bits(){
        constexpr uint32_t MASK = (1 << N) - 1;
        const auto ret = buf_ & MASK;
        length_ -= N;
        buf_ = buf_ >> N;
        return ret;
    }

    constexpr void push_bit(bool bit){
        buf_ = buf_ | (bit << (length_++));
    }

    template<size_t N>
    constexpr void push_bits(const uint32_t bits){
        buf_ = buf_ | (bits << (length_++));
    }

    constexpr uint32_t pop_remaining(){
        length_ = 0;
        return buf_;
    }

    constexpr uint32_t as_u64() const {
        return buf_;
    }

    constexpr size_t available() const {
        return length_;
    }

    constexpr size_t available_for_write() const {
        return 32 - length_;
    }
private:
    uint64_t buf_ = 0;
    size_t length_ = 0;

    static void static_test(){
        {
            constexpr auto queue = []{
                auto q = BitsQueue{};
                q.push_bits<5>(0b10111);
                q.pop_bits<2>();
                return q;
            }();
            static_assert(queue.as_u64() == 0b101);
        }
    }
};



template<typename T>
struct RepeatIter{
    constexpr explicit RepeatIter(const T value, size_t size):
        value_(value), size_(size){;}

    constexpr T next(){
        const auto ret = value_;
        index_ ++;
        return ret;
    }
    constexpr bool has_next() const {
        return index_ + 1 >= size_;
    }
private:
    T value_;
    const size_t size_;
    size_t index_ = 0;
};

template<typename T>
struct OnceIter{
    constexpr explicit OnceIter(const T value):
        value_(value){;}
    constexpr T next(){
        const auto ret = value_;
        is_done_ = true;
        return ret;
    }

    constexpr bool has_next() const{
        return is_done_ == false;
    }
private:
    T value_;
    bool is_done_ = false;
};


template<typename T>
struct BurstIter{
    constexpr explicit BurstIter(const std::span<const T> pbuf):
        pbuf_(pbuf){;}

    constexpr T next(){
        const auto ret = pbuf_[index_];
        index_ ++;
        return ret;
    }
    
    constexpr bool has_next() const {
        return index_ + 1 >= pbuf_.size();
    }
private:
    const std::span<const T> pbuf_;
    size_t index_ = 0;
};

}
namespace ymd::drivers{
class ST7789V3_Phy final:
    ST7789_Prelude{
public:
    template<typename T = void>
    using IResult = Result<void, drivers::DisplayerError>;
    

    template<typename T, typename Iter>
    struct U18BurstPixelDataIter{
        static constexpr bool DATA_BIT = 1;

        constexpr explicit U18BurstPixelDataIter(const Iter iter):
            iter_(iter) {}

        constexpr bool has_next() const {
            return is_runout_ == false;
        };

        constexpr uint16_t next() {
            if((queue_.available_for_write() > 18) and iter_.has_next()){
                const uint16_t next = iter_.next().as_u16();
                queue_.push_bit(DATA_BIT);
                queue_.push_bits<8>(next >> 8);
                queue_.push_bit(DATA_BIT);
                queue_.push_bits<8>(next & 0xff);
            }

            {
                const auto ava = queue_.available();
                if(ava >= 16){
                    return queue_.pop_bits<16>();
                }else{
                    return queue_.pop_remaining() << (16 - ava);
                }
            }
        };
    private:
        Iter iter_;
        BitsQueue queue_;
        bool is_runout_ = false;
    };

    explicit ST7789V3_Phy(
        Some<hal::SpiHw *> spi,
        const hal::SpiSlaveIndex index,
        Option<hal::Gpio &> res_gpio = None
    ):  
        spi_(spi.deref()), 
        idx_(index), 
        res_gpio_(res_gpio)
        {};

    [[nodiscard]] IResult<> init(){
        if(res_gpio_.is_some())
            res_gpio_.unwrap().outpp(HIGH);

        return reset();
    }

    [[nodiscard]] IResult<> reset(){
        if(res_gpio_.is_none()) return Ok();
        auto & res_gpio = res_gpio_.unwrap();
        clock::delay(10ms);
        res_gpio.clr();
        clock::delay(10ms);
        res_gpio.set();
        return Ok();
    }

    [[nodiscard]] IResult<> set_back_light_brightness(const real_t brightness){
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint8_t cmd){
        const auto temp = (uint16_t(cmd) << 7) | 0x8000;
        return write_by_iter<uint16_t>(OnceIter<uint16_t>(temp));
    }

    [[nodiscard]] IResult<> write_data8(const uint8_t data){
        const auto temp = uint16_t(data) << 7;
        return write_by_iter<uint16_t>(OnceIter<uint16_t>(temp));
    }

    [[nodiscard]] IResult<> write_data16(const uint16_t data){
        auto map_u16_to_be_u8 = [](const uint16_t u16) -> std::array<uint8_t, 2> { 
            return {uint8_t(u16 >> 8), uint8_t(u16 & 0xFF)};
        };


        const auto bytes = map_u16_to_be_u8(data);
        if(const auto res = write_data8(bytes[0]);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = write_data8(bytes[1]);
            res.is_err()) return Err(res.unwrap_err());
    }

    template<typename T>
    [[nodiscard]] IResult<> write_burst_pixels(std::span<const T> pbuf){
        return IResult<>(write_by_iter<uint16_t>(U18BurstPixelDataIter(BurstIter<T>(pbuf))));
    }


    template<typename T, typename Iter>
    [[nodiscard]] IResult<> write_by_iter(Iter iter){
        if (const auto res = spi_
            .begin(idx_.to_req()); 
            res.is_err()) 
            return Err(res.unwrap_err()); 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(magic::type_to_bits_v<T>); res.is_err())
                return Err(res.unwrap_err());
        }

        while(iter.has_next()){
            (void)spi_.fast_write(iter.next());
        }

        spi_.end();

        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
private:
    hal::SpiHw & spi_;
    hal::SpiSlaveIndex idx_;

    Option<hal::Gpio &>res_gpio_;

    template<hal::valid_spi_data T>
    [[nodiscard]] hal::HalResult phy_write_single(
        const is_stdlayout auto data, 
        Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));

        if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(sizeof(T) * 8); res.is_err())
                return res;
        }

        if constexpr (sizeof(T) == 1) {
            if(const auto res = spi_.write(uint8_t(data)); res.is_err()) return res;
        } else if constexpr (sizeof(T) == 2) {
            if(const auto res = spi_.write(uint16_t(data)); res.is_err()) return res;
        }

        if (cont == DISC) spi_.end();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return res;
        }

        return hal::HalResult::Ok();
    }

};

}
#pragma once 

#include "st7789_prelude.hpp"
#include "core/utils/bitsqueue.hpp"
#include "core/utils/data_iter.hpp"


namespace ymd::drivers{
struct ST7789V3_Phy final:
    ST7789_Prelude{
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
            if((queue_.writable_capacity() > 18) and iter_.has_next()){
                const uint16_t next = iter_.next();
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
        const hal::SpiSlaveRank index,
        Option<hal::Gpio &> res_gpio = None
    ):  
        spi_(spi.deref()), 
        rank_(index), 
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
            .borrow(rank_); 
            res.is_err()) 
            return Err(res.unwrap_err()); 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(magic::type_to_bits_v<T>); res.is_err())
                return Err(res.unwrap_err());
        }

        while(iter.has_next()){
            (void)spi_.fast_write(iter.next());
        }

        spi_.lend();

        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
private:
    hal::SpiHw & spi_;
    hal::SpiSlaveRank rank_;

    Option<hal::Gpio &>res_gpio_;

    template<hal::valid_spi_data T>
    [[nodiscard]] hal::HalResult phy_write_single(
        const is_stdlayout auto data, 
        Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));

        if(const auto res = spi_.borrow(rank_); res.is_err()) return res;
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(sizeof(T) * 8); res.is_err())
                return res;
        }

        if constexpr (sizeof(T) == 1) {
            if(const auto res = spi_.write(uint8_t(data)); res.is_err()) return res;
        } else if constexpr (sizeof(T) == 2) {
            if(const auto res = spi_.write(uint16_t(data)); res.is_err()) return res;
        }

        if (cont == DISC) spi_.lend();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return res;
        }

        return hal::HalResult::Ok();
    }

};

}
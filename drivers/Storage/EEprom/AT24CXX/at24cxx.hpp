#pragma once

#include "core/io/regs.hpp"
#include "core/clock/clock.hpp"
#include "core/utils/Option.hpp"

#include "primitive/memory.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct AT24CXX_Prelude{
public:
    enum class Error_Kind:uint8_t{
        DeviceIsBusyStore,
        DeviceIsBusyLoad,
        PayloadOverlength
    };

    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error,Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b10100000 >> 1); 


    struct Config{
        
        template<size_t C, size_t P>
        struct ConfigBase{
            static constexpr auto CAPACITY = AddressDiff(C);
            static constexpr auto PAGE_SIZE = AddressDiff(P);
        };

        struct AT24C01 final:public ConfigBase<1 << 7, 8>{};
        struct AT24C02 final:public ConfigBase<1 << 8, 8>{};
        struct AT24C04 final:public ConfigBase<1 << 9, 16>{};
        struct AT24C08 final:public ConfigBase<1 << 10, 16>{};
        struct AT24C16 final:public ConfigBase<1 << 11, 16>{};
        struct AT24C32 final:public ConfigBase<1 << 12, 32>{};
        struct AT24C64 final:public ConfigBase<1 << 13, 32>{};
        struct AT24C128 final:public ConfigBase<1 << 14, 64>{};
        struct AT24C256 final:public ConfigBase<1 << 15, 64>{};
        struct AT24C512 final:public ConfigBase<1 << 16, 128>{};
    };

};



class AT24CXX final:
    public AT24CXX_Prelude{
public:
    template<typename TConfig>
    explicit AT24CXX(TConfig && cfg, const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv), 
        capacity_(cfg.CAPACITY), 
        pagesize_(cfg.PAGE_SIZE){;}

    template<typename TConfig>
    explicit AT24CXX(TConfig && cfg, hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)), 
        capacity_(cfg.CAPACITY), 
        pagesize_(cfg.PAGE_SIZE){;}


    template<typename TConfig>
    explicit AT24CXX(TConfig && cfg, hal::I2c & i2c):
        AT24CXX(std::forward<TConfig>(cfg), 
        hal::I2cDrv{&i2c, DEFAULT_I2C_ADDR}){;}

    AT24CXX(const AT24CXX &) = delete;
    AT24CXX(AT24CXX &&) = delete;


    IResult<> init();
    IResult<> validate();

    AddressDiff capacity(){return capacity_;}

    bool is_idle(){
        return state_.is_idle(clock::millis());
    }

    auto poll(){
        return state_.poll(*this, clock::millis());
    }

    IResult<> store_bytes(const Address begin, const std::span<const uint8_t> pbuf){
        const auto now = clock::millis();
        if(const auto res = state_.add_store_task(begin, pbuf, pagesize_, now);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = state_.initial(*this, now);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    IResult<> load_bytes(const Address begin, const std::span<uint8_t> pbuf){
        const auto now = clock::millis();
        if(const auto res = state_.add_load_task(begin, pbuf, pagesize_, now);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = state_.initial(*this, now);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:


    class Timer final{
    public:
        Timer() = default;
        constexpr void reset(const Milliseconds now){
            begin_ = now;
        }

        constexpr bool is_expired(const Milliseconds now, const Milliseconds period) const {
            return now - begin_ > period;
        }

        constexpr Milliseconds duration(const Milliseconds now) const {
            return now - begin_;
        }
    private:
        Milliseconds begin_ = 0ms;
    };

    
    struct LoadTask final{
    public:
        constexpr LoadTask(
            const uint32_t begin, 
            const std::span<uint8_t> pbuf,
            const uint32_t gsize
        ):
            iter_(RangeGridIter{{begin, begin + pbuf.size()}, gsize}),
            pbuf_(pbuf),
            operating_(iter_.begin())
        {;}

        IResult<> initial(AT24CXX & self, const Milliseconds now){
            return poll(self, now);
        }

        IResult<> poll(AT24CXX & self, const Milliseconds now){
            if(not is_oper_complete(now)) return Ok();
            if(no_next_block_) return Ok();

            last_time_ = now;
            const auto addr = operating_.start;
            const auto piece = iter_.subspan(pbuf_, operating_);
            if(const auto res = self.load_bytes_inblock_impl(Address(addr), piece);
                res.is_err()) return Err(res.unwrap_err());

            const auto next_opt = iter_.next(operating_);
            if(next_opt.is_some()){
                operating_ = next_opt.unwrap();
            }else{
                no_next_block_ = true;
            }

            return Ok();
        }

        constexpr bool is_oper_complete(const Milliseconds now) const{
            return now - last_time_ >= 6ms;
        }
        constexpr bool is_task_complete(const Milliseconds now) const {
            return no_next_block_ and is_oper_complete(now);
        }
    private:
        const RangeGridIter iter_;
        const std::span<uint8_t> pbuf_;
        Milliseconds last_time_ = 0ms;
        Range2u operating_;
        bool no_next_block_ = false;
    };

    friend class LoadTask;

    struct StoreTask final{
    public:
        constexpr StoreTask(
            const uint32_t begin, 
            const std::span<const uint8_t> pbuf,
            const uint32_t gsize
        ):
            iter_(RangeGridIter{{begin, begin + pbuf.size()}, gsize}),
            pbuf_(pbuf),
            operating_(iter_.begin())
        {;}

        IResult<> initial(AT24CXX & self, const Milliseconds now){
            return poll(self, now);
        }

        IResult<> poll(AT24CXX & self, const Milliseconds now){
            if(not is_oper_complete(now)) return Ok();
            if(no_next_block_) return Ok();
            last_time_ = now;
            const auto addr = operating_.start;
            const auto piece = iter_.subspan(pbuf_, operating_);
            if(const auto res = self.store_bytes_inblock_impl(Address(addr), piece);
                res.is_err()) return Err(res.unwrap_err());

            const auto next_opt = iter_.next(operating_);
            if(next_opt.is_some()){
                operating_ = next_opt.unwrap();
            }else{
                no_next_block_ = true;
            }

            return Ok();
        }

        constexpr bool is_oper_complete(const Milliseconds now) const{
            return now - last_time_ >= 6ms;
        }
        constexpr bool is_task_complete(const Milliseconds now) const {
            return no_next_block_ and is_oper_complete(now);
        }
    private:
        const RangeGridIter iter_;
        const std::span<const uint8_t> pbuf_;
        Milliseconds last_time_ = 0ms;
        Range2u operating_;
        bool no_next_block_ = false;
    };

    friend class StoreTask;


    hal::I2cDrv i2c_drv_;
    AddressDiff capacity_;
    AddressDiff pagesize_;

    struct State final{
        IResult<> poll(AT24CXX & self, const Milliseconds now){
            if(is_idle(now) == true){
                may_tasks_ = std::nullopt;
                return Ok();
            }
            auto & tasks = may_tasks_.value();
            const auto res = std::visit([&](auto & task){
                return task.poll(self, now);
            }, tasks);
            return res;
        }

        bool is_idle(const Milliseconds now) const {
            if(not may_tasks_.has_value())
                return true;
            const auto & tasks = may_tasks_.value();
            return std::visit([&](const auto & task){
                return task.is_task_complete(now);
            }, tasks);
        }

        IResult<> add_store_task(
            const Address begin, 
            const std::span<const uint8_t> pbuf,
            const AddressDiff gsize,
            const Milliseconds now
        ){
            if(not is_idle(now)) 
                return Err(map_currtask_to_err(may_tasks_.value()));
            may_tasks_.emplace(Tasks(StoreTask(begin.as_u32(), pbuf, gsize.as_u32())));
            return Ok();
        }


        IResult<> add_load_task(
            const Address begin, 
            const std::span<uint8_t> pbuf,
            const AddressDiff gsize,
            const Milliseconds now
        ){
            if(not is_idle(now)) 
                return Err(map_currtask_to_err(may_tasks_.value()));
            may_tasks_.emplace(Tasks(LoadTask(begin.as_u32(), pbuf, gsize.as_u32())));
            return Ok();
        }

        IResult<> initial(AT24CXX & self, const Milliseconds now){
            return initial_task(self, may_tasks_.value(), now);
        }
    private:

        using Tasks = std::variant<
            LoadTask, 
            StoreTask
        >;
        std::optional<Tasks> may_tasks_;

        static constexpr Error map_currtask_to_err(const Tasks & tasks){
            if(std::holds_alternative<LoadTask>(tasks))
                return Error::DeviceIsBusyLoad;
            else if(std::holds_alternative<StoreTask>(tasks)) 
                return Error::DeviceIsBusyStore;
            __builtin_unreachable();
        }

        // IResult<bool> is_busy(AT24CXX & self, const Milliseconds now){        
        //     if(may_tasks_.has_value() == false) return Ok(false);
        //     const auto oper = map_task_to_oper(may_tasks_.value());
        //     const auto lasting = map_operation_to_lasting(oper);

        //     return Ok(timer_.is_expired(now, lasting));
        // }

        IResult<> initial_task(AT24CXX & self, Tasks & tasks, const Milliseconds now){
            return std::visit([&](auto & task){
                return task.initial(self, now);
            }, tasks);
        }
    };

    State state_;

    constexpr AddressDiff pagesize() const {
        return pagesize_;
    }

    constexpr bool is_small_chip(){
        return capacity_ <= AddressDiff(256);
    }
    IResult<> write_burst(const Address loc, const std::span<const uint8_t> pbuf);
    IResult<> read_burst(const Address loc, const std::span<uint8_t> pbuf);

    IResult<> store_bytes_inblock_impl(const Address loc, const std::span<const uint8_t> pbuf);
    IResult<> load_bytes_inblock_impl(const Address loc, const std::span<uint8_t> pbuf);
};


}
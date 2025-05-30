#pragma once

#include "core/io/regs.hpp"
#include "core/clock/clock.hpp"
#include "core/utils/Option.hpp"

#include "concept/memory.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{





class AT24CXX final:public StorageIntf{
public:
    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u8(0b10100000); 

    #define DEF_AT24CXX_PRESET_TEMPLATE(name, c, p)\
    struct name final{\
        static constexpr auto CAPACITY = AddressDiff(c);\
        static constexpr auto PAGE_SIZE = AddressDiff(p);\
    };\

    DEF_AT24CXX_PRESET_TEMPLATE(AT24C01, 1 << 7, 8)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C02, 1 << 8, 8)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C04, 1 << 9, 16)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C08, 1 << 10, 16)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C16, 1 << 11, 16)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C32, 1 << 12, 32)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C64, 1 << 13, 32)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C128, 1 << 14, 64)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C256, 1 << 15, 64)
    DEF_AT24CXX_PRESET_TEMPLATE(AT24C512, 1 << 16, 128)

    #undef DEF_AT24CXX_PRESET_TEMPLATE

    // AT24CXX(const hal::I2cDrv & i2c_drv, const auto & cfg):
    //     AT24CXX(i2c_drv, cfg.CAPACITY, cfg.PAGE_SIZE) {;}

    AT24CXX(const auto & cfg, hal::I2c & i2c, const hal::I2cSlaveAddr<7> & addr = DEFAULT_I2C_ADDR):
        AT24CXX(hal::I2cDrv{i2c, addr}, cfg.CAPACITY, cfg.PAGE_SIZE) {;}

    void init(){};

    AddressDiff capacity(){return capacity_;}

    bool is_busy(){return true;}
    bool is_available(){return false;}

    IResult<size_t> resume(){return Ok(0u);}
private:
    AT24CXX(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr, const AddressDiff capacity, const AddressDiff pagesize):
        AT24CXX(hal::I2cDrv{i2c, addr}, capacity, pagesize){;}
    AT24CXX(const hal::I2cDrv & i2c_drv, const AddressDiff capacity, const AddressDiff pagesize):
        i2c_drv_(i2c_drv), capacity_(capacity), pagesize_(pagesize){;}
        

private:


    class Timer final{
    public:
        Timer() = default;
        constexpr void reset(const Milliseconds now, const Milliseconds sustain){
            begin_ = now;
            sustain_ = sustain;
        }

        constexpr bool is_expired(const Milliseconds now) const {
            return now - begin_ > sustain_;
        }

        Milliseconds has_passed(const Milliseconds now) const {
            return now - begin_;
        }
    private:
        Milliseconds begin_ = 0ms;
        Milliseconds sustain_ = 0ms;
    };
    // static constexpr auto transform_operation_to_ms(const Operation operation){
    //     switch(operation){
    //         case Operation::Load: return 6ms;
    //         case Operation::Store: return 6ms;
    //         case Operation::Erase: return 6ms;
    //         default: __builtin_unreachable();
    //     }
    // }
    
    struct TaskBase;

    struct State{
    public:
        constexpr State() = default;

        constexpr const auto manipulator() const{
            return manipulator_;
        }

        constexpr bool is_available() const{return manipulator_ == nullptr;}

        void inject_manipulator(TaskBase * task){
            manipulator_ = task; 
        }

    private:
        TaskBase * manipulator_ = nullptr;
        friend class TaskBase;
    };

    struct TaskBase{
        TaskBase(AT24CXX & storage):
            storage_(storage){;}


        IResult<size_t> resume(){
            // auto & self = *this;
            auto & state = storage_.state_;

            const bool is_available = (state.is_available()); 

            // const bool is_under_occupy = (state.manipulator() == this);

            const bool cant_occupy = (state.manipulator() != nullptr 
                and state.manipulator() != this);

            if(cant_occupy) return Err(Error(Error::IsOperatingByOther));

            if(is_available){
                state.inject_manipulator(this);
            }
            return execute();
        }
    protected:
        virtual IResult<size_t> execute() = 0;
        AT24CXX & storage_;
        Timer timer_;
    };

    struct StoreTask final:public TaskBase{
        StoreTask(AT24CXX & device, const Address addr, const std::span<const uint8_t> pdata):
            TaskBase(device),
            pdata_(pdata)
        {
            mem_range_ = {addr.as_u32(), pdata.size()};
            op_range_ = init_grid(addr.as_u32(), grid_size());
        }

        IResult<size_t> execute(){
            //设备正忙于处理上次的操作
            if(!timer_.is_expired(clock::millis())) return Ok(pending());

            //所有操作均结束
            if(op_range_.length() == 0){
                //释放所有权
                storage_.state_.inject_manipulator(nullptr);
                return Ok(0);
            }

            const auto offset = op_range_.from - mem_range_.from;
            storage_.write_burst(Address(op_range_.from), std::span<const uint8_t>(
                pdata_.data() + offset, op_range_.length()));
            //准备下次的范围
            op_range_ = next_grid(op_range_.from, grid_size(), mem_range_);
        }

        size_t pending() const {
            return mem_range_.end() - op_range_.end();
        }

        static constexpr Range2<uint32_t> init_grid(const uint32_t addr, const uint32_t grid){
            const auto rem = addr % grid;
            if(rem == 0){
                return {addr, addr + grid};
            }else{
                return {addr, addr + grid - rem}; 
            }
        }

        static constexpr Range2<uint32_t> next_grid(
            const uint32_t addr, const uint32_t grid, const Range2<uint32_t> range
        ){
            const auto rem = addr % grid;
            const auto next_addr = addr - rem + grid;
            const auto next_end = range.clamp(next_addr + grid);
            if(next_addr >= next_end) return Range2<uint32_t>{next_addr, next_addr};
            else return Range2<uint32_t>{next_addr, next_end};
        }

    private:
        std::span<const uint8_t> pdata_;
        Range2<uint32_t> mem_range_;
        Range2<uint32_t> op_range_;

        const uint32_t grid_size()const{
            return storage_.pagesize().as_u32();
        }
    };


    hal::I2cDrv i2c_drv_;
    State state_;
    AddressDiff capacity_;
    AddressDiff pagesize_;

    constexpr AddressDiff pagesize() const {
        return pagesize_;
    }
    constexpr bool is_small_chip(){
        return capacity_ <= AddressDiff(256);
    }

    // constexpr bool is_busy() const {
    //     constexpr auto STORE_LASTING = 6ms;
    //     constexpr auto LOAD_LASTING = 6ms;
    //     constexpr auto ERASE_LASTING = 6ms;

    //     const auto oper = state_.operation();
    //     if(oper.is_some()){
    //         switch(oper.unwrap()){
    //             case Operation::Store:  return state_.has_passed() < STORE_LASTING;
    //             case Operation::Load:   return state_.has_passed() < LOAD_LASTING;
    //             case Operation::Erase:  return state_.has_passed() < ERASE_LASTING;
    //             default: __builtin_unreachable();
    //         }
    //     }else{
    //         return false;
    //     }
    // }

    hal::HalResult write_burst(const Address loc, const std::span<const uint8_t> pdata);
    hal::HalResult read_burst(const Address loc, const std::span<uint8_t> pdata);


    void store_bytes_impl(const Address loc, const std::span<const uint8_t> pdata);

    void load_bytes_impl(const Address loc, const std::span<uint8_t> pdata);

    void blocking_until_free(){
        while(is_busy()){
            clock::delay(1ms);
        }
    }
};


}
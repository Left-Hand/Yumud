//TODO REFACTOR THIS SHIT

// #define I2CDRV_DEBUG

#ifdef I2CDRV_DEBUG
#undef I2CDRV_DEBUG
#define I2CDRV_DEBUG(...) DEBUG_PRINTLN("I2CDRV DEBUG:", __VA_ARGS__)
#else
#define I2CDRV_DEBUG(...)
#endif


#define __I2CDRV_LENGTH_GUARD\
    if(unlikely(len == 0)) {\
        return BusError::ZERO_LENGTH;\
    }\

#define __I2CDRV_ITER_WRITE_BYTES(pdata, endian)\
return iterate_bytes(\
    pdata, endian, \
    [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },\
    [](const BusError err) -> bool {return err.wrong();},\
    []() -> BusError {return BusError::OK;}\
);\


#define __I2CDRV_ITER_WRITE_SAME_BYTES(data, len, endian)\
return iterate_bytes(\
    data, len, endian, \
    [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },\
    [](const BusError err) -> bool {return err.wrong();},\
    []() -> BusError {return BusError::OK;}\
);\

#define __I2CDRV_ITER_READ_BYTES(pdata, endian)\
return iterate_bytes(\
    pdata, endian, \
    [&](std::byte & byte, const bool is_end) -> BusError{\
        uint32_t dummy = 0; auto err = bus_.read(dummy, is_end ? NACK : ACK); byte = std::byte(dummy); return err;},\
    [](const BusError err) -> bool {return err.wrong();},\
    []() -> BusError {return BusError::OK;}\
);\

namespace ymd::hal{


template<typename T>
requires valid_i2c_data<T>
BusError I2cDrv::writeBody(
    const T data,
    const Endian endian
){
    constexpr size_t size = sizeof(std::decay_t<decltype(data)>);

    switch(size){
        case 1:
            return (bus_.write(uint8_t(data)));
        case 2:
            if(endian == MSB){
                if(auto err = (bus_.write(uint8_t(data >> 8)));    err.wrong()) return err;
                if(auto err = (bus_.write(uint8_t(data)));         err.wrong()) return err;
            }else{
                if(auto err = (bus_.write(uint8_t(data)));         err.wrong()) return err;
                if(auto err = (bus_.write(uint8_t(data >> 8)));    err.wrong()) return err;
            }
        default:
            return BusError::UNSPECIFIED;
    }
}

BusError I2cDrv::writeRegAddress(
    const valid_i2c_regaddr auto addr,
    const Endian endian
){
    auto err = writeBody(addr, endian);

    if(err.wrong()){
        bus_.end();
    }

    return err;
}


BusError I2cDrv::writeCommand_impl(
    const valid_i2c_regaddr auto cmd,
    const Endian endian
){
    const auto guard = createGuard();

    return writeBody(cmd, endian);
}


BusError I2cDrv::writeMulti_impl(
    const valid_i2c_regaddr auto addr, 
    const valid_i2c_data auto * pdata, 
    const size_t len, 
    const Endian endian
){
    __I2CDRV_LENGTH_GUARD;
    
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }

        __I2CDRV_ITER_WRITE_BYTES((std::span(pdata, len)), endian);
    }else{
        return begin_err;
    }
}

BusError I2cDrv::readMulti_impl(
    const valid_i2c_regaddr auto addr, 
    valid_i2c_data auto * pdata, 
    const size_t len, 
    const Endian endian
){
    __I2CDRV_LENGTH_GUARD;

    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }
        if(const auto reset_err = bus_.begin(index_ | 0x01); reset_err.ok()){
            __I2CDRV_ITER_READ_BYTES((std::span(pdata, len)), endian);
        }else{
            return reset_err;
        }
    }else{
        return begin_err;
    }
}

BusError I2cDrv::writeSame_impl(
    const valid_i2c_regaddr auto addr,
    const valid_i2c_data auto data,
    const size_t len,
    const Endian endian
){
    __I2CDRV_LENGTH_GUARD;
    
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }
        __I2CDRV_ITER_WRITE_SAME_BYTES(data, len, endian);
    }else{
        return begin_err;
    }
}



BusError I2cDrv::writeBurst_impl(
    const valid_i2c_regaddr auto addr,
    std::span<const valid_i2c_data auto> pdata,
    const Endian endian
){
    
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();

        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }

        __I2CDRV_ITER_WRITE_BYTES((pdata), endian);
    }else{
        return begin_err;
    }
}

BusError I2cDrv::readBurst_impl(
    const valid_i2c_regaddr auto addr,
    std::span<valid_i2c_data auto> pdata,
    const Endian endian
){
    
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();

        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }

        __I2CDRV_ITER_READ_BYTES((pdata), endian);
    }else{
        return begin_err;
    }
}

}


#undef __I2CDRV_LENGTH_GUARD
#undef __I2CDRV_ITER_WRITE_BYTES
#undef __I2CDRV_ITER_READ_BYTES
#undef __I2CDRV_ITER_WRITE_SAME_BYTES
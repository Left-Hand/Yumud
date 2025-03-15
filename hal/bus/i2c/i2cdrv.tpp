//TODO REFACTOR THIS SHIT

// #define I2CDRV_DEBUG

#ifdef I2CDRV_DEBUG
#undef I2CDRV_DEBUG
#define I2CDRV_DEBUG(...) DEBUG_PRINTLN("I2CDRV DEBUG:", __VA_ARGS__)
#else
#define I2CDRV_DEBUG(...)
#endif



#define __I2CDRV_ITER_WRITE_SAME_BYTES(data, len, endian)\
return iterate_bytes(\
    data, len, endian, \
    [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },\
    [](const BusError err) -> bool {return err.wrong();},\
    []() -> BusError {return BusError::OK;}\
);\


namespace ymd::hal{


BusError I2cDrv::writePayload(
    std::span<const valid_i2c_data auto> pdata,
    const Endian endian
){
    // constexpr size_t size = sizeof(std::decay_t<decltype(data)>);

    // if constexpr (size == 1){
    //     return (bus_.write(uint8_t(data)));
    // }else if constexpr(size == 2){
    //     if(endian == MSB){
    //         if(auto err = (bus_.write(uint8_t(data >> 8)));    err.wrong()) return err;
    //         if(auto err = (bus_.write(uint8_t(data)));         err.wrong()) return err;
    //     }else{
    //         if(auto err = (bus_.write(uint8_t(data)));         err.wrong()) return err;
    //         if(auto err = (bus_.write(uint8_t(data >> 8)));    err.wrong()) return err;
    //     }
    //     return BusError::OK;
    // }else{
    //     return BusError::UNSPECIFIED;
    // }

    return iterate_bytes(
        pdata, endian, 
        [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },
        [](const BusError err) -> bool {return err.wrong();},
        []() -> BusError {return BusError::OK;}
    );
}

BusError I2cDrv::readPayload(
    std::span<valid_i2c_data auto> pdata,
    const Endian endian
){
    // constexpr size_t size = sizeof(std::decay_t<decltype(data)>);

    // if constexpr (size == 1){
    //     uint32_t dummy;
    //     dummy = uint8_t(data);
    //     return (bus_.read(dummy, NACK));
    // }else if constexpr(size == 2){
    //     if(endian == MSB){
    //         uint32_t dummy;
    //         dummy = uint8_t(data >> 8);
    //         if(auto err = (bus_.read(dummy, ACK));    err.wrong()) return err;
    //         dummy = uint8_t(data);
    //         if(auto err = (bus_.read(dummy, NACK));         err.wrong()) return err;
    //     }else{
    //         uint32_t dummy;
    //         dummy = uint8_t(data);
    //         if(auto err = (bus_.read(dummy, ACK));    err.wrong()) return err;
    //         dummy = uint8_t(data >> 8);
    //         if(auto err = (bus_.read(dummy, NACK));         err.wrong()) return err;
    //     }
    //     return BusError::OK;
    // }else{
    //     return BusError::UNSPECIFIED;
    // }

    return iterate_bytes(
        pdata, endian, 
        [&](std::byte & byte, const bool is_end) -> BusError{
            uint32_t dummy = 0; auto err = bus_.read(dummy, is_end ? NACK : ACK); byte = std::byte(dummy); return err;},
        [](const BusError err) -> bool {return err.wrong();},
        []() -> BusError {return BusError::OK;}
    );
    
}


BusError I2cDrv::writeRegAddress(
    const valid_i2c_regaddr auto addr,
    const Endian endian
){
    auto err = writePayload(std::span(&addr,1), endian);

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

    return writePayload(std::span(&cmd,1), endian);
}


BusError I2cDrv::writeBurst_impl(
    const valid_i2c_regaddr auto addr, 
    std::span<const valid_i2c_data auto> pdata, 
    const Endian endian
){
    const auto guard = createGuard();

    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }

        return writePayload(pdata, endian);
    }else{
        return begin_err;
    }
}

BusError I2cDrv::readBurst_impl(
    const valid_i2c_regaddr auto addr, 
    std::span<valid_i2c_data auto> pdata, 
    const Endian endian
){
    const auto guard = createGuard();
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }
        if(const auto reset_err = bus_.begin(index_ | 0x01); reset_err.ok()){
            return readPayload(pdata, endian);
        }else{
            return reset_err;
        }
    }else{
        return begin_err;
    }
}

BusError I2cDrv::writeRepeat_impl(
    const valid_i2c_regaddr auto addr,
    const valid_i2c_data auto data,
    const size_t len,
    const Endian endian
){    
    const auto guard = createGuard();
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }
        __I2CDRV_ITER_WRITE_SAME_BYTES(data, len, endian);
    }else{
        return begin_err;
    }
}

}

#undef __I2CDRV_ITER_WRITE_SAME_BYTES
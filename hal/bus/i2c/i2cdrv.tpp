//TODO REFACTOR THIS SHIT

// #define I2CDRV_DEBUG

#ifdef I2CDRV_DEBUG
#undef I2CDRV_DEBUG
#define I2CDRV_DEBUG(...) DEBUG_PRINTLN("I2CDRV DEBUG:", __VA_ARGS__)
#else
#define I2CDRV_DEBUG(...)
#endif


#define __I2CDRV_LENGTH_GUARD\
    if constexpr(size == 0){\
        return BusError::ZERO_LENGTH;\
    }\
    if(len == 0) {\
        return BusError::ZERO_LENGTH;\
    }\

namespace ymd::hal{

BusError I2cDrv::writeRegAddress(
    const valid_i2c_regaddr auto addr,
    const Endian endian
){
    constexpr size_t size = sizeof(std::decay_t<decltype(addr)>);

    BusError err = BusError::OK;

    switch(size){
        case 1:
            err.emplace(bus_.write(uint8_t(addr)));
            break;
        case 2:
                err.emplace(bus_.write(uint8_t(addr >> 8)));
                err.emplace(bus_.write(uint8_t(addr)));
            break;
        default:
            break;
    }

    if(err.wrong()){
        bus_.end();
    }

    return err;
}

BusError I2cDrv::writeCommand_impl(
    const valid_i2c_regaddr auto cmd,
    const Endian endian
){
    constexpr size_t size = sizeof(std::decay_t<decltype(cmd)>);

    BusError err = BusError::OK;

    const auto guard = createGuard();

    switch(size){
        case 1:
            err.emplace(bus_.write(uint8_t(cmd)));
            break;
        case 2:
            err.emplace(bus_.write(uint8_t(cmd >> 8)));
            err.emplace(bus_.write(uint8_t(cmd)));
            break;
        default:
            break;
    }

    return err;
}

BusError I2cDrv::writeMulti_impl(
    const valid_i2c_regaddr auto addr, 
    const valid_i2c_data auto * pdata, 
    const size_t len, 
    const Endian endian
){
    constexpr size_t size = sizeof(std::decay_t<decltype(*pdata)>);

    __I2CDRV_LENGTH_GUARD;

    const size_t bytes = len * size;
    const uint8_t *u8_ptr = reinterpret_cast<const uint8_t *>(pdata);

    
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }

        for(size_t i = 0; i < bytes; i += size){
            if(endian == MSB){
                for(size_t j = size; j > 0; j--){
                    const auto err = bus_.write(u8_ptr[j-1 + i]);
                    if(err.wrong()) return err;
                }
            }else{
                for(size_t j = 0; j < size; j++){
                    const auto err = bus_.write(u8_ptr[j + i]);
                    if(err.wrong()) return err;
                }
            }
        }
        return BusError::OK;
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
    constexpr size_t size = sizeof(std::decay_t<decltype(data)>);

    __I2CDRV_LENGTH_GUARD;

    const size_t bytes = len * size;
    const uint8_t *u8_ptr = reinterpret_cast<const uint8_t *>(&data);

    
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }
        for(size_t i = 0; i < bytes; i += size){
            if(endian == MSB){
                for(size_t j = size; j > 0; j--){
                    const auto err = bus_.write(u8_ptr[j-1]);
                    if(err.wrong()) return err;
                }
            }else{
                for(size_t j = 0; j < size; j++){
                    const auto err = bus_.write(u8_ptr[j]);
                    if(err.wrong()) return err;
                }
            }
        }


        return BusError::OK;
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
    constexpr size_t size = sizeof(std::decay_t<decltype(*pdata)>);

    __I2CDRV_LENGTH_GUARD;

    const size_t bytes = len * size;
    uint8_t * u8_ptr = reinterpret_cast<uint8_t *>(pdata);

    
    if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
        const auto guard = createGuard();
        if(const auto err = writeRegAddress(addr, endian); err.wrong()){
            return err;
        }
        if(bus_.begin(index_ | 0x01).ok()){
            for(size_t i = 0; i < bytes; i += size){
                if(endian == MSB){
                    for(size_t j = size; j > 0; j--){
                        uint32_t temp = 0;
                        const auto err = bus_.read(temp, Ack::from(!((j == 1) && (i == bytes - size))));
                        if(err.wrong()) return err;
                        u8_ptr[j-1 + i] = temp;
                    }
                }else{
                    for(size_t j = 0; j < size; j++){
                        uint32_t temp = 0;
                        const auto err = bus_.read(temp, Ack::from(i + j != bytes - 1));
                        if(err.wrong()) return err;
                        u8_ptr[j + i] = temp;
                    }
                }
            }
        }
        return BusError::OK;
    }else{
        return begin_err;
    }

}

}


#undef __I2CDRV_LENGTH_GUARD
#undef I2CDRV_DEBUG
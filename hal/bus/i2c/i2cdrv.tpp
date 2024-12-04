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
        return Bus::ErrorType::ZERO_LENGTH;\
    }\
    if(len == 0) {\
        return Bus::ErrorType::ZERO_LENGTH;\
    }\

namespace ymd{

template<typename T>
requires valid_i2c_regaddr<T>
Bus::Error I2cDrv::writeRegAddress(const T addr, const Endian endian){
    constexpr size_t size = sizeof(T);

    Bus::Error err = Bus::ErrorType::OK;

    switch(size){
        case 1:
            err = bus_.write(uint8_t(addr));
            break;
        case 2:
            // if(endian == MSB){
                err = bus_.write(uint8_t(addr >> 8));
                err = bus_.write(uint8_t(addr));
            // }else{
            //     err = bus_.write(uint8_t(addr));
            //     err = bus_.write(uint8_t(addr >> 8));

            // }
            break;
        default:
            break;
    }

    if(err){
        I2CDRV_DEBUG(err);
    }

    return err;
}

template<typename T>
requires valid_i2c_data<T>
Bus::Error I2cDrv::writeMulti_impl(const valid_i2c_regaddr auto addr, const T * pdata, const size_t len, const Endian endian){
    constexpr size_t size = sizeof(T);

    __I2CDRV_LENGTH_GUARD;

    const size_t bytes = len * size;
    const uint8_t *u8_ptr = reinterpret_cast<const uint8_t *>(pdata);

    auto err = bus_.begin(index_);
    if(err == Bus::ErrorType::OK){
        writeRegAddress(addr, endian);
        for(size_t i = 0; i < bytes; i += size){
            if(endian == MSB){
                for(size_t j = size; j > 0; j--){
                    err = bus_.write(u8_ptr[j-1 + i]);
                    if(err) goto handle_error;
                }
            }else{
                for(size_t j = 0; j < size; j++){
                    err = bus_.write(u8_ptr[j + i]);
                    if(err) goto handle_error;
                }
            }
        }

        bus_.end();
    }else{
    handle_error:
        I2CDRV_DEBUG(err);
    }

    return err;
}


template<typename T>
requires valid_i2c_data<T>
Bus::Error I2cDrv::writeSame_impl(const valid_i2c_regaddr auto addr, const T data, const size_t len, const Endian endian){
    constexpr size_t size = sizeof(T);

    __I2CDRV_LENGTH_GUARD;

    const size_t bytes = len * size;
    const uint8_t *u8_ptr = reinterpret_cast<const uint8_t *>(&data);

    auto err = bus_.begin(index_); 
    if(err == Bus::ErrorType::OK){
        writeRegAddress(addr, endian);
        for(size_t i = 0; i < bytes; i += size){
            if(endian == MSB){
                for(size_t j = size; j > 0; j--){
                    err = bus_.write(u8_ptr[j-1]);
                    if(err) goto handle_error;
                }
            }else{
                for(size_t j = 0; j < size; j++){
                    err = bus_.write(u8_ptr[j]);
                    if(err) goto handle_error;
                }
            }
        }

        bus_.end();
    }else{
    handle_error:
        I2CDRV_DEBUG(err);
    }

    return err;
}

template<typename T>
requires valid_i2c_data<T>
Bus::Error I2cDrv::readMulti_impl(const valid_i2c_regaddr auto addr, T * pdata, const size_t len, const Endian endian){
    constexpr size_t size = sizeof(T);

    __I2CDRV_LENGTH_GUARD;

    const size_t bytes = len * size;
    uint8_t * u8_ptr = reinterpret_cast<uint8_t *>(pdata);

    auto err = bus_.begin(index_);
    if(err == Bus::ErrorType::OK){
        this->writeRegAddress(addr, endian);
        if(bus_.begin(index_ | 0x01) == Bus::ErrorType::OK){
            for(size_t i = 0; i < bytes; i += size){
                if(endian == MSB){
                    for(size_t j = size; j > 0; j--){
                        uint32_t temp = 0;
                        err = bus_.read(temp, !((j == 1) && (i == bytes - size)));
                        if(err) goto handle_error;
                        u8_ptr[j-1 + i] = temp;
                    }
                }else{
                    for(size_t j = 0; j < size; j++){
                        uint32_t temp = 0;
                        err = bus_.read(temp, (i + j != bytes - 1));
                        if(err) goto handle_error;
                        u8_ptr[j + i] = temp;
                    }
                }
            }
        }
        bus_.end();
    }else{
    handle_error:
        I2CDRV_DEBUG(err);
    }

    return err;
}

bool I2cDrv::verify(){
    if(auto err = bus_.begin(index_ | 0x00); err) return false;
    bus_.end();
    if(auto err = bus_.begin(index_ | 0x01); err) return false;
    bus_.end();
    return true;
}

void I2cDrv::release(){
    bus_.begin(index_);
    bus_.end();
}
}
#undef __I2CDRV_LENGTH_GUARD
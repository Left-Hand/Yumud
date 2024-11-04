//TODO REFACTOR THIS SHIT
// #define I2CDRV_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__)
#define I2CDRV_DEBUG(...)


namespace yumud{

template<typename T>
requires valid_i2c_regaddr<T>
Bus::Error I2cDrv::writeRegAddress(const T reg_address, const Endian endian){
    auto p = reinterpret_cast<const uint8_t *>(&reg_address);

    Bus::Error err = Bus::ErrorType::OK;

    do{
        if(endian == MSB){
            for(int i = int(sizeof(T) - 1); i >= 0; i--){
                err = bus.write(p[i]);
                if(err) break;
            }
        }else{
            for(size_t i = 0; i < sizeof(T); i++){
                err = bus.write(p[i]);
                if(err) break;
            }
        }
    }while(false);

    if(err){
        I2CDRV_DEBUG(err);
    }

    return err;
}

template<typename T>
requires valid_i2c_data<T>
void I2cDrv::writePool_impl(const valid_i2c_regaddr auto reg_address, const T * data_ptr, const size_t length, const Endian endian){
    constexpr size_t size = sizeof(T);

    if constexpr(size == 0)   return;
    if(length == 0) return;

    size_t bytes = length * size;
    auto *u8_ptr = reinterpret_cast<const uint8_t *>(data_ptr);

    auto err = bus.begin(index);
    if(err == Bus::ErrorType::OK){
        writeRegAddress(reg_address, endian);
        for(size_t i = 0; i < bytes; i += size){
            if(endian == MSB){
                for(size_t j = size; j > 0; j--){
                    bus.write(u8_ptr[j-1 + i]);
                }
            }else{
                for(size_t j = 0; j < size; j++){
                    bus.write(u8_ptr[j + i]);
                }
            }
        }

        bus.end();
    }else{
        I2CDRV_DEBUG(err)
    }
}


template<typename T>
requires valid_i2c_data<T>
void I2cDrv::writePool_impl(const valid_i2c_regaddr auto reg_address, const T data, const size_t length, const Endian endian){
    constexpr size_t size = sizeof(T);

    if constexpr(size == 0)   return;
    if(length == 0) return;

    size_t bytes = length * size;
    auto *u8_ptr = reinterpret_cast<const uint8_t *>(&data);

    auto err = bus.begin(index);
    if(err == Bus::ErrorType::OK){
        writeRegAddress(reg_address, endian);
        for(size_t i = 0; i < bytes; i += size){
            if(endian == MSB){
                for(size_t j = size; j > 0; j--){
                    bus.write(u8_ptr[j-1]);
                }
            }else{
                for(size_t j = 0; j < size; j++){
                    bus.write(u8_ptr[j]);
                }
            }
        }

        bus.end();
    }else{
        I2CDRV_DEBUG(err)
    }
}

template<typename T>
requires valid_i2c_data<T>
void I2cDrv::readPool_impl(const valid_i2c_regaddr auto reg_address, T * data_ptr, const size_t length, const Endian endian){
    if(length == 0) return;
    constexpr size_t size = sizeof(T);
    size_t bytes = length * size;
    auto * u8_ptr = reinterpret_cast<uint8_t *>(data_ptr);

    auto err = bus.begin(index);
    if(err == Bus::ErrorType::OK){
        writeRegAddress(reg_address, endian);
        if(bus.begin(index | 0x01) == Bus::ErrorType::OK){
            for(size_t i = 0; i < bytes; i += size){
                if(endian == MSB){
                    for(size_t j = size; j > 0; j--){
                        uint32_t temp = 0;
                        bus.read(temp, !((j == 1) && (i == bytes - size)));
                        u8_ptr[j-1 + i] = temp;
                    }
                }else{
                    for(size_t j = 0; j < size; j++){
                        uint32_t temp = 0;
                        bus.read(temp, (i + j != bytes - 1));
                        u8_ptr[j + i] = temp;
                    }
                }
            }
        }
        bus.end();
    }else{
        I2CDRV_DEBUG(err);
    }
}

}
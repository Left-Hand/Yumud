#if 0


struct SaveParamRegSet{
    // 保存参数寄存器 可读写 32位无符号整数
    static constexpr uint16_t NUM_PRE_IDX = 0x100D;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;
    static constexpr uint8_t cnt = 0x7f;

    static constexpr uint32_t key = 0x65766173;

    ParamSaverIntf * saver = nullptr;
    
    struct ReadStruct{
        uint32_t en:1;
        uint32_t auto_save:1;
        uint32_t :30;
    };


    SdoAbortCode write(const std::span<const uint8_t> pbuf, const OdSubIndex sidx){
        //TODO support auto save
        if(sidx != 0x00)
            return SdoAbortCode::UnsupportedAccess;

        //判断是否有保存器
        if(!saver) return SdoAbortCode::UnsupportedAccess;

        //判断写入的长度是否正确
        if(pbuf.size() != 4) 
            return SdoAbortCode::InvalidBlockSize;

        //判断是否密钥是否正确
        if(!(*reinterpret_cast<const uint32_t *>(pbuf.data()) == key))
            return SdoAbortCode::GeneralError;

        switch(sidx){
            case 0x00: __builtin_trap();
            case 0x01: saver->saveAllConfig(); break;
            case 0x02: saver->saveCommConfig(); break;
            case 0x03: saver->saveAppConfig(); break;
            default: saver->saveSpecConfig(uint8_t(sidx)); break;
        }

        return Ok();
    } 


    SdoAbortCode read(const std::span<uint8_t> pbuf, const OdSubIndex sidx) const {
        //TODO
        // ReadStruct reg = ReadStruct{pbuf};
        return Ok();
    }
};


struct LoadParamRegSet{
    // 保存参数寄存器 可读写 32位无符号整数
    static constexpr uint16_t NUM_PRE_IDX = 0x1011;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;
    static constexpr uint8_t cnt = 0x7f;

    static constexpr uint32_t key = 0x65766173;

    ParamLoaderIntf * loader = nullptr;
    
    struct ReadStruct{
        uint32_t en:1;
        uint32_t auto_save:1;
        uint32_t :30;
    };


    SdoAbortCode write(const std::span<const uint8_t> pbuf, const OdSubIndex sidx){
        //TODO support auto save
        if(sidx == 0x00){
            return SdoAbortCode::UnsupportedAccess;
        }
        //判断是否有保存器
        if(!loader) return SdoAbortCode::UnsupportedAccess;

        //判断写入的长度是否正确
        if(pbuf.size() != 4) 
            return SdoAbortCode::InvalidBlockSize;

        //判断是否密钥是否正确
        if(!(*reinterpret_cast<const uint32_t *>(pbuf.data()) == key))
            return SdoAbortCode::GeneralError;

        switch(sidx){
            case 0x00: __builtin_trap();
            case 0x01: loader->loadAllConfig(); break;
            case 0x02: loader->loadCommConfig(); break;
            case 0x03: loader->loadAppConfig(); break;
            default: loader->loadSpecConfig(uint8_t(sidx)); break;
        }


        return Ok();
    } 


    SdoAbortCode read(const std::span<uint8_t> pbuf, const OdSubIndex sidx) const {
        //TODO
        // ReadStruct reg = ReadStruct{pbuf};
        return Ok();
    }
};

#endif
#include "ObjectDict.hpp"

namespace ymd::canopen{



class ParamSaverIntf{
public:
    virtual void saveAllConfig() = 0;
    virtual void saveCommConfig() = 0;
    virtual void saveAppConfig() = 0;
    virtual void saveSpecConfig(const uint8_t idx) = 0;
};

class ParamLoaderIntf{
public:
    virtual void loadAllConfig() = 0;
    virtual void loadCommConfig() = 0;
    virtual void loadAppConfig() = 0;
    virtual void loadSpecConfig(const uint8_t idx) = 0;
};


    
struct Cia301ObjectDict:public StaticObjectDictBase{
    struct ControlWordReg:public RegC32<>{
        //控制字寄存器 只读32位
        static constexpr Index idx = 0x1000;
        static constexpr SubIndex subidx = 0x00;

        uint16_t protocol_version;
        uint16_t extra_msg;
    };

    struct ErrorReg:public RegC8<>{
        //错误寄存器 只读8位
        static constexpr Index idx = 0x1001;
        static constexpr SubIndex subidx = 0x0;

        uint8_t uni_err:1;
        uint8_t curr_err:1;
        uint8_t volt_err:1;
        uint8_t temp_err:1;
        uint8_t comm_err:1;
        uint8_t :3;
    };

    struct ManufacturerStatusReg:public RegC32<>{
        //厂商信息 只读32位
        static constexpr Index idx = 0x1002;
        static constexpr SubIndex subidx = 0x0;
        
        uint32_t manufacturer_id;
    };

    struct PerdefErrFieldReg{
        static constexpr Index idx = 0x1003;
        static constexpr SubIndex subidx = 0x0;

        #ifndef CANOPEN_MAX_PERDEF_ERROR_CNT
        static constexpr size_t CANOPEN_MAX_PERDEF_ERROR_CNT = 8;
        #endif
    

        using Error = uint32_t;
        using AbortCode = uint32_t;
        
        void addError(const Error error){error_queue_.push(error);}
        uint8_t getErrorCnt() const{return error_queue_.available();}

        std::optional<Error> getError(const size_t _idx) const {
            if(_idx >= error_queue_.available()){
                return std::nullopt;
            }else{
                TODO();
                // return error_queue_.foresee(_idx);
            }
        }

        SdoAbortCode write(const std::span<const uint8_t> pbuf, const SubIndex sidx){
            if(unlikely(sidx) != 0) 
                return SdoAbortCode::InvalidValue;
            if(unlikely(pbuf.size() < 1)) 
                return SdoAbortCode::InvalidValue;

            if(likely(!bool(pbuf[0]))){
                // error_queue_.waste(error_queue_.available());
                return SdoAbortCode::OK;
            }else{
                return SdoAbortCode::InvalidValue;
            }
        }

        SdoAbortCode read(const std::span<uint8_t> pbuf, const SubIndex sidx) const {
            static constexpr SubIndex base_idx = 1;

            if(unlikely(sidx) < 1){
                pbuf[0] = uint8_t(getErrorCnt());
                return SdoAbortCode::OK;
            }

            const auto offset = size_t(sidx) - size_t(base_idx);
            const auto may_err = getError(offset);

            if(may_err.has_value()){
                *(reinterpret_cast<Error *>(pbuf.data())) = may_err.value();
                return SdoAbortCode::OK;
            }else{
                //企图获取超界的错误
                return SdoAbortCode::NoValidData;
            }
        }
    private:
        RingBuf<Error, CANOPEN_MAX_PERDEF_ERROR_CNT> error_queue_;
    };

    struct CobidSyncMsgReg:public Reg32<>{
        static constexpr Index idx = 0x1005;
        static constexpr SubIndex subidx = 0x0;

        uint32_t cobid:29;
        uint32_t frame:1;//0:11位CANID,1:29位CANID
        uint32_t gen:1;//所在CANOPEN设备是否发出同步消息
        uint32_t :1;
    };

    struct CommCyclicPeriodReg:public Reg32<>{
        static constexpr Index idx = 0x1006;
        static constexpr SubIndex subidx = 0x0;

        uint32_t period_us;//单位us
    };

    struct SyncWindowLengthReg:public Reg32<>{
        static constexpr Index idx = 0x1007;
        static constexpr SubIndex subidx = 0x0;

        uint32_t length;
    };

    struct CoStringObj{

        StringView str;
        CoStringObj(StringView && _str):str(_str){}

        auto * c_str(){
            return str.c_str();
        }

        SdoAbortCode write(const std::span<const uint8_t> pbuf){
            if(unlikely(str.length() != pbuf.size())) return SdoAbortCode::MaxLessThanMin;

            TODO();
            // memcpy(&str[0], pbuf.data(), pbuf.size());
            return SdoAbortCode::OK;
        } 


        SdoAbortCode read(const std::span<uint8_t> pbuf) const {
            if(unlikely(str.length() != pbuf.size())) return SdoAbortCode::MaxLessThanMin;

            memcpy(pbuf.data(), str.c_str(), pbuf.size());
            return SdoAbortCode::OK;
        }
    };

    struct DeviceNameReg : public CoStringObj{
        // 设备名称寄存器 只读 字符串类型
        static constexpr Index idx = 0x1008;
        static constexpr SubIndex subidx = 0x0;

        using CoStringObj::CoStringObj;
    };

    struct HardwareVersionReg:public CoStringObj{
        static constexpr Index idx = 0x1009;
        static constexpr SubIndex subidx = 0x0;

        using CoStringObj::CoStringObj;
    };

    struct SoftwareVersionReg:public CoStringObj{
        static constexpr Index idx = 0x1010;
        static constexpr SubIndex subidx = 0x0;

        using CoStringObj::CoStringObj;
    };

    struct NodeGuardingPeriodReg : public Reg8<> {
        // 节点守护时间寄存器 可读写 16位无符号整数
        static constexpr Index idx = 0x100C;
        static constexpr SubIndex subidx = 0x0;

        uint8_t val;
    };

    struct NodeGuardingPeriodFracReg : public Reg16<> {
        // 节点守护时间寄存器 可读写 16位无符号整数
        static constexpr Index idx = 0x100D;
        static constexpr SubIndex subidx = 0x0;

        uint16_t val;
    };

    struct SaveParamRegSet{
        // 保存参数寄存器 可读写 32位无符号整数
        static constexpr Index idx = 0x100d;
        static constexpr SubIndex subidx = 0x0;
        static constexpr uint8_t cnt = 0x7f;

        static constexpr uint32_t key = 0x65766173;

        ParamSaverIntf * saver = nullptr;
        
        struct ReadStruct:public Reg32<>{
            #pragma pack(push, 1)
            uint32_t en:1;
            uint32_t auto_save:1;
            uint32_t :30;
            #pragma pack(pop)

            using Reg32::Reg32;
        };


        SdoAbortCode write(const std::span<const uint8_t> pbuf, const SubIndex sidx){
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
                case 0x00: HALT;
                case 0x01: saver->saveAllConfig(); break;
                case 0x02: saver->saveCommConfig(); break;
                case 0x03: saver->saveAppConfig(); break;
                default: saver->saveSpecConfig(uint8_t(sidx)); break;
            }

            return SdoAbortCode::OK;
        } 


        SdoAbortCode read(const std::span<uint8_t> pbuf, const SubIndex sidx) const {
            //TODO
            // ReadStruct reg = ReadStruct{pbuf};
            return SdoAbortCode::OK;
        }
    };


    struct LoadParamRegSet{
        // 保存参数寄存器 可读写 32位无符号整数
        static constexpr Index idx = 0x1011;
        static constexpr SubIndex subidx = 0x0;
        static constexpr uint8_t cnt = 0x7f;

        static constexpr uint32_t key = 0x65766173;

        ParamLoaderIntf * loader = nullptr;
        
        struct ReadStruct:public Reg32<>{
            #pragma pack(push, 1)
            uint32_t en:1;
            uint32_t auto_save:1;
            uint32_t :30;
            #pragma pack(pop)

            using Reg32::Reg32;
        };


        SdoAbortCode write(const std::span<const uint8_t> pbuf, const SubIndex sidx){
            //TODO support auto save
            if(sidx == 0x00){
                return SdoAbortCode::UnsupportedAccess;
            }else{
                //判断是否有保存器
                if(!loader) return SdoAbortCode::UnsupportedAccess;

                //判断写入的长度是否正确
                if(pbuf.size() != 4) 
                    return SdoAbortCode::InvalidBlockSize;

                //判断是否密钥是否正确
                if(!(*reinterpret_cast<const uint32_t *>(pbuf.data()) == key))
                    return SdoAbortCode::GeneralError;

                switch(sidx){
                    case 0x00: HALT;
                    case 0x01: loader->loadAllConfig(); break;
                    case 0x02: loader->loadCommConfig(); break;
                    case 0x03: loader->loadAppConfig(); break;
                    default: loader->loadSpecConfig(uint8_t(sidx)); break;
                }
            }

            return SdoAbortCode::OK;
        } 


        SdoAbortCode read(const std::span<uint8_t> pbuf, const SubIndex sidx) const {
            //TODO
            // ReadStruct reg = ReadStruct{pbuf};
            return SdoAbortCode::OK;
        }
    };

    struct TimeStampReg : public Reg32<> {
        static constexpr Index idx = 0x1012;
        static constexpr SubIndex subidx = 0x0;

        uint32_t cobid:29;
        uint32_t frame:1;
        uint32_t produce:1;
        uint32_t consume:1;
    };

    struct GpTimeStampReg : public Reg32<> {
        static constexpr Index idx = 0x1013;
        static constexpr SubIndex subidx = 0x0;

        uint32_t timestamp;
    };


    struct EmcyCobidReg : public Reg16<> {
        static constexpr Index idx = 0x1014;
        static constexpr SubIndex subidx = 0x0;

        uint32_t canid:29;
        uint32_t frame:1;
        const uint32_t __resv__:1 = 0;
        uint32_t valid:1;
    };


    struct EmcyDepressTimeReg : public Reg16<> {
        static constexpr Index idx = 0x1015;
        static constexpr SubIndex subidx = 0x0;

        uint16_t time;
    };

    struct ConsumerHeartbeatOverTimeReg : public Reg32<> {
        //心跳时间寄存器 可读写 32位无符号整数
        static constexpr Index idx = 0x1016;
        static constexpr SubIndex subidx = 0x0;

        uint16_t time;
        uint8_t node_id;
        const uint8_t __resv__ = 0;
    };

    struct ProducterHeartbeatOverTimeReg : public Reg16<> {
        //心跳时间寄存器 可读写 32位无符号整数
        static constexpr Index idx = 0x1016;
        static constexpr SubIndex subidx = 0x0;

        uint16_t time;
    };

    struct IdentificationReg : public Reg32<> {
        static constexpr Index idx = 0x1023;
    };

    struct OsCommandRegSet{
    };

    struct OsCommandModeReg:public Reg8<>{
        static constexpr Index idx = 0x1024;
        enum class Mode{

        };
    };

    struct OsDebugIntfaceRegset{
        static constexpr Index idx = 0x1025;
    };


    ControlWordReg control_word_reg;
    ErrorReg error_reg;
    PerdefErrFieldReg perdef_err_field_reg;
    CobidSyncMsgReg cobid_sync_msg_reg;
    CommCyclicPeriodReg comm_cyclic_period_reg;
    SyncWindowLengthReg sync_window_length_reg;
    DeviceNameReg device_name_reg = {"MyCanopenDevice"};
    HardwareVersionReg hardware_version_reg = {"1.0"};
    SoftwareVersionReg software_version_reg = {"1.0"};
public:
    Cia301ObjectDict() = default;

    std::optional<SubEntry> find(const Didx didx);

    SdoAbortCode write(const std::span<const uint8_t> pbuf, const Didx didx) override;
    
    SdoAbortCode read(const std::span<uint8_t> pbuf, const Didx didx) const override;

};


// class Cia301Device{
// public:
//     Cia301Device(Cia301ObjectDict& od, ) :
//         od_(od), sync_(sync), pdo_(pdo), node_id_(node_id) {}
// private:
//     Cia301ObjectDict od_;
// };


}
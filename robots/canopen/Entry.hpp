#pragma once


#include "utils.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/Option.hpp"
#include "core/string/string_view.hpp"

namespace ymd::canopen {


// Forward declaration of CanOpenListener
class SubEntry;


enum class EntryAccessAuthority : uint8_t {
    RW = 0,
    WO = 0x01,
    RO = 0x02,
    CONST = 0x03
};



class EntryDataType {
public:
    using Item = E_Item<uint8_t>;


    static constexpr Item bit{0x01};
    static constexpr Item int8{0x02};
    static constexpr Item int16{0x03};
    static constexpr Item int32{0x04};
    static constexpr Item uint8{0x05};
    static constexpr Item uint16{0x06};
    static constexpr Item uint32{0x07};
    static constexpr Item real32{0x08};
    static constexpr Item visible_string{0x09};
    static constexpr Item octet_string{0x0A};
    static constexpr Item unicode_string{0x0B};
    static constexpr Item time_of_day{0x0C};
    static constexpr Item time_difference{0x0D};
    static constexpr Item domain{0x0F};
    static constexpr Item int24{0x10};
    static constexpr Item real64{0x11};
    static constexpr Item int40{0x12};
    static constexpr Item int48{0x13};
    static constexpr Item int56{0x14};
    static constexpr Item int64{0x15};
    static constexpr Item uint24{0x16};
    static constexpr Item pdo_mapping{0x21};
    static constexpr Item sdo_parameter{0x22};
    static constexpr Item identity{0x23};

    enum class Kind:uint8_t{
        Bit,
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        Real32,
        Real64,
        Int40,
        Int48,
        Int56,
        Int64,
        Uint24,
        PdoMapping,
        SdoParameter,
        Identity
    };

    constexpr EntryDataType(Item e) : e_(e) {}

    // 类型转换操作符
    constexpr operator Item() const { return e_; }

    // 判断是否为整数类型
    constexpr bool is_int() const {return e_ <= real32;}


    template<typename T>
    static constexpr bool false_v = false;
    // 获取数据类型的大小
    constexpr size_t dsize() const {
        switch (e_.v_) {
            case bit.v_: return 1; // bit 类型通常按 1 字节处理
            case int8.v_: return 1;
            case int16.v_: return 2;
            case int24.v_: return 3;
            case int32.v_: return 4;
            case int40.v_: return 5;
            case int48.v_: return 6;
            case int56.v_: return 7;
            case int64.v_: return 8;
            case uint8.v_: return 1;
            case uint16.v_: return 2;
            case uint24.v_: return 3;
            case uint32.v_: return 4;
            case real32.v_: return 4;
            case real64.v_: return 8;
            default: return 0; // 其他类型（如字符串）大小不确定，返回 0
        }
    }

    template<typename U>
    static constexpr Item from(){
        using T = std::decay_t<U>;
        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>) {
            return uint8;
        } else if constexpr (std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t>) {
            return uint16;
        } else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>){
            return uint32;
        } else{
            static_assert(false_v<U>, "EntryDataType::from() only support int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t");
        }
    }

private:
    Item e_;
};



struct SubEntry {
public:
    using AccessAuthority = EntryAccessAuthority;
    using DataType = EntryDataType;


    SubEntry(const SubEntry &) = default;
    SubEntry(SubEntry &&) = default;

    SubEntry & operator = (const SubEntry &) = default;
    SubEntry & operator = (SubEntry &&) = default;

    template<typename T>
    constexpr SubEntry(
        const StringView name, 
        T && val, 
        AccessAuthority access_authority, 
        DataType data_type
    ): 
        name_(name), 
        access_authority_(access_authority), 
        data_type_(data_type), 
        obj_ref_(std::forward<T>(val)){}


    template<typename T>
    static constexpr SubEntry from_u32_rw(
        const StringView name, 
        T && val 
    ){
        return SubEntry{
            name, std::forward<T>(val),
            AccessAuthority::RW,
            DataType::int32
        };
    }

    template<typename T>
    static constexpr SubEntry from_u16_rw(
        const StringView name, 
        T && val
    ){
        return SubEntry{
            name, std::forward<T>(val),
            AccessAuthority::RW,
            DataType::int16
        };
    }

    template<typename T>
    static constexpr SubEntry from_u8_rw(
        const StringView name, 
        T && val
    ){
        return SubEntry{
            name, std::forward<T>(val),
            AccessAuthority::RW,
            DataType::int8
        };
    }

    explicit operator int() const ;

    template<integral T>
    explicit operator T() const{return int(*this);}

    SdoAbortCode read(std::span<uint8_t> pbuf) const;

    SdoAbortCode write(const std::span<const uint8_t> pbuf);

    SdoAbortCode read_any(void * pbuf);

    SdoAbortCode write_any(const void * pbuf);

    SubEntry copy() const{return *this;}

	size_t dsize() const {return data_type_.dsize();}
	size_t size() const {return data_type_.dsize();}

    bool is_readable() const {return access_authority_ != AccessAuthority::WO;}
    bool is_writeable() const {return access_authority_ == AccessAuthority::RW || access_authority_ == AccessAuthority::WO;}
    StringView name() const {return StringView(name_);}

    void put(const hal::BxCanFrame & frame){
        TODO();
    }
private:
#pragma pack(push, 1)
    StringView name_;
    AccessAuthority access_authority_;
    DataType data_type_;
#pragma pack(pop)
    struct ObjRef{

    public:
        constexpr ObjRef(uint32_t data):is_ref_(false), data32_(data){}

        
        template<typename T>
        requires ((sizeof(std::decay_t<T>) == 4) and std::is_standard_layout_v<std::decay_t<T>> )
        constexpr ObjRef(const T & pbuf):
            is_ref_(true), 
            pbuf_(reinterpret_cast<void *>(const_cast<T *>(&pbuf))){}

        template<typename T>
        requires ((sizeof(std::decay_t<T>) == 4) and std::is_standard_layout_v<std::decay_t<T>> )
        constexpr ObjRef(T && pbuf):
            is_ref_(false), pbuf_(reinterpret_cast<void *>(&pbuf)){}

        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        constexpr void write(const T val){
            if(likely(is_ref_)){
                *reinterpret_cast<T*>(pbuf_) = val;
            }else{
                data32_ = std::bit_cast<T>(val);
            }
        }

        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        constexpr void read(T & val) const{
            if(likely(is_ref_)){
                val = *reinterpret_cast<T*>(pbuf_);
            }else{
                val = std::bit_cast<T>(data32_);
            }
        }

        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        constexpr T read() const {
            if(likely(is_ref_)){
                return *reinterpret_cast<T*>(pbuf_);
            }else{
                return std::bit_cast<T>(data32_);
            }
        }

        constexpr const void * data() const{
            return likely(is_ref_) ? pbuf_ : (&data32_);
        }

        constexpr void * data(){
            return likely(is_ref_) ? pbuf_ : (&data32_);
        }

        constexpr ObjRef & operator =(const auto val){this->write(val); return *this;}
        
        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        explicit constexpr operator T(){return this->read<T>();}

    private:
        bool is_ref_ = false;
        bool is_const_ = false;
        union{
            void * pbuf_;
            uint32_t data32_;
        };
    };

    ObjRef obj_ref_;
};


class OdEntry final{
private:
    using Index = OdIndex; 
    using SubIndex = OdSubIndex; 


	StringView name_;
	std::vector<SubEntry> subentries_ = {};
public:
    OdEntry(const OdEntry &) = default;
    OdEntry(OdEntry &&) = default;

    OdEntry & operator=(const OdEntry &) = default;
    OdEntry & operator=(OdEntry &&) = default;
    
	OdEntry(const StringView name):
        name_(name){}

	OdEntry(): name_(nullptr){}
    
	size_t size(){return(subentries_.size());}

	void add(SubEntry && sub){
		subentries_.push_back(sub);
	}

    Option<SubEntry> operator [](const SubIndex idx) const {
        if(idx >= subentries_.size()) return None;
        return Some(subentries_[idx]);
    }

    StringView name() const {
        return StringView(name_);
    }
};


} // namespace ymd::canopen
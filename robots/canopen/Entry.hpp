#pragma once


#include "utils.hpp"
#include "core/debug/debug.hpp"

namespace ymd::canopen {


// Forward declaration of CanOpenListener
class SubEntry;


enum class EntryAccessType : uint8_t {
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
    static Item from(){
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



class SubEntry {
public:
    using AccessType = EntryAccessType;
    using DataType = EntryDataType;
private:
#pragma pack(push, 1)
    StringView name_;
    AccessType access_type_;
    DataType data_type_;
#pragma pack(pop)
    class ObjRef{
    private:
        bool is_ref_ = false;
        bool is_const_ = false;
        union{
            void * pdata_;
            uint32_t data32_;
        };
    public:
        constexpr ObjRef(uint32_t data):is_ref_(false), data32_(data){}

        
        template<typename T>
        requires ((sizeof(std::decay_t<T>) == 4) and std::is_standard_layout_v<std::decay_t<T>> and std::is_lvalue_reference_v<T>)
        constexpr ObjRef(const T & pdata):is_ref_(true), pdata_(reinterpret_cast<void *>(const_cast<T *>(&pdata))){}

        template<typename T>
        constexpr ObjRef(const T && pdata):is_ref_(false), pdata_(reinterpret_cast<void *>(&pdata)){}

        // template<typename T>
        // requires (sizeof(std::decay_t<T>)= 4 and std::is_standard_layout_v<std::decay_t<T>> and std::is_rvalue_reference_v<T>)
        // constexpr ObjRef(T pdata):is_ref_(true), data32_((pdata)){
        //     // static_assert(false);
        //     // static_assert(std::is_lvalue_reference_v<T>);
        // }


        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        constexpr void write(const T val){
            if(likely(is_ref_)){
                *reinterpret_cast<T*>(pdata_) = val;
            }else{
                data32_ = std::bit_cast<T>(val);
            }
        }

        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        constexpr void read(T & val) const{
            if(likely(is_ref_)){
                val = *reinterpret_cast<T*>(pdata_);
            }else{
                val = std::bit_cast<T>(data32_);
            }
        }

        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        constexpr T read() const {
            if(likely(is_ref_)){
                return *reinterpret_cast<T*>(pdata_);
            }else{
                return std::bit_cast<T>(data32_);
            }
        }

        constexpr const void * data() const{
            return likely(is_ref_) ? pdata_ : (&data32_);
        }

        constexpr void * data(){
            return likely(is_ref_) ? pdata_ : (&data32_);
        }

        constexpr ObjRef & operator =(const auto val){this->write(val); return *this;}
        
        template<typename T>
        requires (sizeof(T) <= 4 and std::is_standard_layout_v<T>)
        explicit constexpr operator T(){return this->read<T>();}

    };

    ObjRef obj_;

public:
    SubEntry(const SubEntry &) = default;
    SubEntry(SubEntry &&) = default;

    SubEntry & operator = (const SubEntry &) = default;
    SubEntry & operator = (SubEntry &&) = default;

    template<typename T>
    constexpr SubEntry(const StringView name, T && val, AccessType access_type, DataType data_type)
        : name_(name), access_type_(access_type), data_type_(data_type), obj_(std::forward<T>(val)){}

    explicit operator int() const ;

    template<integral T>
    explicit operator T() const{return int(*this);}



    template<typename T>
    requires ((sizeof(T) <= 4) and (!std::is_pointer_v<T>))
    SdoAbortCode write_any(const T pdata){
        return write_any((&pdata));
    }

    template<typename T>
    requires ((sizeof(T) <= 4) and (!std::is_pointer_v<T>))
    SdoAbortCode read_any(T & pdata){
        return read_any((&pdata));
    }


    SdoAbortCode read(std::span<uint8_t> pdata) const;

    SdoAbortCode write(const std::span<const uint8_t> pdata);

    SdoAbortCode read_any(void * pdata);

    SdoAbortCode write_any(const void * pdata);

    SubEntry copy() const{return *this;}

	size_t dsize() const {return data_type_.dsize();}
	size_t size() const {return data_type_.dsize();}

    bool is_readable() const {return access_type_ != AccessType::WO;}
    bool is_writeable() const {return access_type_ == AccessType::RW || access_type_ == AccessType::WO;}
    StringView name() const {return StringView(name_);}

    void put(const hal::CanMsg & msg){
        TODO();
    }

};


class OdEntry{
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

	OdEntry():
        name_(std::nullopt){}
    
	size_t size(){return(subentries_.size());}

	void add(SubEntry && sub){
		subentries_.push_back(sub);
	}

    std::optional<SubEntry> operator [](const SubIndex idx) const {
    // optref<SubEntry> operator [](const SubIndex idx){
        return (subentries_[idx]);
    }

    StringView name() const {
        return StringView(name_);
    }
};


} // namespace ymd::canopen
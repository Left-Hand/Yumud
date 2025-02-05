#pragma once


#include "utils.hpp"

namespace ymd::canopen {


// Forward declaration of CanOpenListener
class SubEntry;

class CanOpenListener {
public:
    virtual void onObjDictChange(SubEntry* subEntry) = 0;
    virtual void onMessage(const CanMsg & msg);
    virtual ~CanOpenListener() = default;
};



class SubEntry {
public:
    // AccessType enum
    enum class AccessType : uint8_t {
        RW = 0,
        WO = 0x01,
        RO = 0x02,
        CONST = 0x03
    };

    enum class DataType : uint16_t {
        bit = 0x01, // is boolean in spec, but that is a reserved keyword in C++
        int8 = 0x02, 
        int16 = 0x03, 
        int32 = 0x04, 
        uint8 = 0x05, 
        uint16 = 0x06, 
        uint32 = 0x07, 
        real32 = 0x08,

        // visible_string = 0x09, 
        // octet_string = 0x0A, 
        // unicode_string = 0x0B,

        time_of_day = 0x0C, time_difference = 0x0D,

        domain = 0x0F,
        int24 = 0x10,
        real64 = 0x11, 
        int40 = 0x12, 
        int48 = 0x13, 
        int56 = 0x14, 
        int64 = 0x15, 
        uint24 = 0x16,
        
        pdo_mapping = 0x21, sdo_parameter = 0x22, identity = 0x23
    };

    SubEntry(AccessType accessT, DataType dataT, int size, const StringView& name)
        : accessType(accessT), dataType(dataT), size_(size), pname(name), pdoMapping(false) {}

    // SubEntry(AccessType accessT, DataType dataT, int size, const StringView& name, std::any obj)
    //     : SubEntry(accessT, dataT, size, name) {
    //     pObject = obj;
    // }

    // SubEntry(AccessType accessT, const StringView& name, const StringView& val)
    //     : SubEntry(accessT, DataType::visible_string, val.length(), name) {
    //     pObject = val;
    // }

    SubEntry(AccessType accessT, const StringView& name, int x)
        : SubEntry(accessT, DataType::uint32, 4, name) {
        pObject = x;
    }

    SubEntry(AccessType accessT, const StringView& name, short x)
        : SubEntry(accessT, DataType::uint16, 2, name) {
        pObject = static_cast<int>(x);
    }

    SubEntry(AccessType accessT, const StringView& name, uint8_t x)
        : SubEntry(accessT, DataType::uint8, 1, name) {
        byte1 = x;
    }

    SubEntry(AccessType accessT, const StringView& name, bool x)
        : SubEntry(accessT, DataType::int8, 1, name) {
        pObject = x;
    }
    void addListener(CanOpenListener & coListener) {
        listeners.push_back(&coListener);
    }

    void removeListener(CanOpenListener & coListener) {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), &coListener), listeners.end());
    }

    void notifyListeners() {
        for (const auto& listener : listeners) {
            listener->onObjDictChange(this);
        }
    }

    std::vector<uint8_t> getByteBuffer() const {
        std::vector<uint8_t> retval(size_);
        if (dataType == DataType::uint8) {
            retval[0] = byte1;
        } else if (dataType == DataType::uint32) {
            int val = std::bit_cast<int>(pObject);
            retval[0] = val & 0xFF;
            retval[1] = (val >> 8) & 0xFF;
            retval[2] = (val >> 16) & 0xFF;
            retval[3] = (val >> 24) & 0xFF;
        } else if (dataType == DataType::uint16) {
            int val = std::bit_cast<int>(pObject);
            retval[0] = val & 0xFF;
            retval[1] = (val >> 8) & 0xFF;
        }
        return retval;
    }

    int getInt() const {
        if (dataType == DataType::uint8) {
            return byte1;
        } else if (dataType == DataType::uint32 || dataType == DataType::int32) {
            return std::bit_cast<int>(pObject);
        } else if (dataType == DataType::uint16) {
            return std::bit_cast<int>(pObject) & 0x0000FFFF;
        }
        return 0;
        // COException::invalidLength("dataType: 0x" + std::to_string(static_cast<int>(dataType)));
    }

    operator int() const {
        return getInt();
    }

    void set(int val) {
        if (accessType == AccessType::CONST || accessType == AccessType::RO) {
            // COException::isReadOnly("Value cannot be written, Read only or Const");
        }

        if (dataType == DataType::uint8) {
            byte1 = static_cast<uint8_t>(val);
        } else if (dataType == DataType::uint32 || dataType == DataType::int32) {
            pObject = val;
        } else if (dataType == DataType::uint16) {
            pObject = val & 0x0000FFFF;
        } else {
            // COException::notMappable("unable to cast datatype from int is " + std::to_string(static_cast<int>(dataType)));
        }
        notifyListeners();
    }

    void setIgnorePermissions(int val) {
        if (dataType == DataType::uint8) {
            byte1 = static_cast<uint8_t>(val);
        } else if (dataType == DataType::uint32 || dataType == DataType::int32) {
            pObject = val;
        } else if (dataType == DataType::uint16) {
            pObject = val & 0x0000FFFF;
        } else {
            // COException::notMappable("unable to cast datatype from int is " + std::to_string(static_cast<int>(dataType)));
        }
        notifyListeners();
    }

    void put(const std::vector<uint8_t>& val) {
        if (accessType == AccessType::CONST || accessType == AccessType::RO) {
            // COException::isReadOnly("Value cannot be written, Read only or Const");
        }

        if (dataType == DataType::uint8) {
            byte1 = val[0];
        } else if (dataType == DataType::uint32 || dataType == DataType::int32) {
            int value = (val[0] & 0xFF) | ((val[1] & 0xFF) << 8) | ((val[2] & 0xFF) << 16) | ((val[3] & 0xFF) << 24);
            pObject = value;
        } else if (dataType == DataType::uint16) {
            int value = (val[0] & 0xFF) | ((val[1] & 0xFF) << 8);
            pObject = value & 0x0000FFFF;
        } else {
            // COException::notMappable("unable to cast datatype from ByteBuffer to " + std::to_string(static_cast<int>(dataType)));
        }
        notifyListeners();
    }

    template<typename T>
    void put(const T & val){
        
    }

private:
    const AccessType accessType;
    const DataType dataType;
    const size_t size_;
    const String pname;

    uint32_t pObject;
    // std::any pObject;
    uint8_t byte1;
    bool pdoMapping;
    std::vector<CanOpenListener * > listeners;
};


class OdEntry{
private:
    using Index = uint16_t; 
    using SubIndex = uint8_t; 


	const Index index_;
	const String name_;
	std::vector<SubEntry *> subentries_ = {};
public:

	OdEntry(Index index, StringView name):
        index_(index),
        name_(name){}

	size_t size(){return(subentries_.size());}

	void appendSub(SubEntry & sub){
		subentries_.push_back(&sub);
	}

    SubEntry & getSub(SubIndex i){
        return(*subentries_[i]);
    }

    const SubEntry & getSub(SubIndex i) const {
        return(*subentries_[i]);
    }

    StringView name() const {
        return StringView(name_);
    }

    Index index() const { return index_; }
};


} // namespace ymd::canopen
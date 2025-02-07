#pragma once

#include "Entry.hpp"
#include <map>

namespace ymd::canopen{

class ObjectDictionary{
private:
    using Index = OdIndex;

    struct Hasher{
        size_t operator()(const Index & index) const{
            return index;
        }
    };

    using SubIndex = OdSubIndex;

    // std::unordered_map<Index, OdEntry *, Hasher> dict_;
    std::map<Index, OdEntry *> dict_;
public:
    ObjectDictionary() = default;

    optref<OdEntry> operator [](const Index index){
        return dict_[index];
    }

    optref<SubEntry> operator [](const Index index, const SubIndex subindex){
        auto entry_opt = ((*this)[index]);
        if (entry_opt.has_value())
            return entry_opt.value()[subindex];
        else
            return std::nullopt;
    }

	void insert(OdEntry & odEntry, const uint16_t idx){
		dict_[idx] = &odEntry;
	}
};

}
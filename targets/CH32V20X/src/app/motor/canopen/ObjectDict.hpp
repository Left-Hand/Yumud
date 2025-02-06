#pragma once

#include "Entry.hpp"
#include <unordered_map>

namespace ymd::canopen{




class ObjectDictionary{
private:
    using Index = OdIndex;
    using SubIndex = OdSubIndex;

    std::unordered_map<Index, OdEntry *> dict_;
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

	void insert(OdEntry & odEntry){
		dict_[odEntry.index()] = &odEntry;
	}
};

}
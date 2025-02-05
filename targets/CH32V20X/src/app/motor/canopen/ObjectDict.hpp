#pragma once

#include "Entry.hpp"
#include <unordered_map>

namespace ymd::canopen{

class ObjectDictionary{
private:
    using Index = uint16_t;
    using SubIndex = uint8_t;

    std::unordered_map<Index, OdEntry *> dict_;

public:
    ObjectDictionary() = default;

	void addListener(int index, int subindex, CanOpenListener & coListener){
		SubEntry & se = getSubEntry(index, subindex);
		se.addListener(coListener);
	}


	OdEntry & getEntry(Index index){
		OdEntry & oe = *dict_[index];
		return (oe);
	}

	SubEntry & getSubEntry(Index index, SubIndex subindex){
		OdEntry & oe = getEntry(index);
		return( oe.getSub(subindex));
	}


	void insert(OdEntry & odEntry){
		dict_[odEntry.index()] = &odEntry;
	}
};

}
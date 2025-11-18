#include "storage.hpp"
#include "memory.hpp"

using namespace ymd;

// Memory StorageBase::slice(const AddressView view){
//     return Memory(*this, );
// }

// Memory StorageBase::slice(const StorageBase::Address from, const StorageBase::Address to){
//     // return Memory(*this, view().intersection(StorageBase::AddressView{from, to}));
//     // DEBUG_PRINTLN(from, to);
//     return Memory(*this, {from, to});
// }

// void StorageBase::store(const Address loc, const void * data, const Address len){
//     if(view().has(loc)){
//         entry_store();
//         store_bytes(loc, data, len);
//         exit_store();
//     }
// }

// void StorageBase::load(const Address loc, void * data, const Address len){
//     if(view().has(loc)){
//         entry_load();
//         load_bytes(loc, data, len);
//         exit_load();
//     }
// }


// void StorageBase::erase(const Address loc, const size_t len){
//     if(view().has(loc)){
//         entry_store();
//         erase_bytes(loc, len);
//         exit_store();
//     }
// }

#include "memory.hpp"

using namespace yumud;
Memory::Memory(Storage & _storage, const AddressView & _window):
        storage_(_storage),view_(_window.intersection(_storage.view())){;}


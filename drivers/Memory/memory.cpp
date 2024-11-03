#include "memory.hpp"

using namespace yumud;
Memory::Memory(Storage & _storage, const AddressView & _window):
        storage(_storage),m_view(_window.intersection(_storage.view())){;}


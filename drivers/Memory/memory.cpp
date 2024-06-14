#include "memory.hpp"

Memory::Memory(Storage & _storage, const AddressWindow & _window):
        storage(_storage),m_window(_window.intersection(_storage.window())){;}


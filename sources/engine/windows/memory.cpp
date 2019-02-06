#include "memory.hpp"

#include <stdexcept>

#include <Windows.h>

namespace Windows
{
    static void* const baseAddress = reinterpret_cast<void*>(Terabytes(2));

    Memory::Memory()
        : Game::Memory{ Megabytes(64), Gigabytes(1) }
    {
        uint64 TotalSize = PermanentStorageSize + TransientStorageSize;
        PermanentStorage = VirtualAlloc(baseAddress, (SIZE_T)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!PermanentStorage)
        {
            throw std::domain_error{ "Fail to allocate virtual memory!" };
        }
        TransientStorage = (static_cast<uint8*>(PermanentStorage) + PermanentStorageSize);
    }

    Memory::~Memory() { VirtualFree(baseAddress, 0, MEM_RELEASE); }

} // namespace Windows
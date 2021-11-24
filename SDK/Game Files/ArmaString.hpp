#pragma once
#include <Windows.h>

class ArmaString
{
    uintptr_t Address = 0;
public:

	ArmaString(uintptr_t Address)
	{
		this->Address = Address;
	}

    std::string GetContents()
    {
        uint32_t StringSize = Memory::ReadMemory<uint32_t>(Address + 0x8);
        if (StringSize < 256)
        {
            char* Buffer = new char[StringSize];
            Memory::ReadMemoryRaw(Address + 0x10, (uintptr_t)&Buffer[0], StringSize);
            std::string ReturnBuffer = Buffer;
            delete Buffer;
            return ReturnBuffer;
        }
        else
            return "error";
    }
};

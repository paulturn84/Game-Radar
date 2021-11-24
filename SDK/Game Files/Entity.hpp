#include <Windows.h>
#include "ArmaString.hpp"
#include "VisualState.hpp"
#include "../../Core/Memory/Memory.hpp"

class EntityType
{
	uintptr_t Address = 0;
public:
	EntityType(uintptr_t Address)
	{
		this->Address = Address;
	}

	ArmaString GetKlassName()
	{
		uintptr_t ClassName = Memory::ReadMemory<uintptr_t>(Address + 0xA0);
		return ArmaString(ClassName);
	}

	ArmaString GetSimName()
	{
		uintptr_t SimName = Memory::ReadMemory<uintptr_t>(Address + 0x68);
		return ArmaString(SimName);
	}
};

class Entity
{
	uintptr_t Address = 0;

public: 
	Entity(uintptr_t Address) 
	{
		this->Address = Address;
	}

	EntityType GetEntityType()
	{
		EntityType Type = Memory::ReadMemory<uintptr_t>(Address + 0xE0);
		return Type;
	}

	VisualState GetVisualState()
	{
		VisualState State = Memory::ReadMemory<uintptr_t>(Address + 0x130);
		return State;
	}
};
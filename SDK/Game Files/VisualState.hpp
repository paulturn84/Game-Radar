#pragma once
#include <Windows.h>
#include <vector>
#include <stdio.h>
#include<array>
#include "../../Core/Memory/Memory.hpp"
#include "Vectors.hpp"

class VisualState
{
    uintptr_t Address = 0;
public:

    VisualState(uintptr_t Address)
    {
        this->Address = Address;
    }

    Vector3 GetDirection()
    {
        Vector3 Direction = Memory::ReadMemory<Vector3>(Address + 0x8); // (Important) Body Direction {(-2, 2),(-2, 2)}
        return Direction;
    }

    Vector3 GetCoordinates()
    {
        Vector3 Coordinates = Memory::ReadMemory<Vector3>(Address + 0x2C);
        return Coordinates;
    }
};

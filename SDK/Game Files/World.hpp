#pragma once
#include "ArmaString.hpp"
#include "Entity.hpp"
#include "VisualState.hpp"
#include "../../Header.h"

class World
{
    uintptr_t Address = 0;
public:

    World(uintptr_t Address)
    {
        this->Address = Address;
    }

    std::vector<Entity> GetEntities()
    {
        
        std::vector<Entity> Return = {}; // Create a vector where we will eventually store all (near and far) entities
 

        // Close entities
        uintptr_t CloseEntityTable = Memory::ReadMemory<uintptr_t>(Address + 0xE90); // Specify the STARTING address of the close entity table
        uint32_t CloseEntityCount = Memory::ReadMemory<uint32_t>(Address + 0xE98);   // Specify the address of the number of items in close entity table

        // Far entities
        uintptr_t FarEntityTable = Memory::ReadMemory<uintptr_t>(Address + 0xFD8);
        uintptr_t FarEntityCount = Memory::ReadMemory<uint32_t>(Address + 0xFE0);



        for (int c = 0; c < CloseEntityCount; c++)
        {
            uintptr_t Entity = Memory::ReadMemory<uintptr_t>(CloseEntityTable + c * 8);
            Return.push_back(Entity);
        }

        
        for (int f = 0; f < FarEntityCount; f++)
        {
            uintptr_t Entity = Memory::ReadMemory<uintptr_t>(FarEntityTable + f * 8);
            Return.push_back(Entity);
        }

        return Return;
    }
};

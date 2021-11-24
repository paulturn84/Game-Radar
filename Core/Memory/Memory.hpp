#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <stdint.h>
#include <iostream>
#include <vector>

namespace Memory
{
	DWORD ProcessID = 0;
	HANDLE hProcess = NULL;

	DWORD GetProcID(const wchar_t* proc_name)
	{
		DWORD procID = 0;
		HANDLE const h_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (h_snap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 proc_entry;
			proc_entry.dwSize = sizeof(proc_entry);

			if (Process32First(h_snap, &proc_entry))
			{
				do
				{
					if (!_wcsicmp(proc_entry.szExeFile, proc_name))
					{
						ProcessID = procID = proc_entry.th32ProcessID;
						break;
					}
				} while (Process32Next(h_snap, &proc_entry));
			}
		}
		CloseHandle(h_snap);
		return procID;
	}

	uintptr_t FindBaseAddress()
	{
		uintptr_t baseAddress = 0;
		HANDLE const h_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, ProcessID);

		if (h_snap != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 mod_entry;
			mod_entry.dwSize = sizeof(mod_entry);

			if (Module32First(h_snap, &mod_entry))
			{
				do
				{
					if (mod_entry.th32ProcessID == ProcessID)
					{
						baseAddress = reinterpret_cast<uintptr_t>(mod_entry.modBaseAddr);
						break;
					}
				} while (Module32Next(h_snap, &mod_entry));
			}
		}
		CloseHandle(h_snap);
		return baseAddress;
	}

	uintptr_t FindAddress(uintptr_t baseAddress, std::vector<unsigned int> offsets)
	{
		auto address = baseAddress;
		std::cout << address << std::endl;

		for (unsigned int offset : offsets)
		{
			ReadProcessMemory(hProcess, reinterpret_cast<BYTE*>(address), &address, sizeof(address), nullptr);
			address += offset;
			std::cout << address << std::endl;
		}

		return address;
	}

	bool ReadMemoryRaw(uintptr_t Address, uintptr_t BufferAddress, uint32_t Size) 
	{
		return ReadProcessMemory(hProcess, (LPCVOID)Address, (LPVOID)BufferAddress, Size, nullptr);
	}

	template <typename T>
	T ReadMemory(uintptr_t target)
	{
		T memoryValue = 0;
		ReadProcessMemory(hProcess, (LPCVOID)target, &memoryValue, sizeof(T), nullptr);
		return memoryValue;
	}

	template <typename T>
	T WriteMemory(uintptr_t target, T newValue)
	{
		WriteProcessMemory(hProcess, (LPVOID)target, &newValue, sizeof(T), nullptr);
		return newValue;
	}
}

#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <psapi.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <d3d9.h>
#include <d3dx9.h>
#include<dwrite.h>
#include<dwmapi.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dwrite.lib")

class Memory
{
	HMODULE Module;
	HANDLE Proc;
public:

	HMODULE GetModule(HANDLE Proc)
	{
		HANDLE hThread = CreateRemoteThread(Proc, 0, 0, (LPTHREAD_START_ROUTINE)&GetModuleHandleA, NULL, 0, NULL);
		WaitForSingleObject(hThread, INFINITE);
		DWORD ExitCode;
		GetExitCodeThread(hThread, &ExitCode);
		return (HMODULE)ExitCode;
	}

	Memory(HANDLE Proc) : Proc(Proc), Module(GetModule(Proc)) {}
	Memory() : Proc(NULL), Module(NULL) {}

	template <class T>
	T Read(HANDLE Loc, bool Offset = false)
	{
		T Buffer;
		if (Offset)
			ReadProcessMemory(this->Proc, (LPCVOID)((DWORD)this->Module + (DWORD)Loc), &Buffer, sizeof(T), 0);
		else
			ReadProcessMemory(this->Proc, (LPCVOID)Loc, &Buffer, sizeof(T), 0);
		return Buffer;
	}

	template <class T>
	T Read(DWORD Loc, bool Offset = false)
	{
		return Read<T>((HANDLE)Loc, Offset);
	}

	template <class T>
	void Write(HANDLE Loc, T Value, bool Offset = false)
	{
		if (Offset)
			WriteProcessMemory(this->Proc, (LPVOID)((DWORD)this->Module + (DWORD)Loc), &Value, sizeof(T), 0);
		else
			WriteProcessMemory(this->Proc, (LPVOID)Loc, &Value, sizeof(T), 0);
	}

	template <class T>
	void Write(DWORD Loc, T Value, bool Offset = false)
	{
		return Write<T>((HANDLE)Loc, Value, Offset);
	}

	HANDLE CalculateOffset(HANDLE Loc)
	{
		return (HANDLE)((DWORD)this->Module + (DWORD)Loc - 0x400000);
	}

	DWORD CalculateOffset(DWORD Loc)
	{
		return (DWORD)CalculateOffset((HANDLE)Loc);
	}

	HANDLE CalculateOffsetCE(HANDLE Loc)
	{
		return (HANDLE)((DWORD)this->Module + (DWORD)Loc);
	}

	DWORD CalculateOffsetCE(DWORD Loc)
	{
		return (DWORD)CalculateOffsetCE((HANDLE)Loc);
	}

	void SetProcess(HANDLE Proc)
	{
		this->Proc = Proc;
		this->Module = GetModule(Proc);
	}

	template <class T>
	T* Allocate(T Value)
	{
		HANDLE P = VirtualAllocEx(this->Proc, NULL, sizeof(T), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		Write<T>(P, Value);
		return (T*)P;
	}

	LPCVOID AllocateFunc(LPCVOID Func)
	{
		size_t Size = 0;
		while (((BYTE*)Func)[Size++] != 0xC3) {};
		HANDLE P = VirtualAllocEx(this->Proc, NULL, Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		WriteProcessMemory(this->Proc, P, Func, Size, NULL);
		return P;
	}

	LPCSTR AllocateString(LPCSTR Str)
	{
		LPCSTR P = (LPCSTR)VirtualAllocEx(this->Proc, NULL, strlen(Str), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		WriteProcessMemory(this->Proc, (LPVOID)P, Str, strlen(Str), NULL);
		return P;
	}
};


#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "Misc.h"
#include "Memory.h"
#include <d3d9.h>
#include <d3dx9.h>
#include<dwrite.h>
#include<dwmapi.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dwrite.lib")

class RBX
{
public:

	Memory Mem;
	HANDLE Proc;

	DWORD DataModel, Workspace, Players, LocalPlayer, LocalCharacter, LocalHumanoid, LocalHead, LocalTorso, LocalLeftLeg, Camera, VisualEngine;
	static DWORD Zombies;
	static unsigned int swidth, sheight;

	int s_width = 800;
	int s_height = 600;

	//Enable Options.
	bool espenabled = true;

	//ESP Options.
	bool boxesp = true;
	bool nameesp = true;

	D3DXMATRIX ViewMatrix;

	DWORD Scan(HANDLE Process, DWORD VFTable)
	{
		SYSTEM_INFO SysInfo;
		MEMORY_BASIC_INFORMATION MemInfo;
		GetSystemInfo(&SysInfo);
		DWORD* Buf = new DWORD[SysInfo.dwPageSize];
		for (DWORD Addr = 0; Addr < 0x7FFFFFFF; Addr += SysInfo.dwPageSize)
		{
			VirtualQueryEx(Process, (LPCVOID)Addr, &MemInfo, SysInfo.dwPageSize);
			if (MemInfo.Protect == PAGE_READWRITE)
			{
				ReadProcessMemory(Process, (LPCVOID)Addr, Buf, SysInfo.dwPageSize, NULL);
				for (DWORD i = 0; i <= SysInfo.dwPageSize / 4; i++)
				{
					if (Buf[i] == VFTable)
					{
						delete[] Buf;
						return (DWORD)(Addr + (i * 4));
					}
				}
			}
		}
		delete[] Buf;
		return 0;
	}

	HANDLE LoadRobloxProcess()
	{
		HANDLE Proc;
		HWND Roblox = FindWindow(NULL, "ROBLOX"); //Find the roblox window.
		if (!Roblox)
		{
			std::cout << "Roblox was not found!" << std::endl;
			Sleep(2500);
			exit(EXIT_FAILURE);
		}

		DWORD ProcId;
		GetWindowThreadProcessId(Roblox, &ProcId);

		Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcId);
		return Proc;
	}

	DWORD GetLocalPlayer(DWORD players)
	{
		return Mem.Read<DWORD>(players + 0xC4);
	}

	DWORD GetCharacter(DWORD Player)
	{
		return Mem.Read<DWORD>(Player + 0x58);
	}

	std::string RbxCopyString(const char* s)
	{
		std::string newS;
		size_t counter = 0;
		unsigned char c;
		while (c = Mem.Read<unsigned char>((DWORD)s + counter))
		{
			counter++;
			newS.push_back(c);
		}
		return newS;
	}

	int GetParent(DWORD Instance)
	{
		return Mem.Read<DWORD>(Instance + 0x34);
	}

	std::string GetName(DWORD Instance)
	{
		std::string* str = Mem.Read<std::string*>(Instance + 0x28);
		if (Mem.Read<size_t>((DWORD)str + 20) >= 16)
			return RbxCopyString(Mem.Read<const char*>(str));
		else
			return Mem.Read<std::string>(str);
	}

	std::vector<DWORD> GetChildren(DWORD Instance)
	{
		std::vector<DWORD> Children;
		DWORD Start = Mem.Read<DWORD>(Instance + 0x2C);
		DWORD End = Mem.Read<DWORD>(Start + 4);
		for (DWORD i = Mem.Read<DWORD>(Start); i < End; i += 8)
		{
			Children.push_back(Mem.Read<DWORD>(i));
		}
		return Children;
	}

	DWORD FindFirstChild(DWORD Instance, std::string Name)
	{
		std::vector<DWORD> Children = GetChildren(Instance);
		for (DWORD i = 0; i < Children.size(); i++)
		{
			if (GetName(Children[i]) == Name)
				return Children[i];
		}
		return 0;
	}

	D3DXVECTOR3 GetPlayerPosition(DWORD Instance)
	{
		DWORD Primitive = Mem.Read<DWORD>(Instance + 0x9C);
		DWORD Body = Mem.Read<DWORD>(Primitive + 0x80);
		D3DXVECTOR3 Position = Mem.Read<D3DXVECTOR3>(Body + 0xD4);
		return Position;
	}

	void SetViewMatrix()
	{
		DWORD Link2RenderView = Mem.Read<DWORD>(DataModel + 0x70);
		DWORD RenderView = Mem.Read<DWORD>(Link2RenderView + 0x14);
		DWORD AvEngine = Mem.Read<DWORD>(RenderView + 0x8);
		ViewMatrix = Mem.Read<D3DXMATRIX>(AvEngine + 0xA0);
	}

	void LoadAddresses()
	{
		DWORD ScriptContext = Scan(Proc, Mem.CalculateOffset(0x1861570));
		DataModel = RBX::GetParent(ScriptContext);
		Workspace = RBX::FindFirstChild(DataModel, "Workspace");
		Players = RBX::FindFirstChild(DataModel, "Players");
		Camera = RBX::FindFirstChild(Workspace, "Camera");
		LocalPlayer = RBX::GetLocalPlayer(Players);
		LocalCharacter = RBX::FindFirstChild(Workspace, RBX::GetName(LocalPlayer));
		LocalHumanoid = RBX::FindFirstChild(LocalCharacter, "Humanoid");
		LocalHead = RBX::FindFirstChild(LocalCharacter, "Head");
		LocalTorso = RBX::FindFirstChild(LocalCharacter, "Torso");
		if (LocalTorso < 1)
		{
			LocalTorso = RBX::FindFirstChild(LocalCharacter, "UpperTorso");
		}
	}

	bool WorldToScreen(D3DXVECTOR3 pos, D3DXVECTOR2 &screen, D3DXMATRIX matrix)
	{
		unsigned int screenWidth, screenHeight;
		RECT rc;
		HWND newhwnd = FindWindow(NULL, "ROBLOX");
		if (newhwnd != NULL)
		{
			GetWindowRect(newhwnd, &rc);
			screenWidth = rc.right - rc.left;
			screenHeight = rc.bottom - rc.top;
		}
		else
		{
			ExitProcess(0);
		}
		D3DXVECTOR4 clipCoords;
		clipCoords.x = pos.x*matrix._11 + pos.y*matrix._12 + pos.z*matrix._13 + matrix._14;
		clipCoords.y = pos.x*matrix._21 + pos.y*matrix._22 + pos.z*matrix._23 + matrix._24;
		clipCoords.z = pos.x*matrix._31 + pos.y*matrix._32 + pos.z*matrix._33 + matrix._34;
		clipCoords.w = pos.x*matrix._41 + pos.y*matrix._42 + pos.z*matrix._43 + matrix._44;

		if (clipCoords.w < 0.1f)
			return false;

		D3DXVECTOR3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		screen.x = (screenWidth / 2 * NDC.x) + (NDC.x + screenWidth / 2);
		screen.y = -(screenHeight / 2 * NDC.y) + (NDC.y + screenHeight / 2);
		return true;
	}
};
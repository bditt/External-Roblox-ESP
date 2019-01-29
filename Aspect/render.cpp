#include <render.h>
#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <time.h>
#include <cstdlib>
#include <Psapi.h>
#include <TlHelp32.h>
#include <vector>
#include <fstream>
#include <string>
#include <tchar.h>
#include <Shlwapi.h>
#include "RBX.h"

namespace Render
{
	void DrawInfo(IDirect3DDevice9* pDevice)
	{
		DrawStringOutline("External ESP Example by bditt.\nPress INS to show/hide menu", 15, 10, 255, 255, 255, 255, pFontUISmall);
	}

	void DrawPlayer(DWORD Instance)
	{
		try
		{
			Globals::rbx.LocalCharacter = Globals::rbx.GetCharacter(Globals::rbx.LocalPlayer);
			Globals::rbx.LocalHumanoid = Globals::rbx.FindFirstChild(Globals::rbx.LocalCharacter, "Humanoid");
			Globals::rbx.LocalHead = Globals::rbx.FindFirstChild(Globals::rbx.LocalCharacter, "Head");
			Globals::rbx.LocalLeftLeg = Globals::rbx.FindFirstChild(Globals::rbx.LocalCharacter, "Left Leg");
			if (Globals::rbx.LocalLeftLeg < 1)
			{
				Globals::rbx.LocalLeftLeg = Globals::rbx.FindFirstChild(Globals::rbx.LocalCharacter, "LeftLowerLeg");
			}
			DWORD Character = Globals::rbx.GetCharacter(Instance);
			DWORD Head = Globals::rbx.FindFirstChild(Character, "Head");
			DWORD Torso = Globals::rbx.FindFirstChild(Character, "Torso");
			if (Torso < 1)
			{
				Torso = Globals::rbx.FindFirstChild(Character, "UpperTorso");
			}
			DWORD LeftLeg = Globals::rbx.FindFirstChild(Character, "Left Leg");
			if (LeftLeg < 1)
			{
				LeftLeg = Globals::rbx.FindFirstChild(Character, "LeftLowerLeg");
			}
			D3DXVECTOR2 LocalPlayerLeftLegScreenPosition;
			D3DXVECTOR2 screenPos1;
			D3DXVECTOR2 screenPos2;
			D3DXVECTOR2 screenPos3;
			D3DXVECTOR3 LocalPlayerHeadPosition;
			D3DXVECTOR3 LocalPlayerLeftLegPosition;
			D3DXVECTOR3 vpos1;
			D3DXVECTOR3 vpos2;
			D3DXVECTOR3 vpos3;
			LocalPlayerHeadPosition = Globals::rbx.GetPlayerPosition(Globals::rbx.LocalHead);
			LocalPlayerLeftLegPosition = Globals::rbx.GetPlayerPosition(Globals::rbx.LocalLeftLeg);
			vpos1 = Globals::rbx.GetPlayerPosition(Head);
			vpos2 = Globals::rbx.GetPlayerPosition(Torso);
			vpos3 = Globals::rbx.GetPlayerPosition(LeftLeg);
			vpos1.y = vpos1.y + 2.5f;
			vpos3.y = vpos3.y - 3.0f;
			LocalPlayerLeftLegPosition.y = LocalPlayerLeftLegPosition.y - 3.0f;
			if (Globals::rbx.WorldToScreen(vpos1, screenPos1, Globals::rbx.ViewMatrix) && Globals::rbx.WorldToScreen(vpos2, screenPos2, Globals::rbx.ViewMatrix) && Globals::rbx.WorldToScreen(vpos3, screenPos3, Globals::rbx.ViewMatrix))
			{
				if (Globals::rbx.nameesp)
				{
					DrawStringOutline(Globals::rbx.GetName(Instance).c_str(), screenPos3.x - 20, screenPos3.y, 224, 196, 13, 255, pFontVisualsLarge);
				}
				if (Globals::rbx.boxesp)
				{
					RECT rPosition;
					rPosition.left = screenPos2.x - (screenPos1.y - screenPos2.y) / 2.1f;
					rPosition.top = screenPos1.y;
					rPosition.bottom = screenPos3.y;
					rPosition.right = screenPos2.x + (screenPos1.y - screenPos2.y) / 2.1f;
					DrawRectOutlined(rPosition, 22, 163, 43, 255);
				}
			}
		}
		catch (exception e)
		{
			std::cout << "Drawing Error!" << std::endl;
		}
	}

	void DrawVisuals(IDirect3DDevice9* pDevice) {
		try
		{
			RECT rc;
			HWND rbxhwnd = FindWindow(NULL, "ROBLOX");
			if (rbxhwnd != NULL) {
				GetWindowRect(rbxhwnd, &rc);
				Globals::rbx.s_width = rc.right - rc.left;
				Globals::rbx.s_height = rc.bottom - rc.top;
			}
			else {
				ExitProcess(0);
			}

			if (rbxhwnd == GetForegroundWindow() || rbxhwnd == GetActiveWindow() || Globals::hWnd == GetActiveWindow())
			{
				if (Globals::rbx.espenabled)
				{
					std::vector<DWORD> Children = Globals::rbx.GetChildren(Globals::rbx.Players);
					for (DWORD i = 0; i < Children.size(); i++)
					{
						DrawPlayer(Children[i]);
					}
				}
			}
		}
		catch (exception e)
		{
			std::cout << "ESP Error!" << std::endl;
		}
	}

	void DrawMenu(IDirect3DDevice9* pDevice)
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.62f, 0.93f, 0.06f, 1.00f);

		ImGui_ImplDX9_NewFrame();
		ImGui::Begin("External ESP Example By Bditt", NULL, ImVec2(400, 350), 1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::Checkbox("- ESP Enabled", &Globals::rbx.espenabled);
		ImGui::Checkbox("- Names", &Globals::rbx.nameesp);
		ImGui::Checkbox("- Boxes", &Globals::rbx.boxesp);
		ImGui::End();
		ImGui::Render();
	}
}
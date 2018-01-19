#include "sdk.h"
#include "hooks.h"
#include "Menu.h"
#include "global.h"
#include "ImGUI\imconfig.h"
#include "ImGUI\imgui.h"
#include "ImGUI\imgui_internal.h"
#include "ImGUI\stb_rect_pack.h"
#include "ImGUI\stb_textedit.h"
#include "ImGUI\stb_truetype.h"
#include "ImGUI\DX9\imgui_impl_dx9.h"

bool CurKey[256] = {};
WNDPROC                            Hooks::g_pOldWindowProc; //Old WNDPROC pointer

extern IMGUI_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall Hooks::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto& io = ImGui::GetIO();
	switch (uMsg)
	{

	case WM_LBUTTONDOWN:
		CurKey[VK_LBUTTON] = true;
		break;
	case WM_LBUTTONUP:
		CurKey[VK_LBUTTON] = false;
		break;
	case WM_RBUTTONDOWN:
		CurKey[VK_RBUTTON] = true;
		break;
	case WM_RBUTTONUP:
		CurKey[VK_RBUTTON] = false;
		break;
	case WM_KEYDOWN:
		CurKey[wParam] = true;
		break;
	case WM_KEYUP:
		CurKey[wParam] = false;
		break;
	case WM_MOUSEMOVE:
		io.MousePos.x = static_cast<signed short>(lParam);
		io.MousePos.y = static_cast<signed short>(lParam >> 16);
		break;
	default: break;
	}
	{
		static auto isDown = false;
		static auto isClicked = false;
		if (CurKey[Clientvariables->Misc.MenuKey])
		{
			isClicked = false;
			isDown = true;
		}
		else if (!CurKey[Clientvariables->Misc.MenuKey] && isDown)
		{
			isClicked = true;
			isDown = false;
		}
		else
		{
			isClicked = false;
			isDown = false;
		}
		if (isClicked)
		{
			G::Opened = !G::Opened;

			static auto cl_mouseenable = g_pCvar->FindVar(("cl_mouseenable"));

			cl_mouseenable->SetValue(!G::Opened);

		}

	}
	if (G::Init && G::Opened && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(Hooks::g_pOldWindowProc, hWnd, uMsg, wParam, lParam);
}
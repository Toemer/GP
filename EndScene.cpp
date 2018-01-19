#include "sdk.h"
#include "hooks.h"
#include "Menu.h"
#include "global.h"
#include "Hitmarker.h"
#include "ESP.h"
#include "ImGUI\imconfig.h"
#include "ImGUI\imgui.h"
#include "ImGUI\imgui_internal.h"
#include "ImGUI\stb_rect_pack.h"
#include "ImGUI\stb_textedit.h"
#include "ImGUI\stb_truetype.h"
#include "ImGUI\DX9\imgui_impl_dx9.h"
#include "Items.h"
#include "Config.h"
#include "GameUtils.h"

typedef void(*CL_FullUpdate_t) (void);
CL_FullUpdate_t CL_FullUpdate = nullptr;

void DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float &flRainbow)
{
	ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

	Color colColor(255, 255, 255, 255);

	flRainbow += flSpeed;
	if (flRainbow > 1.f) flRainbow = 0.f;

	for (int i = 0; i < width; i++)
	{
		float hue = (1.f / (float)width) * i;
		hue -= flRainbow;
		if (hue < 0.f) hue += 1.f;

		Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
		windowDrawList->AddRectFilled(ImVec2(x + i, y), ImVec2(width, height), colRainbow.GetU32());
	}
}

void KnifeApplyCallbk()
{
	static auto CL_FullUpdate = reinterpret_cast<CL_FullUpdate_t>(FindPattern("engine.dll", reinterpret_cast<PBYTE>("\xA1\x00\x00\x00\x00\xB9\x00\x00\x00\x00\x56\xFF\x50\x14\x8B\x34\x85"), "x????x????xxxxxxx"));
	CL_FullUpdate();
}

PresentFn oPresent;

ImFont* Impact;
ImFont* Default;
ImFont* Tabs;

tReset oResetScene;

void GUI_Init(IDirect3DDevice9* pDevice) // Setup for imgui
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplDX9_Init(G::Window, pDevice);

	ImGuiStyle& style = ImGui::GetStyle();

	Impact = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Impact.ttf", 32.0f);
	Default = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\micross.ttf", 14.0f);
	Tabs = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Skeet-tabs.ttf", 60.0f);

	ImGuiStyle& styled = ImGui::GetStyle();

	style.Alpha = .0f;
	style.WindowPadding = ImVec2(14, 8);
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowRounding = 0.5f; //4.0
	style.ChildWindowRounding = 0.0f;
	style.FramePadding = ImVec2(4, 1);
	style.FrameRounding = 0.0f; //2.0
	style.ItemSpacing = ImVec2(0, 0);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 3.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 16.0f; //16.0
	style.GrabMinSize = 0.1f;
	style.GrabRounding = 16.0f; //16.0
	style.ButtonTextAlign = ImVec2(0.f, - 3.f);
	style.DisplayWindowPadding = ImVec2(22, 22);
	style.DisplaySafeAreaPadding = ImVec2(4, 4);
	style.AntiAliasedLines = true;
	style.AntiAliasedShapes = true;
	style.CurveTessellationTol = 1.25f;

	G::Init = true;
}

void color()
{
	int MenuRed, MenuGreen, MenuBlue;

	MenuRed = Clientvariables->Colors.MenuColor[0] * 255;
	MenuGreen = Clientvariables->Colors.MenuColor[1] * 255;
	MenuBlue = Clientvariables->Colors.MenuColor[2] * 255;

	ImColor mainColor = ImColor(MenuRed, MenuGreen, MenuBlue, 255);
	ImColor bodyColor = ImColor(24, 24, 24, 239);
	ImColor fontColor = ImColor(255, 255, 255, 255);

	ImGuiStyle& styled = ImGui::GetStyle();

	ImVec4 mainColorHovered = ImVec4(mainColor.Value.x + 0.1f, mainColor.Value.y + 0.1f, mainColor.Value.z + 0.1f, mainColor.Value.w);
	ImVec4 mainColorActive = ImVec4(mainColor.Value.x + 0.2f, mainColor.Value.y + 0.2f, mainColor.Value.z + 0.2f, mainColor.Value.w);
	ImVec4 menubarColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w - 0.8f);
	ImVec4 frameBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .1f);
	ImVec4 tooltipBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .05f);

	styled.Colors[ImGuiCol_Text] = fontColor;
	styled.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	styled.Colors[ImGuiCol_WindowBg] = bodyColor;
	styled.Colors[ImGuiCol_ChildWindowBg] = ImColor(20, 20, 20, 255);
	styled.Colors[ImGuiCol_PopupBg] = tooltipBgColor;
	styled.Colors[ImGuiCol_Border] = ImColor(1, 1, 1, 255);
	styled.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	styled.Colors[ImGuiCol_FrameBg] = frameBgColor;
	styled.Colors[ImGuiCol_FrameBgHovered] = mainColorHovered;
	styled.Colors[ImGuiCol_FrameBgActive] = mainColorActive;
	styled.Colors[ImGuiCol_FrameOut] = ImColor(65, 64, 65, 255);
	styled.Colors[ImGuiCol_Frameout2] = ImColor(15, 15, 15, 255);
	styled.Colors[ImGuiCol_TitleBg] = mainColor;
	styled.Colors[ImGuiCol_TitleBgCollapsed] = mainColor;
	styled.Colors[ImGuiCol_TitleBgActive] = mainColor;
	styled.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	styled.Colors[ImGuiCol_ScrollbarBg] = ImVec4(frameBgColor.x + .1f, frameBgColor.y + .1f, frameBgColor.z + .1f, frameBgColor.w);
	styled.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.40f, 1.f);
	styled.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.f);
	styled.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.f);
	styled.Colors[ImGuiCol_ComboBg] = frameBgColor;
	styled.Colors[ImGuiCol_CheckMark] = mainColor;
	styled.Colors[ImGuiCol_SliderGrab] = mainColor;
	styled.Colors[ImGuiCol_SliderGrabActive] = mainColorHovered;
	styled.Colors[ImGuiCol_Button] = mainColor;
	styled.Colors[ImGuiCol_Buttontemp1] = ImColor(0, 0, 0, 0);
	styled.Colors[ImGuiCol_Buttontemp2] = ImColor(45, 44, 45, 255);
	styled.Colors[ImGuiCol_ButtonHovered] = mainColorHovered;
	styled.Colors[ImGuiCol_ButtonActive] = mainColorActive;
	styled.Colors[ImGuiCol_Header] = mainColor;
	styled.Colors[ImGuiCol_HeaderHovered] = mainColorHovered;
	styled.Colors[ImGuiCol_HeaderActive] = mainColorActive;

	styled.Colors[ImGuiCol_Column] = ImVec4(.30f, .30f, .30f, 1.f);
	styled.Colors[ImGuiCol_ColumnHovered] = mainColorHovered;
	styled.Colors[ImGuiCol_ColumnActive] = mainColorActive;

	styled.Colors[ImGuiCol_ResizeGrip] = mainColor;
	styled.Colors[ImGuiCol_ResizeGripHovered] = mainColorHovered;
	styled.Colors[ImGuiCol_ResizeGripActive] = mainColorActive;
	styled.Colors[ImGuiCol_CloseButton] = ImVec4(0.f, 0.f, 0.f, 0.f);
	styled.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.f, 0.f, 0.f, 0.f);
	styled.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.f, 0.f, 0.f, 0.f);
	styled.Colors[ImGuiCol_PlotLines] = mainColor;
	styled.Colors[ImGuiCol_PlotLinesHovered] = mainColorHovered;
	styled.Colors[ImGuiCol_PlotHistogram] = mainColor;
	styled.Colors[ImGuiCol_PlotHistogramHovered] = mainColorHovered;
	styled.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	styled.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

void TextColor(bool active)
{
	auto& style = ImGui::GetStyle();

	if (active)
		style.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.f);
	else
		style.Colors[ImGuiCol_Text] = ImVec4(0.39f, 0.39f, 0.39f, 1.f);
}


void buttonColor(bool bActive)
{
	ImGuiStyle& style = ImGui::GetStyle();

	if (bActive)
	{
		style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_WindowBg];
		style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_WindowBg];
		style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_WindowBg];
	}
	else
	{
		style.Colors[ImGuiCol_Button] = ImColor(10, 10, 10);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(10, 10, 10);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(10, 10, 10);
	}
}

void lineColor(bool bActive)
{
	ImGuiStyle& style = ImGui::GetStyle();

	if (bActive)
	{
		style.Colors[ImGuiCol_Button] = ImColor(120, 120, 120);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(120, 120, 120);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(120, 120, 120);
	}
	else
	{
		style.Colors[ImGuiCol_Button] = ImColor(10, 10, 10);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(10, 10, 10);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(10, 10, 10);
	}
}

void lineColor1(bool bActive)
{
	ImGuiStyle& style = ImGui::GetStyle();

	if (bActive)
	{
		style.Colors[ImGuiCol_Button] = ImColor(120, 120, 120);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(120, 120, 120);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(120, 120, 120);
	}
	else
	{
		style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_WindowBg];
		style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_WindowBg];
		style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_WindowBg];
	}
}


void Border(bool active)
{
	auto& style = ImGui::GetStyle();

	if (active)
		style.Colors[ImGuiCol_Border] = ImColor(80, 80, 80, 255);
	else
		style.Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
}

HRESULT __stdcall Hooks::D3D9_EndScene(IDirect3DDevice9* pDevice)
{
	HRESULT result = d3d9VMT->GetOriginalMethod<EndSceneFn>(42)(pDevice);

	if (!G::Init)
	{
		GUI_Init(pDevice);
	}
	else
	{
		if (g_pEngine->IsInGame() && g_pEngine->IsConnected())
			ImGui::GetIO().MouseDrawCursor = G::Opened;
		else
			ImGui::GetIO().MouseDrawCursor = true;
		ImGui_ImplDX9_NewFrame();

		auto& style = ImGui::GetStyle();

		if (G::Opened && style.Alpha < 1.f)
		{
			G::ShowMenu = true;
			if (style.Alpha > 1.f)
				style.Alpha = 1.f;
			else if (style.Alpha != 1.f)
				style.Alpha += 0.03f;
		}
		else if (style.Alpha > 0.f)
		{
			if (style.Alpha < 0.f)
				style.Alpha = 0.f;
			else if (style.Alpha != 0.f)
				style.Alpha -= 0.03f;
			if (style.Alpha == 0.f)
				G::ShowMenu = false;
		}

		static int tab;

		if (G::ShowMenu)
		{
			ImVec2 mainWindowPos; // wir brauchen die main window pos um nen vernünftigen border zu drawen

		
		
			

			ImGui::PushFont(Default);
			color();
			ImGui::SetNextWindowSize(ImVec2(649, 539));
			ImGui::Begin("Gamephetamine", &G::ShowMenu, ImVec2(550, 660), 0.98f, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar);
			{
				mainWindowPos = ImGui::GetWindowPos();

			


	

				static float flRainbow;
				float flSpeed = 0.0000f;

				int curWidth = 1;

				ImVec2 curPos = ImGui::GetCursorPos();
				ImVec2 curWindowPos = ImGui::GetWindowPos();
				curPos.x += curWindowPos.x;
				curPos.y += curWindowPos.y;

				int size;
				int y;
				g_pEngine->GetScreenSize(size, y);
				DrawRectRainbow(curPos.x, curPos.y * -1, ImGui::GetWindowSize().x + size, curPos.y * 1.0, flSpeed, flRainbow);
				ImGui::Columns(2, nullptr, false);
				ImGui::SetColumnOffset(1, 100);
				style.Colors[ImGuiCol_Border] = ImColor(80, 80, 80, 0);
				style.ItemSpacing = ImVec2(0, 0);

				/*push your tab font here*/
				//tabs
				
				ImGui::PushFont(Tabs);
				TextColor(tab == 0);
				/*//////////////////////////*/
				buttonColor(false);
				ImGui::Button("##upprspacing", ImVec2(74, 20));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##uppr_t", ImVec2(1, 20));

				lineColor(tab == 0);
				ImGui::Button("##rage_t", ImVec2(74, 1));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##rage_d", ImVec2(1, 1));

				buttonColor(tab == 0);
				if (ImGui::Button("C", ImVec2(74, 76))) tab = 0;
				TextColor(tab == 1);
				ImGui::SameLine();
				lineColor1(tab != 0);
				ImGui::Button("rage_a", ImVec2(1, 76));

				//////////////////////////////////////////////////////

				lineColor(tab == 0 || tab == 1);
				ImGui::Button("##legit_t", ImVec2(74, 1));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##legit_d", ImVec2(1, 1));

				buttonColor(tab == 1);
				if (ImGui::Button("D", ImVec2(74, 76))) tab = 1;
				TextColor(tab == 2);
				ImGui::SameLine();
				lineColor1(tab != 1);
				ImGui::Button("legit_a", ImVec2(1, 76));

				//////////////////////////////////////////////////////

				lineColor(tab == 1 || tab == 2);
				ImGui::Button("##vis_t", ImVec2(74, 1));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##vis_d", ImVec2(1, 1));

				buttonColor(tab == 2);
				if (ImGui::Button("E", ImVec2(74, 76))) tab = 2;
				TextColor(tab == 3);
				ImGui::SameLine();
				lineColor1(tab != 2);
				ImGui::Button("vis_a", ImVec2(1, 76));

				//////////////////////////////////////////////////////

				lineColor(tab == 2 || tab == 3);
				ImGui::Button("##vis_t", ImVec2(74, 1));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##vis_d", ImVec2(1, 1));

				buttonColor(tab == 3);
				if (ImGui::Button("F", ImVec2(74, 76))) tab = 3;
				TextColor(tab == 4);
				ImGui::SameLine();
				lineColor1(tab != 3);
				ImGui::Button("misc_a", ImVec2(1, 76));

				//////////////////////////////////////////////////////

				lineColor(tab == 3 || tab == 4);
				ImGui::Button("##skin_t", ImVec2(74, 1));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##skin_d", ImVec2(1, 1));

				buttonColor(tab == 4);
				if (ImGui::Button("G", ImVec2(74, 76))) tab = 4;
				TextColor(tab == 5);
				ImGui::SameLine();
				lineColor1(tab != 4);
				ImGui::Button("skin_a", ImVec2(1, 76));

				//////////////////////////////////////////////////////

				lineColor(tab == 4 || tab == 5);
				ImGui::Button("##clr_t", ImVec2(74, 1));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##clr_d", ImVec2(1, 1));

				buttonColor(tab == 5);
				if (ImGui::Button("H", ImVec2(74, 76))) tab = 5;
				ImGui::SameLine();
				lineColor1(tab != 5);
				ImGui::Button("clr_a", ImVec2(1, 76));

				lineColor(tab == 5);
				ImGui::Button("down_t", ImVec2(74, 1));
				ImGui::SameLine();
				lineColor(1);
				ImGui::Button("down_b", ImVec2(1, 1));

				buttonColor(false);
				ImGui::Button("##downspacing", ImVec2(74, 49));
				ImGui::SameLine();
				lineColor(true);
				ImGui::Button("##down_t", ImVec2(1, 49));

				//////////////////////////////////////////////////////


				ImGui::PushFont(Default);
				/*//////////////////////////*/
			
				ImGui::NextColumn();
				TextColor(true);
				Border(true);
				color();


				
				style.ItemSpacing = ImVec2(8, 6);
				style.Colors[ImGuiCol_Border] = ImColor(80, 80, 80, 0);
				style.Colors[ImGuiCol_ChildWindowBg] = ImColor(18, 18, 18, 0);
				style.WindowPadding = ImVec2(8, 8);
				ImGui::BeginChild("masterchild", ImVec2(534, 530), true); // Master Child
				{

					

					style.Colors[ImGuiCol_Border] = ImColor(80, 80, 80, 255);
					if (tab == 0) // Ragebot
					{
						style.Colors[ImGuiCol_ChildWindowBg] = ImColor(18, 18, 18, 180);
						ImGui::Columns(2, "yes", false);

						ImGui::Text("Aimbot"); // Title

						ImGui::BeginChild("Aimbot", ImVec2(246, 294), true);
						{
							ImGui::Checkbox("   Enabled", &Clientvariables->Ragebot.EnableAimbot);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Target selection");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##selection", &Clientvariables->Ragebot.AimbotSelection, SelectionMode, ARRAYSIZE(SelectionMode));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Target hitbox");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##hitbox", &Clientvariables->Ragebot.Hitbox, HitboxMode, ARRAYSIZE(HitboxMode));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Target multibox");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##multibox", &Clientvariables->Ragebot.Hitscan, HitscanMode, ARRAYSIZE(HitscanMode));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Head scale");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderFloat("##headscale", &Clientvariables->Ragebot.Headscale, 0.f, 1.f, "%.2f%%");

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Body scale");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderFloat("##bodyscale", &Clientvariables->Ragebot.Bodyscale, 0.f, 1.f, "%.2f%%");

							ImGui::Checkbox("   Automatic fire", &Clientvariables->Ragebot.AutomaticFire);
							ImGui::Checkbox("   Automatic penetration", &Clientvariables->Ragebot.Autowall);
							ImGui::Checkbox("   Hitscan autowall", &Clientvariables->Ragebot.AutowallHitscan);
							ImGui::Checkbox("   Automatic scope", &Clientvariables->Ragebot.AutomaticScope);
							ImGui::Checkbox("   Silent aim", &Clientvariables->Ragebot.SilentAimbot);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Hitchance");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##hitchance", &Clientvariables->Ragebot.Minhitchance, 0, 100, "%.0f%%");

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Minimum damage");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderMessage("##mindamage", &Clientvariables->Ragebot.Mindamage, 1, 101, 101 /*Value at custom msg*/, "Enemy HP");

						} ImGui::EndChild();

						ImGui::Text("Fake lag"); // Title

						ImGui::BeginChild("##fakelag", ImVec2(246, 180), true);
						{
							ImGui::Checkbox("   Enabled", &Clientvariables->Misc.FakelagEnable);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##mode", &Clientvariables->Misc.FakelagMode, FakelagMode, ARRAYSIZE(FakelagMode));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Limit");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##choke", &Clientvariables->Misc.FakelagAmount, 1, 14);

							ImGui::Checkbox("   Fake lag while shooting", &Clientvariables->Misc.FakelagShoot);
							ImGui::Checkbox("   Fakelag on ground", &Clientvariables->Misc.FakelagOnground);


						} ImGui::EndChild();

						ImGui::NextColumn();
						ImGui::Text("Other"); // Title

						ImGui::BeginChild("##other", ImVec2(246, 199), true);
						{
							style.Colors[ImGuiCol_Text] = ImColor(180, 180, 100, 255);
							ImGui::Checkbox("   Remove spread", &Clientvariables->Ragebot.NoSpread);
							style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
							ImGui::Checkbox("   Remove recoil", &Clientvariables->Ragebot.NoRecoil);

							ImGui::Checkbox("   Accuracy boost", &Clientvariables->Ragebot.PositionAdjustment);
							ImGui::Checkbox("   Anti-aim resolver", &Clientvariables->Ragebot.AutomaticResolver);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Step angle");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##stepangle", &Clientvariables->Ragebot.ResolverStepAngle, 25, 90);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Prefer body aim");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##preferbodyaim", &Clientvariables->Ragebot.Preferbodyaim, PreferBodyaim, ARRAYSIZE(PreferBodyaim));

						}ImGui::EndChild();

						ImGui::Text("Anti-aimbot");

						ImGui::BeginChild("##antiaimbot", ImVec2(246, 275), true);
						{
							ImGui::Checkbox("   Enabled", &Clientvariables->Antiaim.AntiaimEnable);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Pitch");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##pitch", &Clientvariables->Antiaim.Pitch, AntiaimbotPitch, ARRAYSIZE(AntiaimbotPitch));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Yaw");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##yaw", &Clientvariables->Antiaim.Yaw, AntiaimbotYaw, ARRAYSIZE(AntiaimbotYaw));
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##yawadd", &Clientvariables->Antiaim.YawAdd, -180, 180);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Yaw while running");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##yawrunning", &Clientvariables->Antiaim.YawRunning, AntiaimbotYawRun, ARRAYSIZE(AntiaimbotYawRun));
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##yawrunadd", &Clientvariables->Antiaim.YawRunningAdd, -180, 180);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Fake yaw");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##fakeyaw", &Clientvariables->Antiaim.FakeYaw, AntiaimbotYawFake, ARRAYSIZE(AntiaimbotYawFake));
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##yawfakeadd", &Clientvariables->Antiaim.FakeYawAdd, -180, 180);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Freestanding");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##freestanding", &Clientvariables->Antiaim.Freestanding, Freestanding, ARRAYSIZE(Freestanding));
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##freestand", &Clientvariables->Antiaim.FreestandingDelta, -180, 180);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Dynamic");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##dynamic", &Clientvariables->Antiaim.AtPlayer, AtTarget, ARRAYSIZE(AtTarget));

						} ImGui::EndChild();
					}
					else if (tab == 1)
					{
						ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Legitbot is in progress!");
					}
					else if (tab == 2)
					{
						style.Colors[ImGuiCol_ChildWindowBg] = ImColor(18, 18, 18, 180);
						ImGui::Columns(2, "yes", false);

						ImGui::Text("Player ESP");
						ImGui::BeginChild("##player", ImVec2(246, 322), true);
						{
							ImGui::Checkbox("   Enabled", &Clientvariables->Visuals.EspEnable);

							ImGui::Checkbox("   Enemy only", &Clientvariables->Visuals.EnemyOnly);
							ImGui::Checkbox("   Bounding box", &Clientvariables->Visuals.BoundingBox);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##box", Clientvariables->Colors.BoundingBox, 1 << 7);
							ImGui::Checkbox("   Health bar", &Clientvariables->Visuals.Health);
							ImGui::Checkbox("   Name", &Clientvariables->Visuals.Name);
							ImGui::Checkbox("   Armor", &Clientvariables->Visuals.Armor);
							ImGui::Checkbox("   Flags", &Clientvariables->Visuals.Flags);
							ImGui::Checkbox("   Fake", &Clientvariables->Visuals.Fake);
							ImGui::Checkbox("   Snap lines", &Clientvariables->Visuals.SnapLines);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##snapline_color", Clientvariables->Colors.Snaplines, 1 << 7);

							ImGui::Checkbox("   Weapon", &Clientvariables->Visuals.Weapon);
							ImGui::Checkbox("   Ammo", &Clientvariables->Visuals.Ammo);
							ImGui::Checkbox("   Skeleton", &Clientvariables->Visuals.Bones);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##bones", Clientvariables->Colors.Skeletons, 1 << 7);
							ImGui::Checkbox("   Glow", &Clientvariables->Visuals.Glow);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##glow", Clientvariables->Colors.Glow, 1 << 7);
							//	ImGui::Checkbox("   World glow", &Clientvariables->Visuals.Dlight);
							ImGui::Checkbox("   Hitmarker", &Clientvariables->Visuals.Hitmarker);
							//	ImGui::Checkbox("   Line of sight", &Clientvariables->Visuals.LineofSight);


						} ImGui::EndChild();

						ImGui::Text("Colored models");
						ImGui::BeginChild("##models", ImVec2(246, 147), true);
						{
							ImGui::Checkbox("   Enabled", &Clientvariables->Visuals.ChamsEnable);
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Type");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##texture", &Clientvariables->Visuals.ChamsStyle, ModelsMode, ARRAYSIZE(ModelsMode));

							ImGui::Checkbox("   Player", &Clientvariables->Visuals.ChamsPlayer);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##chams", Clientvariables->Colors.PlayerChams, 1 << 7);
							ImGui::Checkbox("   Player (behind wall)", &Clientvariables->Visuals.ChamsPlayerWall);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##chamswall", Clientvariables->Colors.PlayerChamsWall, 1 << 7);

							ImGui::Checkbox("   Hand chams", &Clientvariables->Visuals.ChamsHands);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##handchams", Clientvariables->Colors.ChamsHand, 1 << 7);

							ImGui::Checkbox("   Hand wireframe", &Clientvariables->Visuals.ChamsHandsWireframe);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##handwireframe", Clientvariables->Colors.WireframeHand, 1 << 7);

							ImGui::Checkbox("   Weapon chams", &Clientvariables->Visuals.WeaponChams);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##weaponchams", Clientvariables->Colors.ChamsWeapon, 1 << 7);

							ImGui::Checkbox("   Weapon wireframe", &Clientvariables->Visuals.WeaponWireframe);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##weaponwireframe", Clientvariables->Colors.WireframeWeapon, 1 << 7);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Fake angle ghost");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##ghost", &Clientvariables->Visuals.FakeAngleGhost, Fakeghost, ARRAYSIZE(Fakeghost));
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##ghostcolor", Clientvariables->Colors.FakeAngleGhost, 1 << 7);

						}ImGui::EndChild();

						ImGui::NextColumn();
						ImGui::Text("Other ESP");
						ImGui::BeginChild("##other", ImVec2(246, 197), true);
						{
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Dropped weapons");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##droppedweapon", &Clientvariables->Visuals.DroppedWeapons, DroppedWeapons, ARRAYSIZE(DroppedWeapons));
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##weaponcolor", Clientvariables->Colors.DroppedWeapon, 1 << 7);

							ImGui::Checkbox("   Recoil crosshair", &Clientvariables->Visuals.RecoilCrosshair);
							ImGui::Checkbox("   Spread crosshair", &Clientvariables->Visuals.SpreadCrosshair);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##spreadcolor", Clientvariables->Colors.SpreadCrosshair, 1 << 7);
							ImGui::Checkbox("   Bomb", &Clientvariables->Visuals.Bomb);
							ImGui::Checkbox("   Grenade trajectory", &Clientvariables->Visuals.GrenadePrediction);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##nadetrajectory", Clientvariables->Colors.GrenadePrediction, 1 << 7);
							ImGui::Checkbox("   Grenade Projectile", &Clientvariables->Visuals.ThrownNades);
							ImGui::Checkbox("   Bullet Tracers", &Clientvariables->Visuals.BulletTracers);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##tracers", Clientvariables->Colors.Bulletracer, 1 << 7);
							ImGui::Checkbox("   Damage Indicator", &Clientvariables->Visuals.DamageIndicators);
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##damageindicator", Clientvariables->Colors.DamageIndicator, 1 << 7);

						} ImGui::EndChild();

						ImGui::Text("Effects");
						ImGui::BeginChild("##effects", ImVec2(246, 275), true);
						{
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Flashbang alpha");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##flash_alpha", &Clientvariables->Visuals.FlashbangAlpha, 0, 255, "%.0f%%");
							ImGui::Checkbox("   Remove smoke grenades", &Clientvariables->Visuals.Nosmoke);
							ImGui::Checkbox("   Remove visual recoil", &Clientvariables->Visuals.Novisrevoil);
							ImGui::Checkbox("   Night mode", &Clientvariables->Visuals.nightmode);
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Skybox");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##Skybox", &Clientvariables->Visuals.Skybox, Skyboxmode, ARRAYSIZE(Skyboxmode));
							ImGui::Checkbox("   Remove scope", &Clientvariables->Visuals.Noscope);
							ImGui::Checkbox("   Disable post processing", &Clientvariables->Visuals.RemoveParticles);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Force third person");
							ImGui::NewLine();
							ImGui::SameLine(35);
							ImGui::Hotkey("##thirdperson", &Clientvariables->Misc.TPKey, ImVec2(164, 20));
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Combo("##angles", &Clientvariables->Misc.TPangles, ThirdpersonAngles, ARRAYSIZE(ThirdpersonAngles));

						} ImGui::EndChild();
					}
					else if (tab == 3)
					{
						style.Colors[ImGuiCol_ChildWindowBg] = ImColor(18, 18, 18, 180);
						ImGui::Columns(2, "yes", false);
						ImGui::Text("Miscellaneous");
						ImGui::BeginChild("##misc", ImVec2(246, 500), true);
						{
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Override FOV");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##fov", &Clientvariables->Misc.PlayerFOV, 0, 50);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Override viewmodel");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##viewmodel", &Clientvariables->Misc.PlayerViewmodel, 0, 50); // idk max maybe 50

							ImGui::Checkbox("   Bunny hop", &Clientvariables->Misc.AutoJump);
							ImGui::Checkbox("   Air strafe", &Clientvariables->Misc.AutoStrafe);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Pre-speed");
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Hotkey("##speedkey", &Clientvariables->Misc.PrespeedKey, ImVec2(150, 20));
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::SliderInt("##retrack", &Clientvariables->Misc.Retrack, 1, 8);

							//ImGui::Checkbox("   Auto-accept matchmaking", &Clientvariables->Misc.AutoStrafe);
							//ImGui::Checkbox("   Clan tag", &Clientvariables->Misc.AutoStrafe);
						} ImGui::EndChild();

						ImGui::NextColumn();
						ImGui::Text("Settings");
						ImGui::BeginChild("##settings", ImVec2(246, 111), true);
						{
							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Menu key");
							ImGui::NewLine();
							ImGui::SameLine(35);
							ImGui::Hotkey("##menukey", &Clientvariables->Misc.MenuKey, ImVec2(150, 20));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Menu color");
							ImGui::SameLine(216);
							ImGui::MyColorEdit3("##menucolor", Clientvariables->Colors.MenuColor, 1 << 7);


							ImGui::Checkbox("   Anti-untrusted", &Clientvariables->Misc.AntiUT);

						} ImGui::EndChild();

						ImGui::Text("Presets");
						ImGui::BeginChild("##presets", ImVec2(246, 155), true);
						{
							style.Colors[ImGuiCol_Button] = ImColor(26, 28, 33, 180);

							ImGui::SameLine(40);
							ImGui::Combo("##cfg", &Clientvariables->Misc.ConfigSelection, Configs, ARRAYSIZE(Configs));
							ImGui::NewLine();
							ImGui::SameLine(40);
							if (ImGui::Button("                  Save", ImVec2(156, 27)))
								ConSys->SaveConfig();
							ImGui::NewLine();
							ImGui::SameLine(40);
							if (ImGui::Button("                  Load", ImVec2(156, 27)))
								ConSys->LoadConfig();
							ImGui::NewLine();
							ImGui::SameLine(40);
							if (ImGui::Button("                  Reset", ImVec2(156, 27)))
								ConSys->Reset();

						} ImGui::EndChild();

						ImGui::Text("Other");
						ImGui::BeginChild("##other", ImVec2(246, 190), true);
						{
							ImGui::Checkbox("   Enable walkbot", &Clientvariables->Misc.Walkbot);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Walkbot Start");
							ImGui::NewLine();
							ImGui::SameLine(35);
							ImGui::Hotkey("##startwalk", &Clientvariables->Misc.WalkbotStart, ImVec2(150, 20));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Set points");
							ImGui::NewLine();
							ImGui::SameLine(35);
							ImGui::Hotkey("##setpoints", &Clientvariables->Misc.WalkbotSet, ImVec2(150, 20));

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Delete points");
							ImGui::NewLine();
							ImGui::SameLine(35);
							ImGui::Hotkey("##deletepoints", &Clientvariables->Misc.WalkbotDelete, ImVec2(150, 20));

							ImGui::Checkbox("  Fakewalk enable", &Clientvariables->Misc.FakewalkEnable);

							ImGui::NewLine();
							ImGui::SameLine(40);
							ImGui::Text("Fakewalk key");
							ImGui::NewLine();
							ImGui::SameLine(35);
							ImGui::Hotkey("##walkkey", &Clientvariables->Misc.FakewalkKey, ImVec2(150, 20));


						} ImGui::EndChild();
					}
					else if (tab == 4)
					{
						/*
						if (g_pEngine->IsInGame() && g_pEngine->IsConnected() && G::LocalPlayer && G::LocalPlayer->isAlive())
						{
							CBaseCombatWeapon* pWeapon = G::LocalPlayer->GetWeapon();

							if (pWeapon)
							{

								ImGui::Checkbox("   Enable skinchanger", &Clientvariables->Skinchanger.Enabled);

								ImGui::Columns(2, "yes", false);

								if (pWeapon->GetItemDefinitionIndex() == WEAPON_GLOCK)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : Glock-18");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::PopItemWidth();
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.GlockSkin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_P250)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : P250");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.P250Skin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_DEAGLE)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : Desert Eagle");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.DeagleSkin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_TEC9)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : Tec9");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.tec9Skin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_CZ75)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : CZ-Auto");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.CZ75Skin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : R8 Revolver");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.RevolverSkin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_P2000)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : P2000");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.P2000Skin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_USPS)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : USP-S");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.USPSkin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_DUALS)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : USP-S");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.DualSkins, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_FIVE7)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : USP-S");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.FiveSevenSkin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 26u);
								}
								else if (pWeapon->GetItemDefinitionIndex() == WEAPON_AK47)
								{
									ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Active Weapon : AK47");

									if (ImGui::Button(("Force Update")))
										KnifeApplyCallbk();

									ImGui::NextColumn();
									ImGui::Text("Skin list");
									ImGui::ListBox("##skins", &Clientvariables->Skinchanger.AK47Skin, Enumerated_Skin_Names, ARRAYSIZE(Enumerated_Skin_Names), 30u);
								}
							}
							else
							{
								ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "You must be alive to change or modify skins.");

							}
							
						}*/
					}

					else if (tab == 5) {
						//for (UINT Iterator = 0; Iterator <= g_pEngine->GetMaxClients(); Iterator++) {
						//	CBaseEntity* pEntity = (CBaseEntity *)(g_pEntitylist->GetClientEntity(Iterator));
						//	if (!pEntity) continue;

						//	player_info_t *pInfo;
						//	g_pEngine->GetPlayerInfo(Iterator, pInfo);

						//	if (!pInfo) continue;

						//	ImGui::CollapsingHeader("", 0, 0, 0);
						//}
					}
				} ImGui::EndChild();
			} ImGui::End();

			style.WindowPadding = ImVec2(2, 4);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
		
			ImGui::SetNextWindowPos(ImVec2(mainWindowPos.x - 5, mainWindowPos.y - 5));
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.f);
			ImGui::SetNextWindowSize(ImVec2(659, 549));
			ImGui::Begin("##border2", &G::ShowMenu, ImVec2(659, 549), 0.98f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_ShowBorders);
			{
				mainWindowPos = ImGui::GetWindowPos();
			} ImGui::End();

		}
		ImGui::Render();
	}
	return result;
}

HRESULT __stdcall Hooks::hkdReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresParam)
{
	if (!G::Init)
		return oResetScene(pDevice, pPresParam);

	ImGui_ImplDX9_InvalidateDeviceObjects();
	auto newr = oResetScene(pDevice, pPresParam);
	ImGui_ImplDX9_CreateDeviceObjects();

	return newr;
}

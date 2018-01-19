#include "hooks.h"
#include "global.h"
#include "Menu.h"
#include "BacktrackingHelper.h"
#include "xor.h"
#include <intrin.h>
#include "SpoofedConvar.h"
#include "Math.h"
#include "Skinchanger.h"
#include "Items.h"
#define INVALID_EHANDLE_INDEX 0xFFFFFFFF

ConVar* mp_facefronttime;
ConVar* r_DrawSpecificStaticProp;



int get_model_index(int item_def_index)
{
	int ret = 0;
	switch (item_def_index)
	{
	case KNIFE:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
		break;
	case KNIFE_T:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");
		break;
	case KNIFE_KARAMBIT:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
		break;
	case KNIFE_BAYONET:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
		break;
	case KNIFE_M9_BAYONET:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
		break;
	case KNIFE_TACTICAL:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
		break;
	case KNIFE_GUT:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
		break;
	case KNIFE_FALCHION:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
		break;
	case KNIFE_PUSH:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
		break;
	case KNIFE_BUTTERFLY:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
		break;
	case KNIFE_FLIP:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
		break;
	case KNIFE_BOWIE:
		ret = g_pModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
		break;
	default:
		break;
	}
	return ret;
}

struct ResolverData
{
	float simtime, flcycle[13], flprevcycle[13], flweight[13], flweightdatarate[13], fakewalkdetection[2], fakeanglesimtimedetection[2], fakewalkdetectionsimtime[2];
	float yaw, addyaw, lbycurtime;
	float shotsimtime, oldlby, lastmovinglby, balanceadjustsimtime, balanceadjustflcycle;
	int fakeanglesimtickdetectionaverage[4], amountgreaterthan2, amountequal1or2, amountequal0or1, amountequal1, amountequal0, resetmovetick, resetmovetick2;
	int tick, balanceadjusttick, missedshots, activity[13];
	bool bfakeangle, bfakewalk, playerhurtcalled, weaponfirecalled;
	Vector shotaimangles, hitboxPos, balanceadjustaimangles;
	uint32_t norder[13];
	char* resolvermode = "NONE", *fakewalk = "Not Moving";
} pResolverData[64];;

bool isPartOf(char *a, char *b) {
	if (std::strstr(b, a) != NULL) {    //Strstr says does b contain a
		return true;
	}
	return false;
}


std::vector<const char*> smoke_materials = {
	"particle/beam_smoke_01",
	"particle/particle_smokegrenade",
	"particle/particle_smokegrenade1",
	"particle/particle_smokegrenade2",
	"particle/particle_smokegrenade3",
	"particle/particle_smokegrenade_sc",
	"particle/smoke1/smoke1",
	"particle/smoke1/smoke1_ash",
	"particle/smoke1/smoke1_nearcull",
	"particle/smoke1/smoke1_nearcull2",
	"particle/smoke1/smoke1_snow",
	"particle/smokesprites_0001",
	"particle/smokestack",
	"particle/vistasmokev1/vistasmokev1",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_nearcull",
	"particle/vistasmokev1/vistasmokev1_nearcull_fog",
	"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev4_emods_nocull",
	"particle/vistasmokev1/vistasmokev4_nearcull",
	"particle/vistasmokev1/vistasmokev4_nocull"
};

void DrawBeam(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_pszModelName = "sprites/physbeam.vmt";
	beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 1.0f;
	beamInfo.m_flWidth = 1.0f;
	beamInfo.m_flEndWidth = 1.0f;
	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 0.0f;
	beamInfo.m_flBrightness = color.a();
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = 0;

	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;

	Beam_t* myBeam = g_pViewRenderBeams->CreateBeamPoints(beamInfo);

	if (myBeam)
		g_pViewRenderBeams->DrawBeam(myBeam);
}

void __stdcall Hooks::FrameStageNotify(ClientFrameStage_t curStage)
{
	static std::string old_Skyname = "";
	static bool OldNightmode;
	static int OldSky;

	if (!G::LocalPlayer || !g_pEngine->IsConnected() || !g_pEngine->IsInGame())
	{
		clientVMT->GetOriginalMethod<FrameStageNotifyFn>(36)(curStage);
		old_Skyname = "";
		OldNightmode = false;
		OldSky = 0;
		return;
	}



	if (OldNightmode != Clientvariables->Visuals.nightmode)
	{

		if (!r_DrawSpecificStaticProp)
			r_DrawSpecificStaticProp = g_pCvar->FindVar("r_DrawSpecificStaticProp");

		r_DrawSpecificStaticProp->SetValue(0);

		for (MaterialHandle_t i = g_pMaterialSystem->FirstMaterial(); i != g_pMaterialSystem->InvalidMaterial(); i = g_pMaterialSystem->NextMaterial(i))
		{
			IMaterial* pMaterial = g_pMaterialSystem->GetMaterial(i);

			if (!pMaterial)
				continue;

			if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
			{
				if (Clientvariables->Visuals.nightmode)
					if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
						pMaterial->ColorModulate(0.3f, 0.3f, 0.3f);
					else
						pMaterial->ColorModulate(0.05f, 0.05f, 0.05f);
				else
					pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
			}

		}
		OldNightmode = Clientvariables->Visuals.nightmode;
	}

	if (OldSky != Clientvariables->Visuals.Skybox)
	{
		auto LoadNamedSky = reinterpret_cast<void(__fastcall*)(const char*)>(FindPatternIDA("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));
		if (old_Skyname == "")
		{
			//old_Skyname = g_pCvar->FindVar("sv_skyname")->GetName();
		}

		int type = Clientvariables->Visuals.Skybox;

		/*if(type == 0)
			LoadNamedSky(old_Skyname.c_str());
		else */if (type == 1)
	LoadNamedSky("cs_baggage_skybox_");
		else if (type == 2)
			LoadNamedSky("cs_tibet");
		else if (type == 3)
			LoadNamedSky("italy");
		else if (type == 4)
			LoadNamedSky("jungle");
		else if (type == 5)
			LoadNamedSky("office");
		else if (type == 6)
			LoadNamedSky("sky_cs15_daylight02_hdr");
		else if (type == 7)
			LoadNamedSky("sky_csgo_night02");
		else if (type == 8)
			LoadNamedSky("vertigo");

		OldSky = Clientvariables->Visuals.Skybox;
	}



	static Vector oldViewPunch;
	static Vector oldAimPunch;


	Vector* view_punch = G::LocalPlayer->GetViewPunchPtr();
	Vector* aim_punch = G::LocalPlayer->GetPunchAnglePtr();

	if (curStage == FRAME_RENDER_START && G::LocalPlayer->GetHealth() > 0)
	{
		static bool enabledtp = false, check = false;

		if (GetAsyncKeyState(Clientvariables->Misc.TPKey))
		{
			if (!check)
				enabledtp = !enabledtp;
			check = true;
		}
		else
			check = false;

		if (enabledtp)
		{
			*reinterpret_cast<QAngle*>(reinterpret_cast<DWORD>(G::LocalPlayer + 0x31C0 + 0x8)) = G::AAAngle; //to visualize real/faked aa angles
		}

		if (view_punch && aim_punch && Clientvariables->Visuals.Novisrevoil)
		{
			oldViewPunch = *view_punch;
			oldAimPunch = *aim_punch;

			view_punch->Init();
			aim_punch->Init();
		}

		if (enabledtp && G::LocalPlayer->isAlive()) {
			*(bool*)((DWORD)g_pInput + 0xA5) = true;
			*(float*)((DWORD)g_pInput + 0xA8 + 0x8) = 150; // Distance
		}
		else {
			*(bool*)((DWORD)g_pInput + 0xA5) = false;
			*(float*)((DWORD)g_pInput + 0xA8 + 0x8);
		}
	}

	if (curStage == FRAME_NET_UPDATE_START)
	{
		if (Clientvariables->Visuals.BulletTracers)
		{
			float Red, Green, Blue;

			Red = Clientvariables->Colors.Bulletracer[0] * 255;
			Green = Clientvariables->Colors.Bulletracer[1] * 255;
			Blue = Clientvariables->Colors.Bulletracer[2] * 255;

			for (unsigned int i = 0; i < trace_logs.size(); i++) {

				auto *shooter = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(trace_logs[i].userid));

				if (!shooter) return;

				Color color;
				if (shooter->GetTeamNum() == 3)
					color = Color(Red, Green, Blue, 210);
				else
					color = Color(Red, Green, Blue, 210);

				DrawBeam(trace_logs[i].start, trace_logs[i].position, color);

				trace_logs.erase(trace_logs.begin() + i);
			}
		}


		for (auto material_name : smoke_materials) {
			IMaterial* mat = g_pMaterialSystem->FindMaterial(material_name, TEXTURE_GROUP_OTHER);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, Clientvariables->Visuals.Nosmoke ? true : false);
		}

		if (Clientvariables->Visuals.Nosmoke) {
			static int* smokecount = *(int**)(FindPatternIDA("client.dll", "A3 ? ? ? ? 57 8B CB") + 0x1);
			*smokecount = 0;
		}

		for (int i = 1; i < g_pGlobals->maxClients; i++)
		{

			CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
			if (pEntity)
			{
				if (pEntity->GetHealth() > 0)
				{
					if (i != g_pEngine->GetLocalPlayer())
					{
						VarMapping_t* map = pEntity->GetVarMap();
						if (map)
						{
							if (Clientvariables->Ragebot.PositionAdjustment)
							{
								map->m_nInterpolatedEntries = 0;
							}
							else
							{
								if (map->m_nInterpolatedEntries == 0)
									map->m_nInterpolatedEntries = 6;
							}
						}

					}
				}
			}
		}
	}

	if (curStage == FRAME_RENDER_START)
	{
		for (int i = 1; i < g_pGlobals->maxClients; i++)
		{
			if (i == g_pEngine->GetLocalPlayer())
				continue;
			CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
			if (pEntity)
			{
				if (pEntity->GetHealth() > 0 && !pEntity->IsDormant())
				{
					*(int*)((uintptr_t)pEntity + 0xA30) = g_pGlobals->framecount; //we'll skip occlusion checks now
					*(int*)((uintptr_t)pEntity + 0xA28) = 0;//clear occlusion flags
				}
			}
		}
	}

	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < g_pGlobals->maxClients; i++)
		{
			if (i == g_pEngine->GetLocalPlayer())
				continue;
			CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);

			if (pEntity && pEntity->GetHealth() > 0)
			{
				if (pEntity->IsDormant())
					continue;

				if (Clientvariables->Ragebot.AutomaticResolver)
				{
					if (G::weaponfirecalled)
					{
						if (!G::playerhurtcalled)
						{
							if (isPartOf("Brute", pResolverData[i].resolvermode))
							{
								pResolverData[i].addyaw += 65.f;
								Math::NormalizeYaw(pResolverData[i].addyaw);
							}
						}
						else
							G::playerhurtcalled = false;
						G::weaponfirecalled = false;
					}
					for (int w = 0; w < 13; w++)
					{
						AnimationLayer currentLayer = pEntity->GetAnimOverlay(w);
						const int activity = pEntity->GetSequenceActivity(currentLayer.m_nSequence);
						float flcycle = currentLayer.m_flCycle, flprevcycle = currentLayer.m_flPrevCycle, flweight = currentLayer.m_flWeight, flweightdatarate = currentLayer.m_flWeightDeltaRate;
						uint32_t norder = currentLayer.m_nOrder;
						Vector* pAngles = pEntity->GetEyeAnglesPtr();

						if (norder == 12)
						{
							pResolverData[pEntity->GetIndex()].fakewalkdetection[G::UserCmd->command_number % 2] = flweight;
							pResolverData[pEntity->GetIndex()].fakewalkdetectionsimtime[G::UserCmd->command_number % 2] = pEntity->GetSimulationTime();
							for (int t = 0; t < 2; t++)
							{
								int resetmovetick2{};
								if (pResolverData[pEntity->GetIndex()].fakewalkdetection[t] > 0.f)
									pResolverData[pEntity->GetIndex()].resetmovetick = t;
								else if (t == 1)
								{
									if (pEntity->GetVelocity().Length2D() < 0.50 && flweight == 0.f)
									{
										pResolverData[pEntity->GetIndex()].fakewalk = "Not Moving";
										pResolverData[pEntity->GetIndex()].bfakewalk = false;
									}
								}
								else {
									if (pResolverData[pEntity->GetIndex()].resetmovetick > 0)
										resetmovetick2 = pResolverData[pEntity->GetIndex()].resetmovetick - 1;
									else
										resetmovetick2 = pResolverData[pEntity->GetIndex()].resetmovetick + 1;

									if (pResolverData[pEntity->GetIndex()].fakewalkdetection[resetmovetick2] == 0.f)
									{
										pResolverData[pEntity->GetIndex()].fakewalk = "Fake Walking";
										pResolverData[pEntity->GetIndex()].bfakewalk = true;
									}
								}
							}
						}

						if (pEntity->GetVelocity().Length2D() >= 0.50 && norder == 6 && flweight >= 0.550000 || pEntity->GetVelocity().Length2D() >= 0.50 && norder == 5 && flweight >= 0.550000 || !pResolverData[pEntity->GetIndex()].bfakewalk && pEntity->GetVelocity().Length2D() >= 0.50)
						{
							pResolverData[pEntity->GetIndex()].lastmovinglby = pEntity->LowerBodyYaw();
							pResolverData[pEntity->GetIndex()].resolvermode = "LBY Move";
							pResolverData[pEntity->GetIndex()].fakewalk = "No Fake Walk";
							pAngles->y = pEntity->LowerBodyYaw();
						}
						else
						{

							if (activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST && flweight <= 1.0f && flcycle <= 0.851166f) //[06:40AM] == > Activity > 979, Weight > 0.494118, Cycle > 0.851166
							{
								pResolverData[pEntity->GetIndex()].resolvermode = "Less BA Brute";
								G::FakeDetection[i] = 1;
								pAngles->y = pResolverData[pEntity->GetIndex()].lastmovinglby;
								pResolverData[pEntity->GetIndex()].addyaw > 0.f ? pAngles->y = pEntity->LowerBodyYaw() - pResolverData[pEntity->GetIndex()].addyaw - 77.5 : pAngles->y = pEntity->LowerBodyYaw() - 77.5;
							}

							else if (activity == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING || activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST && flweight == 0.000000f && flcycle >= 0.955994) // High delta
							{
								pResolverData[pEntity->GetIndex()].resolvermode = "LBY Brute";
								G::FakeDetection[i] = 2;
								pAngles->y = pResolverData[pEntity->GetIndex()].lastmovinglby;
								pResolverData[pEntity->GetIndex()].addyaw > 0.f ? pAngles->y = pEntity->LowerBodyYaw() - pResolverData[pEntity->GetIndex()].addyaw : pAngles->y = pEntity->LowerBodyYaw() - 17.5;
							}
						}
						Math::NormalizeVector(*pAngles);
					}

					if (!Clientvariables->Misc.AntiUT)
					{
						std::string strPitch = std::to_string(pEntity->GetEyeAnglesPtr()->x);

						if (pEntity->GetEyeAnglesPtr()->x < -179.f) pEntity->GetEyeAnglesPtr()->x += 360.f;
						else if (pEntity->GetEyeAnglesPtr()->x > 90.0 || pEntity->GetEyeAnglesPtr()->x < -90.0) pEntity->GetEyeAnglesPtr()->x = 89.f;
						else if (pEntity->GetEyeAnglesPtr()->x > 89.0 && pEntity->GetEyeAnglesPtr()->x < 91.0) pEntity->GetEyeAnglesPtr()->x -= 90.f;
						else if (pEntity->GetEyeAnglesPtr()->x > 179.0 && pEntity->GetEyeAnglesPtr()->x < 181.0) pEntity->GetEyeAnglesPtr()->x -= 180;
						else if (pEntity->GetEyeAnglesPtr()->x > -179.0 && pEntity->GetEyeAnglesPtr()->x < -181.0) pEntity->GetEyeAnglesPtr()->x += 180;
						else if (fabs(pEntity->GetEyeAnglesPtr()->x) == 0) pEntity->GetEyeAnglesPtr()->x = std::copysign(89.0f, pEntity->GetEyeAnglesPtr()->x);
					}
				}
				else
				{
					g_BacktrackHelper->PlayerRecord[i].records.clear();
				}
			}
		}

		if (Clientvariables->Skinchanger.Enabled)
		{

			CBaseCombatWeapon* pWeapon = G::LocalPlayer->GetWeapon();

			if (pWeapon)
			{
				if (pWeapon->GetItemDefinitionIndex() == WEAPON_GLOCK)
				{
					int GlockLoop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < GlockLoop; i++)
					{
						if (Clientvariables->Skinchanger.GlockSkin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_P250)
				{
					int P250Loop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < P250Loop; i++)
					{
						if (Clientvariables->Skinchanger.P250Skin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_DEAGLE)
				{
					int DeagleLoop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < DeagleLoop; i++)
					{
						if (Clientvariables->Skinchanger.DeagleSkin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_TEC9)
				{
					int Tec9Loop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < Tec9Loop; i++)
					{
						if (Clientvariables->Skinchanger.tec9Skin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_CZ75)
				{
					int CZAutoLoop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < CZAutoLoop; i++)
					{
						if (Clientvariables->Skinchanger.CZ75Skin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
				{
					int RevolverLoop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < RevolverLoop; i++)
					{
						if (Clientvariables->Skinchanger.RevolverSkin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_P2000)
				{
					int P2000Loop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < P2000Loop; i++)
					{
						if (Clientvariables->Skinchanger.P2000Skin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_USPS)
				{
					int USPLoop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < USPLoop; i++)
					{
						if (Clientvariables->Skinchanger.USPSkin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_DUALS)
				{
					int DualsLoop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < DualsLoop; i++)
					{
						if (Clientvariables->Skinchanger.DualSkins == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_FIVE7)
				{
					int FiveSevenLoop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < FiveSevenLoop; i++)
					{
						if (Clientvariables->Skinchanger.FiveSevenSkin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
				else if (pWeapon->GetItemDefinitionIndex() == WEAPON_AK47)
				{
					int AK47Loop = SIZEOF(Enumerated_Skins_Values);

					for (int i = 0; i < AK47Loop; i++)
					{
						if (Clientvariables->Skinchanger.AK47Skin == i) {
							pWeapon->SetPattern(Enumerated_Skins_Values[i], 1, 1, 1337, Enumerated_Skin_Names[i]);
						}
					}
				}
			}
		}
	}

	if (curStage == FRAME_RENDER_START)
	{
		for (int i = 1; i < g_pGlobals->maxClients; i++)
		{
			CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
			if (pEntity)
			{
				if (pEntity->GetHealth() > 0 && !pEntity->IsDormant())
				{

					g_BacktrackHelper->UpdateBacktrackRecords(pEntity);
					g_BacktrackHelper->UpdateExtrapolationRecords(pEntity);

				}
			}
		}
	}

	clientVMT->GetOriginalMethod<FrameStageNotifyFn>(36)(curStage);

	if (curStage == FRAME_RENDER_START && G::LocalPlayer && G::LocalPlayer->GetHealth() > 0)
	{
		if (Clientvariables->Visuals.Novisrevoil)
		{
			*aim_punch = oldAimPunch;
			*view_punch = oldViewPunch;
		}
	}
}



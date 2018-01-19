#include "hooks.h"
#include "Menu.h"
#include "global.h"
#include "MaterialHelper.h"
#include "xor.h"

void __fastcall Hooks::scene_end(void* thisptr, void* edx) {

	static auto scene_end_o = renderviewVMT->GetOriginalMethod< decltype(&scene_end) >(9);
	scene_end_o(thisptr, edx);

	if (Clientvariables->Visuals.ChamsEnable)
	{
		//float Red, Green, Blue, RedZ, GreenZ, BlueZ;

		static IMaterial* CoveredLit = g_MaterialHelper->CreateMaterial(true);
		static IMaterial* OpenLit = g_MaterialHelper->CreateMaterial(false);
		static IMaterial* CoveredFlat = g_MaterialHelper->CreateMaterial(true, false);
		static IMaterial* OpenFlat = g_MaterialHelper->CreateMaterial(false, false);

		IMaterial *covered = Clientvariables->Visuals.ChamsStyle == 1 ? CoveredLit : CoveredFlat;
		IMaterial *open = Clientvariables->Visuals.ChamsStyle == 1 ? OpenLit : OpenFlat;

		auto b_IsThirdPerson = *reinterpret_cast<bool*>(reinterpret_cast<DWORD>(g_pInput) + 0xA5);

		for (int i = 1; i < g_pEngine->GetMaxClients(); ++i) {
			CBaseEntity* ent = (CBaseEntity*)g_pEntitylist->GetClientEntity(i);

			if (ent == G::LocalPlayer && G::LocalPlayer != nullptr)
			{
				if (G::LocalPlayer->isAlive())
				{
					if (b_IsThirdPerson && Clientvariables->Visuals.FakeAngleGhost != 0)
					{
						Vector OrigAng;
						OrigAng = ent->GetEyeAngles();

						if (Clientvariables->Visuals.FakeAngleGhost == 1)
							ent->SetAngle2(Vector(0, G::FakeAngle.y, 0));
						else
							ent->SetAngle2(Vector(0, G::LocalPlayer->LowerBodyYaw(), 0));

						g_pRenderView->SetColorModulation(Clientvariables->Colors.FakeAngleGhost);
						g_pModelRender->ForcedMaterialOverride(covered);
						ent->draw_model(0x1, 255);
						g_pModelRender->ForcedMaterialOverride(nullptr);
						ent->SetAngle2(OrigAng);
					}
				}
			}

			if (ent->IsValidRenderable() && Clientvariables->Visuals.ChamsPlayer)
			{
				if (Clientvariables->Visuals.ChamsPlayerWall)
				{
					g_pRenderView->SetColorModulation(Clientvariables->Colors.PlayerChamsWall);
					g_pModelRender->ForcedMaterialOverride(covered);
					ent->draw_model(0x1/*STUDIO_RENDER*/, 255);
					g_pModelRender->ForcedMaterialOverride(nullptr);
				}
				g_pRenderView->SetColorModulation(Clientvariables->Colors.PlayerChams);
				g_pModelRender->ForcedMaterialOverride(open);
				ent->draw_model(0x1/*STUDIO_RENDER*/, 255);
				g_pModelRender->ForcedMaterialOverride(nullptr);
			}
		}
	}
}

void __fastcall Hooks::DrawModelExecute(void* ecx, void* edx, void* * ctx, void *state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld)
{
	if (!G::LocalPlayer)
	{
		modelrenderVMT->GetOriginalMethod<DrawModelExecuteFn>(21)(ecx, ctx, state, pInfo, pCustomBoneToWorld);
		return;
	}

	const char* ModelName = g_pModelInfo->GetModelName((model_t*)pInfo.pModel);

	static IMaterial* pWireframeMaterial = g_MaterialHelper->CreateMaterial(false, false, true);
	static IMaterial* OpenFlat = g_MaterialHelper->CreateMaterial(false, false);

	if (!*(bool*)((DWORD)g_pInput + 0xA5)) { // Thirdperson

		if (Clientvariables->Visuals.WeaponChams && strstr(ModelName, XorStr("models/weapons")))
		{
			g_pRenderView->SetColorModulation(Clientvariables->Colors.ChamsWeapon);
			g_pRenderView->SetBlend(1.f);
			g_pModelRender->ForcedMaterialOverride(OpenFlat);

			modelrenderVMT->GetOriginalMethod<DrawModelExecuteFn>(21)(ecx, ctx, state, pInfo, pCustomBoneToWorld);

			if (Clientvariables->Visuals.WeaponWireframe)
			{
				g_pRenderView->SetColorModulation(Clientvariables->Colors.WireframeWeapon);
				g_pRenderView->SetBlend(1.f);
				g_pModelRender->ForcedMaterialOverride(pWireframeMaterial);

			}
		}
	}

	if (Clientvariables->Visuals.ChamsHands && strstr(ModelName, XorStr("arms")))
	{
		g_pRenderView->SetColorModulation(Clientvariables->Colors.ChamsHand);
		g_pRenderView->SetBlend(1.f);
		g_pModelRender->ForcedMaterialOverride(OpenFlat);

		modelrenderVMT->GetOriginalMethod<DrawModelExecuteFn>(21)(ecx, ctx, state, pInfo, pCustomBoneToWorld);

		if (Clientvariables->Visuals.ChamsHandsWireframe)
		{
			g_pRenderView->SetColorModulation(Clientvariables->Colors.WireframeHand);
			g_pRenderView->SetBlend(1.f);
			g_pModelRender->ForcedMaterialOverride(pWireframeMaterial);
		}
	}
	modelrenderVMT->GetOriginalMethod<DrawModelExecuteFn>(21)(ecx, ctx, state, pInfo, pCustomBoneToWorld);
	g_pModelRender->ForcedMaterialOverride(NULL);
}

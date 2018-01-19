#include "sdk.h"
#include "ESP.h"
#include "global.h"
#include "xor.h"
#include "Math.h"
#include "Menu.h"
#include "Misc.h"
#include "BacktrackingHelper.h"
#include "GameUtils.h"
#include "Draw.h"
CESP* g_ESP = new CESP;
#define TICK_INTERVAL			( g_pGlobals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )

char* GetWeaponName(CBaseCombatWeapon *pWeapon)
{
	int ID = pWeapon->WeaponID();

	switch (ID) {
	case 4:
		return "Glock";
	case 2:
		return "Elite";
	case 36:
		return "P250";
	case 30:
		return "Tec9";
	case 1:
		return "Deagle";
	case 32:
		return "P2000";
	case 3:
		return "FiveSeven";
	case 64:
		return "Revolver";
	case 63:
		return "CZ75 Auto";
	case 61:
		return "USP";
	case 35:
		return "Nova";
	case 25:
		return "XM1014";
	case 29:
		return "Sawed Off";
	case 14:
		return "M249";
	case 28:
		return "Negev";
	case 27:
		return "Mag7";
	case 17:
		return "Mac10";
	case 33:
		return "MP7";
	case 24:
		return "UMP-45";
	case 19:
		return "P90";
	case 26:
		return "Bizon";
	case 34:
		return "MP9";
	case 10:
		return "Famas";
	case 16:
		return "M4A4";
	case 40:
		return "SSG08";
	case 8:
		return "Aug";
	case 9:
		return "AWP";
	case 38:
		return "SCAR20";
	case 13:
		return "Galil";
	case 7:
		return "Ak47";
	case 39:
		return "SG553";
	case 11:
		return "G3SG1";
	case 60:
		return "M4A1-S";
	case 46:
	case 48:
		return "Molotov";
	case 44:
		return "Grenade";
	case 43:
		return "Flashbang";
	case 45:
		return "Smoke";
	case 47:
		return "Decoy";
	case 31:
		return "Taser";
	default:
		return "Knife";
	}
}

template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

bool GetCBaseEntityBox(CBaseEntity* pBaseEntity, Vector& BotCenter, Vector& TopCenter, float& Left, float& Right, bool is_player)
{
	if (!pBaseEntity)
		return false;

	if (!is_player)
	{
		const VMatrix& trans = pBaseEntity->GetCollisionBoundTrans();

		CollisionProperty* collision = pBaseEntity->GetCollision();

		if (!collision)
			return false;

		Vector min, max;

		//pBaseEntity->GetRenderable()->GetRenderBounds(min, max);
		min = collision->VecMins();
		max = collision->VecMaxs();

		Vector points[] = { Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z) };

		auto vector_transform = [](const Vector in1, const VMatrix& in2)
		{
			auto dot_product = [](const Vector &v1, const float *v2)
			{
				float ret = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				return ret;
			};

			auto out = Vector();
			out[0] = dot_product(in1, in2[0]) + in2[0][3];
			out[1] = dot_product(in1, in2[1]) + in2[1][3];
			out[2] = dot_product(in1, in2[2]) + in2[2][3];
			return out;
		};

		Vector pointsTransformed[8];
		for (int i = 0; i < 8; i++)
		{
			pointsTransformed[i] = vector_transform(points[i], trans);
		}

		Vector pos = pBaseEntity->GetAbsOrigin();
		Vector flb;
		Vector brt;
		Vector blb;
		Vector frt;
		Vector frb;
		Vector brb;
		Vector blt;
		Vector flt;


		if (!GameUtils::WorldToScreen(pointsTransformed[3], flb) || !GameUtils::WorldToScreen(pointsTransformed[5], brt)
			|| !GameUtils::WorldToScreen(pointsTransformed[0], blb) || !GameUtils::WorldToScreen(pointsTransformed[4], frt)
			|| !GameUtils::WorldToScreen(pointsTransformed[2], frb) || !GameUtils::WorldToScreen(pointsTransformed[1], brb)
			|| !GameUtils::WorldToScreen(pointsTransformed[6], blt) || !GameUtils::WorldToScreen(pointsTransformed[7], flt))
			return false;

		Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
		//+1 for each cuz of borders at the original box
		float left = flb.x;        // left
		float top = flb.y;        // top
		float right = flb.x;    // right
		float bottom = flb.y;    // bottom

		for (int i = 1; i < 8; i++)
		{
			if (left > arr[i].x)
				left = arr[i].x;
			if (top < arr[i].y)
				top = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (bottom > arr[i].y)
				bottom = arr[i].y;
		}
		BotCenter = Vector(right - ((right - left) / 2), top, 0);
		TopCenter = Vector(right - ((right - left) / 2), bottom, 0);
		Left = left;
		Right = right;
	}
	else
	{
		Vector org = pBaseEntity->GetAbsOrigin();
		Vector head;
		if ((*pBaseEntity->GetFlags() & FL_DUCKING))
			head = org + Vector(0.f, 0.f, 52.f);
		else
			head = org + Vector(0.f, 0.f, 72.f);

		Vector org_screen, head_screen;
		if (!GameUtils::WorldToScreen(org, org_screen) || !GameUtils::WorldToScreen(head, head_screen))
			return false;

		int height = int(org_screen.y - head_screen.y);
		int width = int(height / 2);
		Left = int(head_screen.x - width / 2);
		Right = int(head_screen.x + width / 2);
		BotCenter = Vector(head_screen.x, org_screen.y);
		TopCenter = BotCenter; TopCenter.y = head_screen.y;
	}
	return true;
}

void CESP::DrawBones(CBaseEntity* pBaseEntity, int r, int g, int b, int a)
{
	float Red = Clientvariables->Colors.Skeletons[0] * 255;
		float Green = Clientvariables->Colors.Skeletons[1] * 255;
		float Blue = Clientvariables->Colors.Skeletons[2] * 255;


	int index = pBaseEntity->Index();
	studiohdr_t* pStudioModel = g_pModelInfo->GetStudioModel((model_t*)pBaseEntity->GetModel());
	if (pStudioModel && g_BacktrackHelper->PlayerRecord[index].records.size() > 0)
	{
		tick_record record = g_BacktrackHelper->PlayerRecord[index].records.back();

		float lerptime = g_BacktrackHelper->GetLerpTime();
		float desired_time = record.m_flSimulationTime + lerptime;
		float estimated_time = g_BacktrackHelper->GetEstimateServerTime();

		float SV_MAXUNLAG = 1.0f;

		float latency = g_BacktrackHelper->GetNetworkLatency();
		float m_flLerpTime = g_BacktrackHelper->GetLerpTime();
		float correct = clamp<float>(latency + m_flLerpTime, 0.0f, SV_MAXUNLAG);

		float deltaTime = correct - (estimated_time + lerptime - desired_time);

		if (fabs(deltaTime) <= 0.2f)
		{

			for (int i = 0; i < pStudioModel->numbones; i++)
			{
				mstudiobone_t* pBone = pStudioModel->pBone(i);

				if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
					continue;

				Vector sBonePos1 = GameUtils::GetBonePosition(pBaseEntity, i, record.boneMatrix), vBonePos1; //.back() for nice skeleton

				if (sBonePos1 == Vector(0, 0, 0))
					continue;
				if (!GameUtils::WorldToScreen(sBonePos1, vBonePos1))
					continue;

				Vector sBonePos2 = GameUtils::GetBonePosition(pBaseEntity, pBone->parent, record.boneMatrix), vBonePos2;//.back() for nice skeleton

				if (sBonePos2 == Vector(0, 0, 0))
					continue;
				if (!GameUtils::WorldToScreen(sBonePos2, vBonePos2))
					continue;

				Color col = Color(Red, Green, Blue, 255);

				if (record.needs_extrapolation)
					col = Color(255, 40, 0, 255);

				g_Draw.LineRGBA((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, col.r(), col.g(), col.b(), 255);

				if (record.needs_extrapolation)
				{
					Vector position = record.m_vecOrigin;
					Vector extr_position = position;
					float simtime = record.m_flSimulationTime;
					g_BacktrackHelper->ExtrapolatePosition(pBaseEntity, extr_position, simtime, record.m_vecVelocity);

					Vector sBonePos1 = GameUtils::GetBonePosition(pBaseEntity, i, record.boneMatrix), vBonePos1; //.back() for nice skeleton

					if (sBonePos1 == Vector(0, 0, 0))
						continue;

					sBonePos1 -= position;
					sBonePos1 += extr_position;

					if (!GameUtils::WorldToScreen(sBonePos1, vBonePos1))
						continue;

					Vector sBonePos2 = GameUtils::GetBonePosition(pBaseEntity, pBone->parent, record.boneMatrix), vBonePos2;//.back() for nice skeleton

					if (sBonePos2 == Vector(0, 0, 0))
						continue;

					sBonePos2 -= position;
					sBonePos2 += extr_position;

					if (!GameUtils::WorldToScreen(sBonePos2, vBonePos2))
						continue;

					Color col = Color(0, 0, 255);

					g_Draw.LineRGBA((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, col.r(), col.g(), col.b(), 255);
				}
			}
		}
	}
}

RECT GetViewport()
{
	RECT Viewport = { 0, 0, 0, 0 };
	int w, h;
	g_pEngine->GetScreenSize(w, h);
	Viewport.right = w; Viewport.bottom = h;
	return Viewport;
}

void CESP::WalkbotPoints() {

	if (Clientvariables->Misc.Walkbot)
	{

		if (G::wbpoints == -1)
			return;

		Vector pos;
		Vector draw;

		for (int i = 0; i < G::walkpoints.size(); i++)
		{
			pos = G::walkpoints[i];
			pos.z += 5.f;

			if (!GameUtils::WorldToScreen(pos, draw))
				continue;

			Color clr = Color(255, 255, 255, 50);

			if (i == G::wbcurpoint)
				clr = Color(0, 255, 0, 50);

			g_Draw.StringA(g_Draw.font_esp, true, draw.x + 5, draw.y + 15, 255, 255, 255, 255, "%d", i);
		}
	}
}

void DrawTexturedPoly(int n, Vertex_t* vertice, Color col)
{
	static int texture_id = g_pSurface->CreateNewTextureID(true);
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	g_pSurface->DrawSetTextureRGBA(texture_id, buf, 1, 1);
	g_pSurface->SetDrawColor(col);
	g_pSurface->DrawSetTexture(texture_id);
	g_pSurface->DrawTexturedPolygon(n, vertice);
}

void DrawFilledCircle(Vector2D center, Color color, float radius, float points)
{
	std::vector<Vertex_t> vertices;
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + center.x, radius * sinf(a) + center.y)));

	DrawTexturedPoly((int)points, vertices.data(), color);
}

void CESP::Crosshair()
{
	int width = 0;
	int height = 0;

	if (Clientvariables->Visuals.SpreadCrosshair)
	{
		g_pEngine->GetScreenSize(width, height);

		if (G::LocalPlayer && G::LocalPlayer->isAlive())
		{
			CBaseCombatWeapon* pWeapon = G::LocalPlayer->GetWeapon();
			float cone = pWeapon->GetSpread() + pWeapon->GetInaccuracy();

			RECT View = GetViewport();
			int MidX = View.right / 1.9999999999999;
			int MidY = View.bottom / 2;

			if (cone > 0.0f)
			{
				float Red, Green, Blue;
				Red = Clientvariables->Colors.SpreadCrosshair[0] * 255;
				Green = Clientvariables->Colors.SpreadCrosshair[1] * 255;
				Blue = Clientvariables->Colors.SpreadCrosshair[2] * 255;

				if (cone < 0.01f) cone = 0.01f;
				float size = (cone * height) * 0.7f;
				Color color(Red, Green, Blue, 10);

				g_pSurface->SetDrawColor(color);
				DrawFilledCircle(Vector2D(MidX, MidY), color, (int)size, 70);	
			}
		}
	}
}

void CESP::DrawLinesAA() {

	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = G::LocalPlayer;

	src3D = G::LocalPlayer->GetAbsOrigin();

	Math::AngleVectors(Vector(0, G::LocalPlayer->LowerBodyYaw(), 0), &forward);
	dst3D = src3D + (forward * 50.f); // Line Length
	ray.Init(src3D, dst3D);
	g_pEngineTrace->TraceRay(ray, 0, &filter, &tr);

	if (!GameUtils::WorldToScreen(src3D, src) || !GameUtils::WorldToScreen(tr.endpos, dst)) // Lowerbody
		return;

	g_pSurface->SetDrawColor(Color(0, 190, 255, 255));
	g_pSurface->DrawLine(src.x, src.y, dst.x, dst.y);
	g_Draw.StringA(g_Draw.font_esp, true, dst.x, dst.y, 0,190,255,255, "L");
}

void CESP::DrawPlayer(CBaseEntity* pPlayer, CBaseEntity* pLocalPlayer)
{
	Vector m_position = pPlayer->GetAbsOrigin();
	Vector m_top_position = m_position;

	if (*pPlayer->GetFlags() & IN_DUCK)
		m_top_position += Vector(0, 0, 52);
	else
		m_top_position += Vector(0, 0, 72);

	Vector footpos, headpos;

	if (pPlayer->IsDormant() && flPlayerAlpha[pPlayer->GetIndex()] > 0.f)
		flPlayerAlpha[pPlayer->GetIndex()] -= 0.3f;
	else if (flPlayerAlpha[pPlayer->GetIndex()] < 255.f && !pPlayer->IsDormant())
		flPlayerAlpha[pPlayer->GetIndex()] = 255.f;

	if (pPlayer != G::LocalPlayer && !pPlayer->IsDormant() && Clientvariables->Visuals.SnapLines) // Crosshair to abs origin
	{
		RECT View = GetViewport();
		int MidX = View.right / 1.9999999999999;
		int MidY = View.bottom / 2;

		Vector VecOrigin = pPlayer->GetAbsOrigin(), SecondVector;

		if (GameUtils::WorldToScreen(VecOrigin, SecondVector))
		{
			g_Draw.LineRGBA(MidX, MidY, SecondVector.x, SecondVector.y, 255, 255, 255, 255);
		}
	}

	if (GameUtils::WorldToScreen(m_position, footpos) && GameUtils::WorldToScreen(m_top_position, headpos))
	{
		Vector Bot, Top;
		float Left, Right;
		GetCBaseEntityBox(pPlayer, Bot, Top, Left, Right, true);
		int height = Bot.y - Top.y;
		int width = Right - Left;
		int x = Left;
		int y = Top.y;

		CBaseCombatWeapon* pMainWeapon = pPlayer->GetWeapon();

		if (Clientvariables->Visuals.Bones)
			DrawBones(pPlayer, 255, 255, 255, flPlayerAlpha[pPlayer->GetIndex()]);

		if (Clientvariables->Visuals.BoundingBox)
		{
			float Red = Clientvariables->Colors.BoundingBox[0] * 255;
			float Green = Clientvariables->Colors.BoundingBox[1] * 255;
			float Blue = Clientvariables->Colors.BoundingBox[2] * 255;

			if (pPlayer->IsDormant())
			{
				g_Draw.Box(x - 1, y - 1, width + 2, height + 2, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
				g_Draw.Box(x, y, width, height, 140, 140, 140, flPlayerAlpha[pPlayer->GetIndex()]); // Main
				g_Draw.Box(x + 1, y + 1, width - 2, height - 2, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
			}
			else
			{
				g_Draw.Box(x - 1, y - 1, width + 2, height + 2, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
				g_Draw.Box(x, y, width, height, Red, Green, Blue, flPlayerAlpha[pPlayer->GetIndex()]); // Main
				g_Draw.Box(x + 1, y + 1, width - 2, height - 2, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
			}
		}

		if (Clientvariables->Visuals.Armor)
		{
			if (pPlayer->GetArmor() > 0)
			{
				char hp[50];
				sprintf_s(hp, sizeof(hp), "%i", pPlayer->GetArmor());
				if (pPlayer->HasHelmet())
					g_Draw.StringA(g_Draw.font_espinfo, false, x + width + 3, y, 255, 255, 255, flPlayerAlpha[pPlayer->GetIndex()], "HK");
				else
					g_Draw.StringA(g_Draw.font_espinfo, false, x + width + 3, y, 255, 255, 255, flPlayerAlpha[pPlayer->GetIndex()], "K");
			}
		}

		if (Clientvariables->Visuals.Flags)
		{
			if (pPlayer->IsScoped())
				g_Draw.StringA(g_Draw.font_espinfo, false, x + width + 3, y + 20, 96, 146, 144, flPlayerAlpha[pPlayer->GetIndex()], "ZOOM");
		}

		if (Clientvariables->Visuals.Fake)
		{
			if (G::FakeDetection[pPlayer->Index()] == 1)
				g_Draw.StringA(g_Draw.font_espinfo, false, x + width + 3, y + 10, 96, 146, 144, flPlayerAlpha[pPlayer->GetIndex()], "FAKE HIGH"); // High delta breaker
			else if (G::FakeDetection[pPlayer->Index()] == 2)
				g_Draw.StringA(g_Draw.font_espinfo, false, x + width + 3, y + 10, 96, 146, 144, flPlayerAlpha[pPlayer->GetIndex()], "FAKE LOW"); // Low delta
		}

		if (Clientvariables->Visuals.Health)
		{
			int health = 100 - pPlayer->GetHealth();
			int w = 4;
			if (width < 4)
				w = width;

			int hr, hg, hb;

			hr = 255 - (pPlayer->GetHealth()*2.55);
			hg = pPlayer->GetHealth() * 2.55;
			hb = 0;

			if (pPlayer->IsDormant())
			{
				g_Draw.FillRGBA(x - (5), y, w, height, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
				g_Draw.FillRGBA(x - (5), y + health * height / 100, w, pPlayer->GetHealth()*height / 100, 140, 140, 140, flPlayerAlpha[pPlayer->GetIndex()]);
				g_Draw.Box(x - (5), y, w, height, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
			}
			else
			{
				g_Draw.FillRGBA(x - (5), y, w, height, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
				g_Draw.FillRGBA(x - (5), y + health * height / 100, w, pPlayer->GetHealth()*height / 100, hr, hg, hb, flPlayerAlpha[pPlayer->GetIndex()]);
				g_Draw.Box(x - (5), y, w, height, 0, 0, 0, flPlayerAlpha[pPlayer->GetIndex()]);
			}
			if (pPlayer->GetHealth() <= 100)
			{
				char hp[20];
				sprintf_s(hp, sizeof(hp), "%i", pPlayer->GetHealth());

				g_Draw.StringA(g_Draw.font_esp, false, x - g_Draw.getWidht(hp, g_Draw.font_espnum), y + health * height / 100 - 6, 255, 255, 255, flPlayerAlpha[pPlayer->GetIndex()], "%i", pPlayer->GetHealth());
			}
		}


		int bot_add = 0;
		int top_add = 0;

		if (Clientvariables->Visuals.Name)
		{
			player_info_s info;
			g_pEngine->GetPlayerInfo(pPlayer->Index(), &info);
			g_Draw.StringA(g_Draw.font_esp, true, x + width / 2, y - 12 - top_add, 255, 255, 255, flPlayerAlpha[pPlayer->GetIndex()], "%s", info.m_szPlayerName);
			top_add += 12;
		}

		if (Clientvariables->Visuals.Weapon)
		{
			UINT* hWeapons = pPlayer->GetWeapons();
			if (hWeapons)
			{
				if (pMainWeapon)
				{
					std::string s1 = GetWeaponName(pMainWeapon);
					if (Clientvariables->Visuals.Ammo && pMainWeapon->Clip1() != -1 && pMainWeapon->GetWeaponType() != WEAPONCLASS::WEPCLASS_INVALID)
					{
						s1.append(XorStr(" | "));
						s1.append(to_string(pMainWeapon->Clip1()));
					}
					g_Draw.StringA(g_Draw.font_esp, true, x + width / 2, y + height + bot_add, 255, 255, 255, flPlayerAlpha[pPlayer->GetIndex()], XorStr("%s"), s1.c_str());
				}
			}
		}
	}
}

void CESP::DrawC4(CBaseEntity* pBomb, bool is_planted, CBaseEntity* pLocalPlayer)
{
	Vector Bot, Top;
	float Left, Right;
	GetCBaseEntityBox(pBomb, Bot, Top, Left, Right, false);
	int height = Bot.y - Top.y;
	int width = Right - Left;

	DWORD col = is_planted ? D3DCOLOR_RGBA(250, 42, 42, 255) : D3DCOLOR_RGBA(255, 255, 0, 255);

	g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y, 255, 255, 255, 255, "%s", "Bomb");

	if (is_planted)
	{
		CBomb* bomb = (CBomb*)pBomb;
		float flBlow = bomb->GetC4BlowTime();
		float TimeRemaining = flBlow - (g_pGlobals->interval_per_tick * pLocalPlayer->GetTickBase());
		if (TimeRemaining < 0)
			TimeRemaining = 0;
		char buffer[64];

		if (bomb->IsDefused())
			sprintf_s(buffer, XorStr("Defused"));
		else
			sprintf_s(buffer, XorStr("%.1fs remaining!"), TimeRemaining);


		//g_pRender->String(Left + width / 2, Bot.y + 13, centered, g_pRender->Fonts.esp, true, WHITE(255), "%s", buffer);
		g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y + 13, 255, 255, 255, 255, "%s", buffer);

		if (!bomb->IsDefused())
		{
			float a = 450.7f;
			float b = 75.68f;
			float c = 789.2f;
			float d = ((pLocalPlayer->GetAbsOrigin().DistTo(pBomb->GetOrigin()) - b) / c);
			float flDamage = a * expf(-d * d);

			auto GetArmourHealth = [](float flDamage, int ArmorValue)
			{
				float flCurDamage = flDamage;

				if (flCurDamage == 0.0f || ArmorValue == 0)
					return flCurDamage;

				float flArmorRatio = 0.5f;
				float flArmorBonus = 0.5f;
				float flNew = flCurDamage * flArmorRatio;
				float flArmor = (flCurDamage - flNew) * flArmorBonus;

				if (flArmor > ArmorValue)
				{
					flArmor = ArmorValue * (1.0f / flArmorBonus);
					flNew = flCurDamage - flArmor;
				}

				return flNew;
			};

			float damage = max((int)ceilf(GetArmourHealth(flDamage, pLocalPlayer->GetArmor())), 0);


			//g_pRender->String(Left + width / 2, Bot.y + 26, centered, g_pRender->Fonts.esp, true, WHITE(255), "Damage: %f", damage);
			g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y + 26, 255, 255, 255, 255, "Damage: %f", damage);
		}
	}
}

void CESP::DrawThrowable(CBaseEntity* pThrowable)
{
	const model_t* nadeModel = pThrowable->GetModel();

	if (!nadeModel)
		return;

	studiohdr_t* hdr = g_pModelInfo->GetStudioModel(nadeModel);

	if (!hdr)
		return;

	if (!strstr(hdr->name, XorStr("thrown")) && !strstr(hdr->name, XorStr("dropped")))
		return;

	std::string nadeName = XorStr("Unknown Grenade");

	IMaterial* mats[32];
	g_pModelInfo->GetModelMaterials(nadeModel, hdr->numtextures, mats);

	for (int i = 0; i < hdr->numtextures; i++)
	{
		IMaterial* mat = mats[i];
		if (!mat)
			continue;

		if (strstr(mat->GetName(), XorStr("flashbang")))
		{
			nadeName = XorStr("Flashbang");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("m67_grenade")) || strstr(mat->GetName(), XorStr("hegrenade")))
		{
			nadeName = XorStr("HE Grenade");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("smoke")))
		{
			nadeName = XorStr("Smoke");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("decoy")))
		{
			nadeName = XorStr("Decoy");

			break;
		}
		else if (strstr(mat->GetName(), XorStr("incendiary")) || strstr(mat->GetName(), XorStr("molotov")))
		{
			nadeName = XorStr("Molotov");

			break;
		}
	}
	Vector Bot, Top;
	float Left, Right;
	GetCBaseEntityBox(pThrowable, Bot, Top, Left, Right, false);
	int height = Bot.y - Top.y;
	int width = Right - Left;

	//g_pRender->Text((char*)nadeName.c_str(), Left + width / 2, Bot.y, centered, g_pRender->Fonts.esp, true, WHITE(255), BLACK(255));
	g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y, 255, 255, 255, 255, nadeName.c_str());
}

void CESP::DrawDroppedWeapon(CBaseCombatWeapon* pWeapon)
{
	CBaseEntity* pCBaseEntity = (CBaseEntity*)pWeapon;
	CBaseCombatWeapon* Weapon = (CBaseCombatWeapon*)pWeapon;

	if (!pCBaseEntity || !g_pModelInfo)
		return;

	std::string sCurWeapon = g_pModelInfo->GetModelName(pCBaseEntity->GetModel());
	if (!sCurWeapon.empty() && sCurWeapon.find(XorStr("w_")) != std::string::npos)
	{
		std::string name;

		if (sCurWeapon.find(XorStr("defuser")) != std::string::npos/* insert check for defuser here*/)
			name = XorStr("defuser");
		else
		{
			CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pCBaseEntity;
			if (!pWeapon)return;
			name = GetWeaponName(pWeapon);//pWeapon->GetName(false);
		}

		Vector Bot, Top;
		float Left, Right;
		GetCBaseEntityBox(pCBaseEntity, Bot, Top, Left, Right, false);
		int width = Right - Left;

		float Red = Clientvariables->Colors.DroppedWeapon[0] * 255;
		float Green = Clientvariables->Colors.DroppedWeapon[1] * 255;
		float Blue = Clientvariables->Colors.DroppedWeapon[2] * 255;

		g_Draw.StringA(g_Draw.font_esp, true, Left + width / 2, Bot.y, Red, Green, Blue, 255, name.c_str());
	}
}

void CESP::Loop()
{
	if (!G::LocalPlayer)
		return;

	if (G::LocalPlayer->isAlive())
		Crosshair();

	if (Clientvariables->Misc.Walkbot)
		WalkbotPoints();

	if (Clientvariables->Antiaim.Freestanding != 0 && G::LocalPlayer->isAlive())
		DrawLinesAA();

	for (int i = 1; i <= g_pEntitylist->GetHighestEntityIndex(); i++)
	{
		CBaseEntity* pBaseEntity = g_pEntitylist->GetClientEntity(i);

		if (!pBaseEntity)
			continue;

		ClientClass* pClass = (ClientClass*)pBaseEntity->GetClientClass();

		if (pClass->m_ClassID == int(EClassIds::CCSPlayer) && Clientvariables->Visuals.EspEnable)
		{
			if (pBaseEntity->GetHealth() < 1)
				continue;

			bool is_friendly = pBaseEntity->GetTeamNum() == G::LocalPlayer->GetTeamNum();

			if (!is_friendly || is_friendly && !Clientvariables->Visuals.EnemyOnly)
				g_ESP->DrawPlayer(pBaseEntity, G::LocalPlayer);
		}

		Vector buf, pos = pBaseEntity->GetAbsOrigin();

		if (pos.x == 0 || pos.y == 0 || pos.z == 0 || !GameUtils::WorldToScreen(pos, buf))
			continue;

		if ((pClass->m_ClassID != int(EClassIds::CBaseWeaponWorldModel) && (strstr(pClass->m_pNetworkName, XorStr("Weapon")) || pClass->m_ClassID == int(EClassIds::CDEagle) || pClass->m_ClassID == int(EClassIds::CAK47))) && Clientvariables->Visuals.DroppedWeapons == 1)
			DrawDroppedWeapon((CBaseCombatWeapon*)pBaseEntity);

		if ((pClass->m_ClassID == int(EClassIds::CC4) || pClass->m_ClassID == int(EClassIds::CPlantedC4)))
		{
			if (Clientvariables->Visuals.Bomb)
				DrawC4(pBaseEntity, pClass->m_ClassID == int(EClassIds::CPlantedC4), G::LocalPlayer);
		}

		if (strstr(pClass->m_pNetworkName, XorStr("Projectile")) && Clientvariables->Visuals.ThrownNades)
			DrawThrowable(pBaseEntity);
	}
}

void CESP::DrawScope(CBaseEntity* pLocalPlayer)
{
	CBaseCombatWeapon* pLocalWeapon = pLocalPlayer->GetWeapon();

	if (pLocalWeapon)
	{
		if (pLocalWeapon->IsSniper())
		{
			if (pLocalPlayer->IsScoped())
			{
				int width = 0;
				int height = 0;
				g_pEngine->GetScreenSize(width, height);
				int centerX = static_cast<int>(width * 0.5f);
				int centerY = static_cast<int>(height * 0.5f);
				g_pSurface->SetDrawColor(Color(0, 0, 0, 255));
				g_pSurface->DrawLine(0, centerY, width, centerY);
				g_pSurface->DrawLine(centerX, 0, centerX, height);
			}
		}
	}
}
#include "sdk.h"
#include "global.h"
#include "Draw.h"
#include "GameUtils.h"

/*---------------------ENGINE & UTIL CLASSES---------------------*/
CDataMapUtils* g_pData = new CDataMapUtils;
CEncryption* g_pEncrypt = new CEncryption;
CBaseEntity* G::LocalPlayer = nullptr;
CBaseCombatWeapon* G::MainWeapon = nullptr;
CSWeaponInfo* G::WeaponData = nullptr;
CUserCmd*	G::UserCmd = nullptr;
CUserCmd* G::UserCmdForBacktracking = nullptr;
CBaseEntity* G::Target = nullptr;
CPlayerlistInfo* g_pPlayerlistInfo[64] = { nullptr };

/*---------------------QANGLE---------------------*/
QAngle G::LastAngle = QAngle();
QAngle G::AAAngle = QAngle();
QAngle G::StrafeAngle = QAngle();
QAngle G::RealAngle = QAngle();
QAngle G::FakeAngle = QAngle();

/*---------------------BOOLEAN---------------------*/
bool G::Aimbotting = false;
bool G::BreakingLagComp = false;
bool G::Return = true;
bool G::ForceRealAA = false;
bool G::SendPacket = true;
bool G::ShowMenu = false;
bool G::Opened = false;
bool G::NewRound = false;
bool G::Init = false;
bool G::weaponfirecalled = false;
bool G::playerhurtcalled = false;
bool G::walkbotenabled = false;
bool G::bShouldChoke = false;
bool G::bFakewalking = false;

/*---------------------INTEGER---------------------*/
int G::wbpoints = -1;
int G::wbcurpoint = 0;
int G::ChokedPackets = 0;
int G::DamageDealt;
int G::ResolverMode[64];
int G::FakeDetection[64];
int G::Shots[64];
int G::MenuTab;
int G::nChokedTicks = 0;

/*---------------------FLOATING POINT---------------------*/
float G::PredictedTime = 0.f;
float G::lby_update_end_time;
float G::CurrTime;

/*---------------------D3DX DEFINES---------------------*/
HWND G::Window = nullptr;
CScreen G::Screen = CScreen();

/*---------------------STANDARD VECTOR MAPS---------------------*/
std::vector<FloatingText> G::DamageHit;
std::vector<Vector> G::walkpoints;

/*---------------------VECTOR---------------------*/
Vector G::vecUnpredictedVel = Vector(0, 0, 0);


itemTimer::itemTimer() {
	maxTime = 0;
}

itemTimer::itemTimer(float _maxTime) {
	maxTime = _maxTime;
}

float itemTimer::getTimeRemaining() {
	auto time = (timeStarted - (float(clock()) / float(CLOCKS_PER_SEC))) + maxTime;
	return time ? time : 0.00001f; //ensure we don't ever return a 0
}

float itemTimer::getTimeRemainingRatio() {
	return getTimeRemaining() / getMaxTime();
}

float itemTimer::getMaxTime() {
	return maxTime ? maxTime : 1; //don't ever return 0
}

void itemTimer::startTimer() {
	timeStarted = float(float(clock()) / float(CLOCKS_PER_SEC));
}

void itemTimer::setMaxTime(float time) {
	maxTime = time;
}

FloatingText::FloatingText(CBaseEntity* attachEnt, float lifetime, int Damage)
{
	TimeCreated = G::CurrTime;
	ExpireTime = TimeCreated + lifetime;
	pEnt = attachEnt;
	DamageAmt = Damage;
}

void FloatingText::Draw()
{
	float Red, Green, Blue;
	Red = Clientvariables->Colors.DamageIndicator[0] * 255;
	Green = Clientvariables->Colors.DamageIndicator[1] * 255;
	Blue = Clientvariables->Colors.DamageIndicator[2] * 255;
	auto head = pEnt->GetBonePos(8);
	Vector screen;

	if (GameUtils::WorldToScreen(head, screen))
	{
		auto lifetime = ExpireTime - TimeCreated;
		auto pct = 1 - ((ExpireTime - G::CurrTime) / lifetime);
		int offset = pct * 50;
		int y = screen.y - 15 - offset;

		if (DamageAmt >= 100)
		{
			g_Draw.StringA(g_Draw.font_esp, true, screen.x, y, Red, Green, Blue, 255, "Instant Death!");
		}
		else
		{
			g_Draw.StringA(g_Draw.font_esp, true, screen.x, y, Red, Green, Blue, 255, "-%i", DamageAmt);
		}
	}
}

int CDataMapUtils::Find(datamap_t* pMap, const char* szName) {
	while (pMap)
	{
		for (int i = 0; i < pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == NULL)
				continue;

			if (strcmp(szName, pMap->dataDesc[i].fieldName) == 0)
				return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

			if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED)
			{
				if (pMap->dataDesc[i].td)
				{
					unsigned int offset;

					if ((offset = Find(pMap->dataDesc[i].td, szName)) != 0)
						return offset;
				}
			}
		}
		pMap = pMap->baseMap;
	}

	return 0;
}
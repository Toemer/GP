#include "hooks.h"
#include <time.h>
#include "Mmsystem.h"
#include <thread>
#include "Hitmarker.h"
#include "global.h"
#include "Menu.h"
#include "controls.h"
#pragma comment(lib, "winmm.lib") 

#define EVENT_HOOK( x )
#define TICK_INTERVAL			(Globals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

cGameEvent g_Event;
std::vector<trace_info> trace_logs;

void cGameEvent::FireGameEvent(IGameEvent *event)
{
	/*short   m_nUserID		user ID who was hurt
	short	attacker	user ID who attacked
	byte	health		remaining health points
	byte	armor		remaining armor points
	string	weapon		weapon name attacker used, if not the world
	short	dmg_health	damage done to health
	byte	dmg_armor	damage done to armor
	byte	hitgroup	hitgroup that was damaged*/
	const char* szEventName = event->GetName();
	if (!szEventName)
		return;

	if (!strcmp(szEventName, "round_start"))
		G::NewRound = true;
	else
		G::NewRound = false;

	if (strcmp(szEventName, "player_connect") == 0)
	{
		memset(&G::Shots, 0, sizeof(G::Shots)); // reset shotsfired	
	}

	if (strcmp(szEventName, "weapon_fire") == 0)
	{
		CBaseEntity *player = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userID")));
		if (player == G::LocalPlayer)
		{
			if (G::Target != NULL)
			{
				G::Shots[G::Target->GetIndex()]++;

				G::weaponfirecalled = true;

				Msg("weapon_fire\n");
			}
		}
	}

	if (Clientvariables->Visuals.DamageIndicators)
	{
		if (strcmp(szEventName, "player_hurt") == 0)
		{
			auto attacker = event->GetInt("attacker");
			auto dmgdone = event->GetInt("dmg_health");
			auto Hurt = event->GetInt("userid");
			CBaseEntity* pEnt = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(Hurt));
			if (g_pEngine->GetPlayerForUserID(attacker) == g_pEngine->GetLocalPlayer()) {
				G::DamageDealt = dmgdone;
				G::DamageHit.push_back(FloatingText(pEnt, 1.f, dmgdone));

				G::playerhurtcalled = true;
				Msg("player_hurt\n");
			}
		}
	}

	if (strcmp(szEventName, "player_hurt") == 0)
	{
		CBaseEntity * hurt = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt(XorStr("userID"))));
		CBaseEntity * attacker = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt(XorStr("attacker"))));

		if (attacker == G::LocalPlayer)
		{
			if (hurt == G::Target)
			{
				if (G::Target != NULL)
				{
					G::Shots[G::Target->GetIndex()] -= 1;
				}
			}
		}



		bool IsHeadshot = (event->GetInt(XorStr("hitgroup")) == 1);

		int damage = event->GetInt("dmg_health");

		int CBaseEntityServerID = event->GetInt(XorStr("userID"));
		int AttackerServerID = event->GetInt(XorStr("attacker"));
		int AttackerIndex = g_pEngine->GetPlayerForUserID(AttackerServerID);
		if (AttackerIndex != g_pEngine->GetLocalPlayer())
			return;
		int CBaseEntityIndex = g_pEngine->GetPlayerForUserID(CBaseEntityServerID);
		if (CBaseEntityIndex == g_pEngine->GetLocalPlayer())
			return;

		auto* entity = g_pEntitylist->GetClientEntity(CBaseEntityIndex);


		if (!entity)
			return;

		g_Hitmarker->update_end_time();
		g_Hitmarker->play_sound();
	}

	if (Clientvariables->Visuals.Hitmarker)
	{
		if (strcmp(szEventName, "player_hurt") == 0)
		{
			int Attacker = event->GetInt("attacker");
			int UserID = event->GetInt("userid");

			CBaseEntity* Entity = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(UserID));

			if (g_pEngine->GetPlayerForUserID(Attacker) == g_pEngine->GetLocalPlayer())
			{
				g_pEngine->ClientCmd_Unrestricted("play buttons\\arena_switch_press_02.wav");
			}
		}
	}

	if (strcmp(szEventName, "bullet_impact") == 0)
	{
		auto* index = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));

		//return if the userid is not valid or we werent the entity who was firing
		//if (G::LocalPlayer)
		//{
		//get the bullet impact's position
		Vector position(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));

		//	if (VisualElements.Visual_Player_EnemyOnly->Checked && index->GetTeamNum() == G::LocalPlayer->GetTeamNum())
		//		return;

		//	Msg("pos = %f, %f, %f", event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
		if (index)
			trace_logs.push_back(trace_info(index->GetEyePosition(), position, g_pGlobals->curtime, event->GetInt("userid")));
		//	}
	}
}

int cGameEvent::GetEventDebugID()
{
	return 42;
}

void cGameEvent::RegisterSelf()
{
	g_pGameEventManager->AddListener(this, "player_connect", false);
	g_pGameEventManager->AddListener(this, "player_hurt", false);
	g_pGameEventManager->AddListener(this, "round_start", false);
	g_pGameEventManager->AddListener(this, "round_end", false);
	g_pGameEventManager->AddListener(this, "player_death", false);
	g_pGameEventManager->AddListener(this, "weapon_fire", false);
	g_pGameEventManager->AddListener(this, "bullet_impact", false);
}

void cGameEvent::Register()
{
	EVENT_HOOK(FireEvent);
}
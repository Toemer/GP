#include "sdk.h"
#include "PredictionSystem.h"
#include "global.h"
#include "xor.h"

static char* cMoveData = nullptr;

void CPredictionSystem::EnginePrediction(CUserCmd* pCmd)
{
	if (!g_pMoveHelper || !pCmd || !G::LocalPlayer)
		return;

	CMoveData C_MoveData;

	//Let's back things up for later use
	iFlagsBackup = *G::LocalPlayer->GetFlags();
	iButtonsBackup = pCmd->buttons;

	//Set m_pCurrentCommand to cmd. We'll set this to nullptr later.
	G::LocalPlayer->SetCurrentCommand(pCmd);

	//Let's get some signatures now
	if (!m_pPredictionRandomSeed || !m_pSetPredictionPlayer)
	{
		m_pPredictionRandomSeed = *reinterpret_cast<int**>(FindPatternIDA(("client.dll"), "A3 ? ? ? ? 66 0F 6E 86") + 1);
		m_pSetPredictionPlayer = *reinterpret_cast<int**>(FindPatternIDA(("client.dll"), "89 35 ? ? ? ? F3 0F 10 48 20") + 2);
	}

	//We don't want to be calling this every tick, bad for framerate.
	//Thanks, zbe and Heep
	if (!cMoveData)
		cMoveData = (char*)(calloc(1, sizeof(CMoveData)));

	//Let's set it up so that it predicts ourselves
	g_pMoveHelper->SetHost(G::LocalPlayer);
	*m_pPredictionRandomSeed = MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;
	*m_pSetPredictionPlayer = uintptr_t(G::LocalPlayer);

	//Let's set some global variables
	g_pGlobals->curtime = G::LocalPlayer->GetTickBase() * g_pGlobals->interval_per_tick;
	g_pGlobals->frametime = g_pGlobals->interval_per_tick;

	//Don't know is for exactly, but here's the relevant IDA picture of it. https://i.imgur.com/hT6caQV.png
	pCmd->buttons |= *reinterpret_cast< uint8_t* >(uintptr_t(G::LocalPlayer) + 0x3310);

	//This is for flashlights in older Source games, Thanks, friendly for the info
	if (pCmd->impulse)
		*reinterpret_cast< uint8_t* >(uintptr_t(G::LocalPlayer) + 0x31EC) = pCmd->impulse;

	//Here we're doing CBasePlayer::UpdateButtonState
	C_MoveData.m_nButtons = pCmd->buttons;
	int buttonsChanged = pCmd->buttons ^ *reinterpret_cast<int*>(uintptr_t(G::LocalPlayer) + 0x31E8);
	*reinterpret_cast<int*>(uintptr_t(G::LocalPlayer) + 0x31DC) = (uintptr_t(G::LocalPlayer) + 0x31E8);
	*reinterpret_cast<int*>(uintptr_t(G::LocalPlayer) + 0x31E8) = pCmd->buttons;
	*reinterpret_cast<int*>(uintptr_t(G::LocalPlayer) + 0x31E0) = pCmd->buttons & buttonsChanged;  //m_afButtonPressed ~ The changed ones still down are "pressed"
	*reinterpret_cast<int*>(uintptr_t(G::LocalPlayer) + 0x31E4) = buttonsChanged & ~pCmd->buttons; //m_afButtonReleased ~ The ones not down are "released"

	g_pGameMovement->StartTrackPredictionErrors(G::LocalPlayer);

	iTickBaseBackup = G::LocalPlayer->GetTickBase();

	g_pPrediction->SetupMove(G::LocalPlayer, pCmd, g_pMoveHelper, reinterpret_cast< CMoveData* >(cMoveData));
	g_pGameMovement->ProcessMovement(G::LocalPlayer, reinterpret_cast< CMoveData* >(cMoveData));
	g_pPrediction->FinishMove(G::LocalPlayer, pCmd, reinterpret_cast< CMoveData* >(cMoveData));

	//Let's override our tickbase with the backed up tickbase
	G::LocalPlayer->GetTickBase() = iTickBaseBackup;

	g_pGameMovement->FinishTrackPredictionErrors(G::LocalPlayer);

	//Let's nullify these here
	G::LocalPlayer->SetCurrentCommand(nullptr);
	*m_pPredictionRandomSeed = -1;
	*m_pSetPredictionPlayer = 0;
	g_pMoveHelper->SetHost(0);

	//Last but not least, set these to their backups 
	*G::LocalPlayer->GetFlags() = iFlagsBackup;
	pCmd->buttons = iButtonsBackup;
}
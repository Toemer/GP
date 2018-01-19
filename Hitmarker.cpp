#include "sdk.h"
#include "Hitmarker.h"
#include "Menu.h"

CHitmarker* g_Hitmarker = new CHitmarker;

void CHitmarker::update_end_time()
{
	this->end_time = g_pGlobals->curtime + 0.2f;
}
void CHitmarker::draw()
{
	if (g_pGlobals->curtime > this->end_time)
		return;
}

void CHitmarker::play_sound()
{
	if(Clientvariables->Visuals.Hitmarker == 2)
		g_pEngine->ClientCmd_Unrestricted("play buttons\\arena_switch_press_02.wav");
}
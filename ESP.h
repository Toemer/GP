#pragma once
struct player_esp
{
	CBaseEntity* pPlayer = nullptr;
	bool is_visible = false;
	bool render = false;
	int screen_x = 0, screen_y = 0, width = 0, height = 0;
	DWORD color = D3DCOLOR_RGBA(255, 255, 255, 255);

	player_esp()
	{

	}
};
class CESP
{
public:
	player_esp player_draw_array[64];
	float flPlayerAlpha[64];

	void DrawPlayer(CBaseEntity* pPlayer, CBaseEntity* pLocalPlayer);
	void DrawC4(CBaseEntity* pBomb,bool is_planted, CBaseEntity* pLocalPlayer);

	void DrawThrowable(CBaseEntity* pThrowable);
	void DoEnemyCircle(const Vector & vecDelta, float flRotation);
	void DrawDroppedWeapon(CBaseCombatWeapon* pWeapon);
	void DrawBones(CBaseEntity* pBaseEntity, int r, int g, int b, int a);
	void WalkbotPoints();
	void Crosshair();
	void DrawLinesAA();
	void DrawScope(CBaseEntity* pLocalPlayer);
	void Loop();
	

}; extern CESP* g_ESP;
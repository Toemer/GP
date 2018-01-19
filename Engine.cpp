#include "sdk.h"
#include "xor.h"
typedef void(__thiscall* SetConVarConstructor_t)(void*);
SetConVarConstructor_t pNET_SetConVar;
typedef void(__thiscall* SetConVarInit_t)(void*, const char*, const char*);
SetConVarInit_t pNET_SetConVarInit;
NET_SetConVar::NET_SetConVar(const char* name, const char* value)
{
	if(!pNET_SetConVar)
		pNET_SetConVar = (SetConVarConstructor_t)(Utilities::Memory::FindPattern(XorStr("engine.dll"), (PBYTE)XorStr("\x83\xE9\x04\xE9\x00\x00\x00\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x56\x8B\xF1\xC7\x06\x00\x00\x00\x00\x8D\x4E\x08"), XorStr("xxxx????xxxxxxxxxxxxxxxx????xxx")) + 19);
	
	pNET_SetConVar(this);

	if(!pNET_SetConVarInit)
		pNET_SetConVarInit = (SetConVarInit_t)Utilities::Memory::FindPattern(XorStr("engine.dll"), (PBYTE)XorStr("\x55\x8B\xEC\x56\x8B\xF1\x57\x83\x4E\x14\x01\x83\x7E\x0C\x00"), XorStr("xxxxxxxxxxxxxxx"));
	
	pNET_SetConVarInit(this, name, value);
}
NET_SetConVar::~NET_SetConVar()
{
	if(!pNET_SetConVar)
		pNET_SetConVar = (SetConVarConstructor_t)(Utilities::Memory::FindPattern(XorStr("engine.dll"), (PBYTE)XorStr("\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x56\x8B\xF1\x57\x8D\x7E\x04\xC7\x07\x00\x00\x00\x00"), XorStr("xxxxxxxxxxxxxxxxxxxxx????")) + 12);
	
	pNET_SetConVar(this);
}
void CEngine::SetName(std::string name)
{
	//NET_SetConVar convar(XorStr("name"), name.c_str());
	//g_pEngine->GetNetChannelInfo()->SendNetMsg(convar);
	static ConVar* nameCvar = g_pCvar->FindVar("name");
	*(int*)((DWORD)&nameCvar->m_fnChangeCallbacksV1+ 0xC) = 0;
	if (nameCvar)
	{
		nameCvar->SetValue(name.c_str());
	}
}
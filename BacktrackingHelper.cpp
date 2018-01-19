#include "sdk.h"
#include "BacktrackingHelper.h"
#include "global.h"
#include "xor.h"
#include "Math.h"
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
#define TICK_INTERVAL			( g_pGlobals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( floorf(( 0.5f + (float)(dt) / TICK_INTERVAL ) ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )
CBacktrackHelper* g_BacktrackHelper = new CBacktrackHelper;

int CBacktrackHelper::GetLatencyTicks()
{
	double v0; // st7@0
	INetChannelInfo* v1; // esi@1
	INetChannelInfo* v2; // eax@1
	float v3; // ST08_4@1
	float v4; // ST0C_4@1

	v1 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();
	v2 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();

	v3 = v1->GetAvgLatency(Typetype_t::TYPE_LOCALPLAYER);
	v4 = v2->GetAvgLatency(Typetype_t::TYPE_GENERIC);

	float interval_per_tick = 1.0f / g_pGlobals->interval_per_tick;

	return floorf(((v3 + v4) * interval_per_tick) + 0.5f);
}

float CBacktrackHelper::GetEstimateServerTime()
{
	double v0; // st7@0
	INetChannelInfo* v1; // esi@1
	INetChannelInfo* v2; // eax@1
	float v3; // ST08_4@1
	float v4; // ST0C_4@1

	v1 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();
	v2 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();

	v3 = v1->GetAvgLatency(Typetype_t::TYPE_LOCALPLAYER);
	v4 = v2->GetAvgLatency(Typetype_t::TYPE_GENERIC);

	//return floorf(((v3 + v4) / g_pGlobals->interval_per_tick) + 0.5f) + 1 + G::UserCmdForBacktracking->tick_count;*/

	return v3 + v4 + TICKS_TO_TIME(1) + TICKS_TO_TIME(G::UserCmdForBacktracking->tick_count);

}
float CBacktrackHelper::GetNetworkLatency()
{
	// Get true latency
	INetChannelInfo *nci = g_pEngine->GetNetChannelInfo();
	if (nci)
	{
		//float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING); //ppl say use only this one, but meh
		float OutgoingLatency = nci->GetLatency(0);
		return OutgoingLatency;
	}
	return 0.0f;
}
ConVar* minupdate;
ConVar* maxupdate;
ConVar * updaterate;
ConVar * interprate;
ConVar* cmin;
ConVar* cmax;
ConVar* interp;
float CBacktrackHelper::GetLerpTime()
{
	if (!minupdate)
		minupdate = g_pCvar->FindVar(XorStr("sv_minupdaterate"));
	if (!maxupdate)
		maxupdate = g_pCvar->FindVar(XorStr("sv_maxupdaterate"));
	if (!updaterate)
		updaterate = g_pCvar->FindVar(XorStr("cl_updaterate"));
	if (!interprate)
		interprate = g_pCvar->FindVar(XorStr("cl_interp_ratio"));
	if (!cmin)
		cmin = g_pCvar->FindVar(XorStr("sv_client_min_interp_ratio"));
	if (!cmax)
		cmax = g_pCvar->FindVar(XorStr("sv_client_max_interp_ratio"));
	if (!interp)
		interp = g_pCvar->FindVar(XorStr("cl_interp"));

	float UpdateRate = updaterate->GetValue();
	float LerpRatio = interprate->GetValue();

	return max(LerpRatio / UpdateRate, interp->GetValue());
}

void CBacktrackHelper::UpdateBacktrackRecords(CBaseEntity* pPlayer)
{
	int i = pPlayer->Index();

	for (int j = g_BacktrackHelper->PlayerRecord[i].records.size() - 1; j >= 0; j--)
	{
		float lerptime = g_BacktrackHelper->GetLerpTime();
		float desired_time = g_BacktrackHelper->PlayerRecord[i].records.at(j).m_flSimulationTime + lerptime;
		float estimated_time = g_BacktrackHelper->GetEstimateServerTime();
		float SV_MAXUNLAG = 1.0f;
		float latency = g_BacktrackHelper->GetNetworkLatency();
		float m_flLerpTime = g_BacktrackHelper->GetLerpTime();
		float correct = clamp<float>(latency + m_flLerpTime, 0.0f, SV_MAXUNLAG);
		float deltaTime = correct - (estimated_time + lerptime - desired_time);

		if (fabs(deltaTime) > 0.2f)
			g_BacktrackHelper->PlayerRecord[i].records.erase(g_BacktrackHelper->PlayerRecord[i].records.begin() + j);
	}

	static Vector old_origin[64];

	if (PlayerRecord[i].records.size() > 0 && pPlayer->GetSimulationTime() == PlayerRecord[i].records.back().m_flSimulationTime) //already got such a record
		return;

	if (PlayerRecord[i].records.size() > 0 && PlayerRecord[i].records.back().m_flSimulationTime > pPlayer->GetSimulationTime())//Invalid lag record, maybe from diffrent game?
	{
		PlayerRecord[i].records.clear();
		return;
	}

	Vector cur_origin = pPlayer->GetOrigin();
	Vector v = cur_origin - old_origin[i];
	bool breaks_lagcomp = v.LengthSqr() > 4096.f;
	old_origin[i] = cur_origin;
	tick_record new_record;

	new_record.needs_extrapolation = breaks_lagcomp;
	static float OldLower[64];

	PlayerRecord[i].LowerBodyYawTarget = pPlayer->LowerBodyYaw();
	new_record.m_angEyeAngles = pPlayer->GetEyeAngles();
	new_record.m_flCycle = pPlayer->GetCycle();
	new_record.m_flSimulationTime = pPlayer->GetSimulationTime();
	new_record.m_flAnimTime = pPlayer->GetAnimationTime();
	new_record.bLowerBodyYawUpdated = false;
	new_record.m_nSequence = pPlayer->GetSequence();
	new_record.m_vecOrigin = pPlayer->GetOrigin();
	new_record.m_vecVelocity = pPlayer->GetVelocity();
	new_record.m_flUpdateTime = g_pGlobals->curtime;
	new_record.backtrack_time = new_record.m_flSimulationTime + GetLerpTime();

	if (PlayerRecord[i].LowerBodyYawTarget != OldLower[i] || (*pPlayer->GetFlags() & FL_ONGROUND && pPlayer->GetVelocity().Length() > 29.f))
		new_record.bLowerBodyYawUpdated = true;

	for (int i = 0; i < 24; i++)
		new_record.m_flPoseParameter[i] = *(float*)((DWORD)pPlayer + offys.m_flPoseParameter + sizeof(float) * i);

	pPlayer->SetupBones(new_record.boneMatrix, 128, 0x00000100, g_pGlobals->curtime);

	OldLower[i] = PlayerRecord[i].LowerBodyYawTarget;
	PlayerRecord[i].records.push_back(new_record);
}
void CBacktrackHelper::UpdateExtrapolationRecords(CBaseEntity* pPlayer)
{
	int index = pPlayer->Index();
	if (pPlayer->GetSimulationTime() == this->SimRecord[index][0].simulation_time)
		return;

	for (int i = 7; i > 0; i--)
	{
		this->SimRecord[index][i].acceleration = this->SimRecord[index][i - 1].acceleration;
		this->SimRecord[index][i].origin = this->SimRecord[index][i - 1].origin;
		this->SimRecord[index][i].simulation_time = this->SimRecord[index][i - 1].simulation_time;
		this->SimRecord[index][i].update_time = this->SimRecord[index][i - 1].update_time;
		this->SimRecord[index][i].velocity = this->SimRecord[index][i - 1].velocity;
	}

	this->SimRecord[index][0].simulation_time = pPlayer->GetSimulationTime();
	this->SimRecord[index][0].update_time = g_pGlobals->curtime;
	this->SimRecord[index][0].origin = pPlayer->GetOrigin();

	int lost_ticks = TIME_TO_TICKS(this->SimRecord[index][0].simulation_time) - TIME_TO_TICKS(this->SimRecord[index][1].simulation_time);

	this->SimRecord[index][0].simulation_time_increasment_per_tick = (this->SimRecord[index][0].simulation_time - this->SimRecord[index][1].simulation_time) / lost_ticks;

	/*calculate velocity by ourselves*/
	Vector velocity = this->SimRecord[index][0].origin - this->SimRecord[index][1].origin;
	/*divide through lost ticks to get the velocity per tick*/
	velocity /= lost_ticks;

	this->SimRecord[index][0].velocity = pPlayer->GetVelocity();//velocity;
}

void CBacktrackHelper::ExtrapolatePosition(CBaseEntity* pPlayer, Vector& position, float &simtime, Vector velocity)
{
	int index = pPlayer->GetIndex();
	simulation_record latest_record = this->SimRecord[index][0];
	simulation_record pre_latest_record = this->SimRecord[index][1];
	int latency_ticks = GetLatencyTicks();
	Vector vel = velocity;
	float s_time = simtime;
	INetChannelInfo* nci = g_pEngine->GetNetChannelInfo();
	float latency = nci->GetAvgLatency(0) + nci->GetAvgLatency(1);
	float ticks_per_second = 1.0f / g_pGlobals->interval_per_tick;
	float server_time = ((floorf(((latency)*ticks_per_second) + 0.5) + G::UserCmdForBacktracking->tick_count + 1) * g_pGlobals->interval_per_tick);
	float server_time_difference = server_time - s_time;

	if (server_time_difference > 1.0f)
		server_time_difference = 1.0f;

	float sim_time_difference = g_pGlobals->curtime - latest_record.update_time;
	//proper method
	int ticks_choked = /*TIME_TO_TICKS(latest_record.simulation_time - pre_latest_record.simulation_time_increasment_per_tick);*/TIME_TO_TICKS(sim_time_difference);

	if (ticks_choked <= 15)
	{
		latency_ticks = 1;
		if (ticks_choked < 1)
			ticks_choked = 1;
	}
	else
		ticks_choked = 15;

	int total_difference = floorf((server_time_difference * ticks_per_second) + 0.5) - ticks_choked;

	vel.z -= g_pCvar->FindVar("sv_gravity")->GetValue() * g_pGlobals->interval_per_tick;

	Ray_t ray;
	trace_t tr;
	CTraceFilterWorldAndPropsOnly filter;

	if (total_difference < 0)
	{
		return;
	}
	else
	{
		do
		{
			int chokes = ticks_choked;

			//test, aw has that do statement
			do
			{

				ray.Init(position, position + (vel* g_pGlobals->interval_per_tick), G::LocalPlayer->GetCollision()->VecMins(), G::LocalPlayer->GetCollision()->VecMaxs());

				g_pEngineTrace->TraceRay_NEW(ray, MASK_SOLID, &filter, &tr);

				if (tr.fraction > 0)
				{
					position += vel * (g_pGlobals->interval_per_tick);
					simtime += g_pGlobals->interval_per_tick;
				}
				else
				{

					return;
				}
				/*if (tr.DidHitWorld())
				{
				position = tr.endpos;
				if (position.z < tr.endpos.z)
				vel.z *= -1;
				else
				{

				chokes = 0;
				total_difference = 0;
				break;
				}
				}*/

				/*CUserCmd* cmd = G::UserCmdForBacktracking;
				Math::VectorAngles(pPlayer->GetVelocity(), cmd->viewangles);
				cmd->aimdirection = cmd->viewangles;
				cmd->buttons = 0;
				cmd->command_number = G::UserCmdForBacktracking->command_number;
				cmd->forwardmove = 400;
				cmd->sidemove = 0;
				cmd->hasbeenpredicted = false;
				cmd->impulse = G::UserCmdForBacktracking->impulse;
				cmd->mousedx = 0;
				cmd->mousedy = 0;
				cmd->random_seed = G::UserCmdForBacktracking->random_seed;
				cmd->tick_count = TIME_TO_TICKS(pPlayer->GetSimulationTime());
				cmd->upmove = 0;
				cmd->weaponselect = 0;
				cmd->weaponsubtype = 0;

				g_pPrediction->RunCommand(pPlayer, cmd, g_pMoveHelper);
				*/
				chokes--;
				/*vel.z -= (g_pCvar->FindVar("sv_gravity")->GetValue() * 0.5f * g_pGlobals->interval_per_tick);
				vel.z += (vel.z * g_pGlobals->interval_per_tick);

				float fmove = 400;
				float smove = 0;

				QAngle angle = QAngle();
				Math::VectorAngles(vel, angle);

				Vector forward = Vector();
				Vector right = Vector();
				Vector up = Vector();

				Math::AngleVectors(angle, &forward, &right, &up);

				forward.z = 0;
				right.z = 0;

				forward.NormalizeInPlace();
				right.NormalizeInPlace();

				Vector wishvel = forward * fmove + right * smove;

				wishvel.z = 0;

				Vector wishdir = wishvel;

				float wishspeed = wishdir.Length();

				if (wishspeed != 0 && (wishspeed > pPlayer->GetMaxSpeed()))
				{
				wishvel *= (pPlayer->GetMaxSpeed() / wishspeed);
				wishspeed = pPlayer->GetMaxSpeed();
				}

				AirAccelerate(pPlayer, wishdir, wishspeed, g_pCvar->FindVar("sv_airaccelerate")->GetValue(), vel);*/
			} while (chokes);

			total_difference -= ticks_choked;
		} while (total_difference >= 0);
	}
	//position.z = z_origin;
	//By notwav
	/*
	float TimeTotal =  nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING) + EnemyPing + FudgeFactor;

	for ( ; TotalTotal >= MinimumTime; TimeTotal -= nci->GetAvgLatency(FLOW_OUTGOING) )*/



	/*
	if (!G::UserCmdForBacktracking)
	return;

	CUserCmd* test = G::UserCmdForBacktracking;

	QAngle angle;
	SpreadVectorAngles(pPlayer->GetVelocity(), angle);

	test->upmove = 0;
	test->forwardmove = 400;
	test->sidemove = 0;
	test->aimdirection = angle;
	test->viewangles = angle;
	test->buttons = NULL;
	test->hasbeenpredicted = false;
	test->tick_count = TIME_TO_TICKS(pPlayer->GetSimulationTime());


	//g_pMoveHelper->SetHost(pPlayer);


	for (int i = ticks_choked; i >= 0; i--)
	{
	//ghetto way
	//position += vel;
	//simtime += sim_record.simulation_time_increasment_per_tick;
	//
	//g_pPrediction->RunCommand(pPlayer, test, g_pMoveHelper);
	CMoveData m_MoveData;
	memset(&m_MoveData, 0, sizeof(m_MoveData));
	g_pMoveHelper->SetHost(pPlayer);
	g_pPrediction->SetupMove(pPlayer, test, g_pMoveHelper, &m_MoveData);
	g_pGameMovement->ProcessMovement(pPlayer, &m_MoveData);
	g_pPrediction->FinishMove(pPlayer, test, &m_MoveData);
	}
	position = pPlayer->GetOrigin();
	simtime = pPlayer->GetSimulationTime();*/


	/*float gravity = (g_pCvar->FindVar("sv_gravity")->GetValue() * 0.5f * g_pGlobals->interval_per_tick);
	float basevelocity = (pPlayer->GetBaseVelocity().z * g_pGlobals->interval_per_tick);

	Vector vel = velocity;  vel.z -= gravity; vel.z += basevelocity;

	vel *= g_pGlobals->interval_per_tick;

	Vector old_origin, old_velocity,  acceleration;

	for (int i = ticks_choked; i > 0; i--)
	{
	old_origin = position;
	old_velocity = vel;

	//should be using that tbh
	//vel += acceleration;

	position += vel;

	//vel = position - old_origin;

	acceleration = vel - old_velocity;
	}*/
}
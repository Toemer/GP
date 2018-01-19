#include "sdk.h"
#include "Antiaim.h"
#include "global.h"
#include "GameUtils.h"
#include "Math.h"
#include "Aimbot.h"

CAntiaim* g_Antiaim = new CAntiaim;

float get3ddist2(Vector myCoords, Vector enemyCoords)
{
	return sqrt(
		pow(double(enemyCoords.x - myCoords.x), 2.0) +
		pow(double(enemyCoords.y - myCoords.y), 2.0) +
		pow(double(enemyCoords.z - myCoords.z), 2.0));
}

void AimAtPlayers()
{
	int value = G::UserCmd->viewangles.y, num = 0;
	float best_dist = G::MainWeapon->GetCSWpnData()->flRange;
	for (int i = 1; i <= g_pGlobals->maxClients; ++i) // Ranges between 1-64, 0 is world entity and (>64) throws access violation.
	{
		if (i == g_pEngine->GetLocalPlayer())
			continue;

		CBaseEntity* pTarget = g_pEntitylist->GetClientEntity(i);

		//Calls from left->right so we wont get an access violation error
		Vector pos;
		if (!pTarget || pTarget->GetHealth() < 1)
			continue;
		if (G::LocalPlayer->GetTeamNum() != pTarget->GetTeamNum())
		{
			pos = pTarget->GetEyePosition();
			float dist = get3ddist2(pos, G::LocalPlayer->GetEyePosition());
			if (Clientvariables->Antiaim.AtPlayer == 1)
			{
				if (dist >= best_dist)
					continue;
			}
			best_dist = dist;

			QAngle angle = GameUtils::CalculateAngle(G::LocalPlayer->GetEyePosition(), pos);
			angle.y = Math::NormalizeYaw(angle.y);
			Math::ClampAngles(angle);

			if (Clientvariables->Antiaim.AtPlayer == 1)
				value = angle.y;
			else
			{
				num++;
				value += angle.y;
			}
		}
	}
	if (Clientvariables->Antiaim.AtPlayer == 2 && num != 0)
		value /= num;

	G::UserCmd->viewangles.y = value;
}

void CAntiaim::Pitch_AA()
{
	float temp = 0;

	switch (Clientvariables->Antiaim.Pitch)
	{
	case 1:
		temp = 179.f;
		break;
	case 2:
		temp = 88.9f;
		break;
	case 3:
		temp = 180.17f;
		break;
	case 4:
		temp = 271.f;
		break;
	case 5:
		temp = 1080.f; // Overflow Down
		break;
	case 6:
		temp = 1089.f; // Overflow More Down ????????????????
		break;
	case 7:
		temp = 132.f;
		break;
	default:
		break;
	}
	G::UserCmd->viewangles.x = temp;
}

void CAntiaim::Yaw_AA()
{
	float temp = G::UserCmd->viewangles.y;

	int Type = Clientvariables->Antiaim.Yaw;

	int random = rand() % 100;

	static bool swap;
	if (G::SendPacket)
		swap = !swap;

	if (Type == 1)
	{
		if (G::SendPacket)
			temp += 90.f;
		else
			if (swap)
				temp -= 180.f;
			else
				temp -= 90.f;
	}
	else if (Type == 2)
	{
		if (G::SendPacket)
			if (swap)
				temp += 90.f;
			else
				temp -= 90.f;
		else
		{

			if (G::UserCmd->command_number % 3)
				temp -= 180.f;
			else
			{
				if (swap)
					temp -= 165;
				else
					temp += 165;
			}
		}
	}
	else if (Type == 3)
	{
		if (G::SendPacket)
			if (swap)
				temp -= 165;
			else
				temp += 165;
		else
			if (swap)
				temp += 165;
			else
				temp -= 165;
	}
	else if (Type == 4)
	{
		static int step = 0;
		if (G::SendPacket)
			step += 1;
		if (step > 3)
			step = 0;

		static int currentstepyaw;

		if (G::SendPacket)
		{
			switch (step)
			{
			case 0:
				temp = 45;
				break;
			case 1:
				temp = 225;
				break;
			case 2:
				temp = 315;
				break;
			case 3:
				temp = 135;
				break;
			default:
				break;
			}
			currentstepyaw = temp;
		}
		else
		{
			static bool intern_swap;
			intern_swap = !intern_swap;
			if (G::UserCmd->command_number % 3)
				temp -= 180.f;
			else
			{
				if (intern_swap)
					temp = currentstepyaw + 90.f;
				else
					temp = currentstepyaw - 90.f;
			}
		}
	}
	else if (Type == 5)
	{
		temp = G::LocalPlayer->LowerBodyYaw() + 180;
	}
	G::UserCmd->viewangles.y = temp;
}

float BestHeadDirection(float yaw)
{
	for (int i = 0; i < g_pEngine->GetMaxClients(); i++)
	{
		CBaseEntity* player = g_pEntitylist->GetClientEntity(i);

		if (player->IsValidTarget())
		{
			QAngle CalculatedAngle = GameUtils::CalculateAngle(G::LocalPlayer->GetEyePosition(), player->GetOrigin()); // no
			float Atangle = CalculatedAngle.y;

			Vector Difference = G::LocalPlayer->GetOrigin() - player->GetOrigin();

			if (Difference.y > 0)
				return (Atangle - 90.f);
			else if (Difference.y < 0)
				return (Atangle + 90.f);
		}
	}
	return 0;
}

float GetBestHeadAngle(float yaw)
{
	float Back, Right, Left;

	Vector src3D, dst3D, forward, right, up, src, dst;
	trace_t tr;
	Ray_t ray, ray2, ray3, ray4, ray5;
	CTraceFilter filter;

	QAngle engineViewAngles;
	g_pEngine->GetViewAngles(engineViewAngles);

	engineViewAngles.x = 0;

	Math::AngleVectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = G::LocalPlayer;
	src3D = G::LocalPlayer->GetEyePosition();
	dst3D = src3D + (forward * 384);

	ray.Init(src3D, dst3D);

	g_pEngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	Back = (tr.endpos - tr.start).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);

	g_pEngineTrace->TraceRay(ray2, MASK_SHOT, &filter, &tr);

	Right = (tr.endpos - tr.start).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);

	g_pEngineTrace->TraceRay(ray3, MASK_SHOT, &filter, &tr);

	Left = (tr.endpos - tr.start).Length();

	if (Left > Right)
	{
		return (yaw - 90);
	}
	else if (Right > Left)
	{
		return (yaw + 90);
	}
	else if (Back > Right || Back > Left)
	{
		return (yaw - 180);
	}
	return 0;
}

bool ShouldPredict()
{
	INetChannelInfo* nci = g_pEngine->GetNetChannelInfo();

	float server_time = g_pGlobals->curtime + nci->GetLatency(FLOW_OUTGOING);

	static bool initialized;
	bool will_update = false;

	if (!initialized && !G::LocalPlayer->IsMoving())
	{
		initialized = true;
		G::PredictedTime = server_time + 0.22f;
	}
	else if (G::LocalPlayer->IsMoving())
	{
		initialized = false;
	}

	if (server_time >= (G::PredictedTime) && *G::LocalPlayer->GetFlags() & FL_ONGROUND)
	{
		G::PredictedTime = server_time + 1.1f;
		will_update = true;
	}
	return will_update;
}

void BreakLowerbodyFreestand()
{
	QAngle Angles;
	g_pEngine->GetViewAngles(Angles);
	float BestHeadPosition = GetBestHeadAngle(Angles.y);

	int LowerbodyDelta = Clientvariables->Antiaim.FreestandingDelta;

	if (G::SendPacket)
	{
		G::UserCmd->viewangles.y = BestHeadPosition + LowerbodyDelta + Math::RandomFloat2(-65.f, 65.f);
	}
	else
	{
		if (ShouldPredict())
			G::UserCmd->viewangles.y = BestHeadPosition + LowerbodyDelta;
		else
			G::UserCmd->viewangles.y = BestHeadPosition;
	}
}

void CAntiaim::AntiAimYaw_Real()
{
	if (G::LocalPlayer->GetVelocity().Length() > 0.1f)
	{
		float temp = G::UserCmd->viewangles.y;

		static float add = 0;
		static bool reset = true;
		static int Ticks = 0;
		float temp_base = temp;

		switch (Clientvariables->Antiaim.YawRunning)
		{
		case 1:
			temp -= 180.f;
			break;

		case 2:
			temp -= 180.f + Math::RandomFloat2(-25.f, 25.f); // Lowerbody updates when the delta goes above 35.f
			break;

		case 3:
			if (reset)
			{
				add = 0;
				reset = false;
			}

			temp += 135;
			temp += add;
			add += 15;

			if (temp_base + 225 < temp)
			{
				reset = true;
				temp = temp_base + 225;
			}
			break;

		case 4:
			temp -= Ticks; // 180z using ticks
			Ticks += 2;

			if (Ticks > 240)
				Ticks = 120;
			break;

		case 5:
			temp -= Ticks; // Spin using ticks
			Ticks += 5;

			if (Ticks > 360)
				Ticks = 0;
			break;

		case 6:
			temp -= Math::RandomFloat2(-180.f, 180.f);
			break;

		default:
			break;
		}
		G::UserCmd->viewangles.y = temp + Clientvariables->Antiaim.YawRunningAdd;
	}
	else
	{
		float temp = G::UserCmd->viewangles.y;

		static float add = 0;
		static bool reset = true;
		static int Ticks = 120;
		float temp_base = temp;

		switch (Clientvariables->Antiaim.Yaw)
		{
		case 1:
			temp -= 180.f;
			break;

		case 2:
			temp -= 180.f + Math::RandomFloat2(-25.f, 25.f); // Lowerbody updates when the delta goes above 35.f
			break;

		case 3:
			if (reset)
			{
				add = 0;
				reset = false;
			}

			temp += 135;
			temp += add;
			add += 15;

			if (temp_base + 225 < temp)
			{
				reset = true;
				temp = temp_base + 225;
			}
			break;

		case 4:
			temp -= Ticks; // 180z using ticks
			Ticks += 2;

			if (Ticks > 240)
				Ticks = 120;
			break;

		case 5:
			temp -= Ticks; // Spin using ticks
			Ticks += 5;

			if (Ticks > 360)
				Ticks = 0;
			break;

		case 6:
			temp -= Math::RandomFloat2(-180.f, 180.f);
			break;

		default:
			break;
		}
		G::UserCmd->viewangles.y = temp + Clientvariables->Antiaim.YawAdd;
	}
}

void CAntiaim::AntiAimYaw_Fake()
{
	float temp = G::UserCmd->viewangles.y;
	static int Ticks;
	static int Side;

	switch (Clientvariables->Antiaim.FakeYaw)
	{
	case 1:
		temp -= 180.f;
		break;

	case 2:
		temp -= G::RealAngle.y - G::RealAngle.y;
		break;

	case 3:
		temp -= 0.f;
		break;

	case 4:
		temp -= Ticks;
		Ticks += 2;

		if (Ticks > 360)
			Ticks = 0;
		break;

	case 5:
		temp -= Ticks; // 180z using ticks // Update: this doesn't use ticks, he just fucking sets yaw to a number which is calculated by intervals.
		Ticks += 2;

		if (Ticks > 240)
			Ticks = 120;
		break;

	case 6:
		temp -= Math::RandomFloat2(-180.f, 180.f);
		break;

	default:
		break;
	}
	G::UserCmd->viewangles.y = temp + Clientvariables->Antiaim.FakeYawAdd;
}

bool GetAlive(CBaseEntity* pLocal)
{
	for (int i = 1; i < 64; ++i)
	{
		if (i == g_pEngine->GetLocalPlayer())
			continue;

		CBaseEntity* target = g_pEntitylist->GetClientEntity(i);
		player_info_t info;
		//Calls from left->right so we wont get an access violation error
		Vector pos;
		if (!target || target->GetHealth() < 1)
			continue;
		if (pLocal->GetTeamNum() != target->GetTeamNum())
		{
			return true;
		}
	}
	return false;
}

void CAntiaim::Run(QAngle org_view)
{
	if (Clientvariables->Antiaim.AntiaimEnable)
	{
		static int iChokedPackets = -1;

		if ((g_Aimbot->fired_in_that_tick && iChokedPackets < 4 && GameUtils::IsAbleToShoot()) && !G::ForceRealAA)
		{
			G::SendPacket = false;
			iChokedPackets++;
		}
		else
		{
			iChokedPackets = 0;

			CGrenade* pCSGrenade = (CGrenade*) G::LocalPlayer->GetWeapon();

			if (G::UserCmd->buttons & IN_USE
				|| !GetAlive(G::LocalPlayer) && !Clientvariables->Misc.AntiUT
				|| G::LocalPlayer->GetMoveType() == MOVETYPE_LADDER && G::LocalPlayer->GetVelocity().Length() > 0
				|| G::LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP
				|| pCSGrenade && pCSGrenade->GetThrowTime() > 0.f)
				return;

			choke = !choke;
			if (!Clientvariables->Misc.FakelagEnable || (*G::LocalPlayer->GetFlags() & FL_ONGROUND && !Clientvariables->Misc.FakelagOnground || *G::LocalPlayer->GetFlags() & FL_ONGROUND && G::LocalPlayer->GetVelocity().Length() < 0.1f))
				G::SendPacket = choke;

			G::UserCmd->viewangles = org_view; //fixes aimbot angles

			if (Clientvariables->Antiaim.AtPlayer > 0)
				AimAtPlayers();

			Fakewalk(G::UserCmd);

			if (G::bShouldChoke)
				G::SendPacket = G::bShouldChoke = false;

			if (!G::SendPacket)
				G::nChokedTicks++;
			else
				G::nChokedTicks = 0;

			if (Clientvariables->Antiaim.Freestanding > 0 && G::LocalPlayer->GetVelocity().Length() <= 75.f && *G::LocalPlayer->GetFlags() & FL_ONGROUND) // freestand
			{
				Pitch_AA();
				BreakLowerbodyFreestand();
			}
			else
			{
				Pitch_AA();
				if (G::SendPacket)
					AntiAimYaw_Fake();
				else
				{
					AntiAimYaw_Real();
				}
			}
		}
	}
}

void Accelerate(CBaseEntity *player, Vector &wishdir, float wishspeed, float accel, Vector &outVel)
{
	// See if we are changing direction a bit
	float currentspeed = outVel.Dot(wishdir);

	// Reduce wishspeed by the amount of veer.
	float addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if (addspeed <= 0)
		return;

	// Determine amount of accleration.
	float accelspeed = accel * g_pGlobals->frametime * wishspeed * player->m_surfaceFriction();

	// Cap at addspeed
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust velocity.
	for (int i = 0; i < 3; i++)
		outVel[i] += accelspeed * wishdir[i];
}

void WalkMove(CBaseEntity *player, Vector &outVel)
{
	Vector forward, right, up, wishvel, wishdir, dest;
	float_t fmove, smove, wishspeed;

	Math::AngleVectors(player->GetEyeAngles(), forward, right, up);  // Determine movement angles

	g_pMoveHelper->SetHost(player);
	fmove = g_pMoveHelper->m_flForwardMove;
	smove = g_pMoveHelper->m_flSideMove;
	g_pMoveHelper->SetHost((CBaseEntity* )nullptr);

	if (forward[2] != 0)
	{
		forward[2] = 0;
		Math::NormalizeVector(forward);
	}

	if (right[2] != 0)
	{
		right[2] = 0;
		Math::NormalizeVector(right);
	}

	for (int i = 0; i < 2; i++)	// Determine x and y parts of velocity
		wishvel[i] = forward[i] * fmove + right[i] * smove;

	wishvel[2] = 0;	// Zero out z part of velocity

	wishdir = wishvel; // Determine maginitude of speed of move
	wishspeed = wishdir.Normalize();

	// Clamp to server defined max speed
	g_pMoveHelper->SetHost(player);
	if ((wishspeed != 0.0f) && (wishspeed > g_pMoveHelper->m_flMaxSpeed))
	{
		VectorMultiply(wishvel, player->m_flMaxspeed() / wishspeed, wishvel);
		wishspeed = player->m_flMaxspeed();
	}
	g_pMoveHelper->SetHost(nullptr);
	// Set pmove velocity
	outVel[2] = 0;
	Accelerate(player, wishdir, wishspeed, g_pCvar->FindVar("sv_accelerate")->GetFloat(), outVel); // Always have to have the biggest dynamic variable searching ever.
	outVel[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd(outVel, player->GetBaseVelocity(), outVel);

	float spd = outVel.Length();

	if (spd < 1.0f)
	{
		outVel.Init();
		// Now pull the base velocity back out. Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(outVel, player->GetBaseVelocity(), outVel);
		return;
	}

	g_pMoveHelper->SetHost(player);
	g_pMoveHelper->m_outWishVel += wishdir * wishspeed;
	g_pMoveHelper->SetHost(nullptr);

	// Don't walk up stairs if not on ground.
	if (!(*player->GetFlags() & FL_ONGROUND))
	{
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(outVel, player->GetBaseVelocity(), outVel);
		return;
	}

	// Now pull the base velocity back out. Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract(outVel, player->GetBaseVelocity(), outVel);
}

void CAntiaim::Fakewalk(CUserCmd *userCMD)
{
	if (!Clientvariables->Misc.FakelagEnable || !GetKeyState(Clientvariables->Misc.FakewalkKey))
		return;

	Vector velocity = G::vecUnpredictedVel;

	int Iterations = 0;
	for (; Iterations < 15; ++Iterations) {
		if (velocity.Length() < 0.1)
		{
			//g_pCvar->ConsolePrintf("Ticks till stop %d\n", Iterations);
			Msg("Ticks till stop %d\n", Iterations);
			break;
		}

		Friction(velocity);
		WalkMove(G::LocalPlayer, velocity);
	}

	int choked_ticks = G::nChokedTicks;

	if (Iterations > 7 - choked_ticks || !choked_ticks)
	{
		float_t speed = velocity.Length();

		QAngle direction;
		Math::VectorAngles(velocity, direction);

		direction.y = userCMD->viewangles.y - direction.y;

		Vector forward;
		Math::AngleVectors(direction, forward);
		Vector negated_direction = forward * -speed;

		userCMD->forwardmove = negated_direction.x;
		userCMD->sidemove = negated_direction.y;
	}

	if (G::nChokedTicks < 7)
		G::bShouldChoke = true;
}

void CAntiaim::Friction(Vector &outVel)
{
	float speed, newspeed, control;
	float friction;
	float drop;

	speed = outVel.Length();

	if (speed <= 0.1f)
		return;

	drop = 0;

	// apply ground friction
	if (*G::LocalPlayer->GetFlags() & FL_ONGROUND)
	{
		friction = g_pCvar->FindVar("sv_friction")->GetFloat() * G::LocalPlayer->m_surfaceFriction();

		// Bleed off some speed, but if we have less than the bleed
		// threshold, bleed the threshold amount.
		control = (speed < g_pCvar->FindVar("sv_stopspeed")->GetFloat()) ? g_pCvar->FindVar("sv_stopspeed")->GetFloat() : speed;

		// Add the amount to the drop amount.
		drop += control * friction * g_pGlobals->frametime;
	}

	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	if (newspeed != speed)
	{
		// Determine proportion of old speed we are using.
		newspeed /= speed;
		// Adjust velocity according to proportion.
		VectorMultiply(outVel, newspeed, outVel);
	}
}
#include "sdk.h"
#include "Aimbot.h"
#include "global.h"
#include "Menu.h"
#include "Math.h"
#include "GameUtils.h"
#include "Autowall.h"
#include "BacktrackingHelper.h"

#define TICK_INTERVAL			( g_pGlobals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )

template<class T, class U>
inline T clamp( T in, U low, U high )
{
    if( in <= low )
        return low;
    else if( in >= high )
        return high;
    else
        return in;
}

bool CAimbot::HitChance(CBaseEntity* pCSTarget, CBaseCombatWeapon* pCSWeapon, QAngle qAngle, int chance) {

	if (!chance)
		return true;

	int iHit = 0;
	int iHitsNeed = (int)((float)256.f * ((float)chance / 100.f));
	bool bHitchance = false;

	Vector forward, right, up;
	Math::AngleVectors(qAngle, &forward, &right, &up);

	pCSWeapon->UpdateAccuracyPenalty();

	for (auto i = 0; i < 256; ++i) {

		float RandomA = Math::RandomFloat2(0.0f, 1.0f);
		float RandomB = 1.0f - RandomA * RandomA;
		RandomA = Math::RandomFloat2(0.0f, M_PI_F * 2.0f);
		RandomB *= pCSWeapon->GetSpread() + pCSWeapon->GetInaccuracy();
		float SpreadX1 = (cos(RandomA) * RandomB);
		float SpreadY1 = (sin(RandomA) * RandomB);
		float RandomC = Math::RandomFloat2(0.0f, 1.0f);
		float RandomF = RandomF = 1.0f - RandomC * RandomC;
		RandomC = Math::RandomFloat2(0.0f, M_PI_F * 2.0f);
		RandomF *= pCSWeapon->GetSpread();
		float SpreadX2 = (cos(RandomC) * RandomF);
		float SpreadY2 = (sin(RandomC) * RandomF);
		float fSpreadX = SpreadX1 + SpreadX2;
		float fSpreadY = SpreadY1 + SpreadY2;

		Vector vSpreadForward;
		vSpreadForward[0] = forward[0] + (fSpreadX * right[0]) + (fSpreadY * up[0]);
		vSpreadForward[1] = forward[1] + (fSpreadX * right[1]) + (fSpreadY * up[1]);
		vSpreadForward[2] = forward[2] + (fSpreadX * right[2]) + (fSpreadY * up[2]);
		vSpreadForward.NormalizeInPlace();

		QAngle qaNewAngle;
		Math::VectorAngles(vSpreadForward, qaNewAngle);
		qaNewAngle.NormalizeVec();

		QAngle vEnd;
		Math::AngleVectors(qaNewAngle, &vEnd);
		vEnd = G::LocalPlayer->GetEyePosition() + (vEnd * 8192.f);

		if (G::LocalPlayer->canHit(vEnd, pCSTarget))
			iHit++;

		if ((int)(((float)iHit / 256.f) * 100.f) >= chance) {
			bHitchance = true;
			break;
		}
		if ((256.f - 1 - i + iHit) < iHitsNeed)
			break;
	}
	return bHitchance;
}

std::vector<Vector> MainHitbox(CBaseEntity* pTarget, VMatrix BoneMatrix[128])
{
	std::vector<Vector> Points;
	int Aimspot = Clientvariables->Ragebot.Hitbox;

	switch (Aimspot) {
	case 0:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 0, BoneMatrix);
		break;
	case 1:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 1, BoneMatrix);
		break;
	case 2:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 6, BoneMatrix);
		break;
	case 3:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 3, BoneMatrix);//pelvis
		break;
	case 4:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 4, BoneMatrix);//stomach
		break;
	case 5:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 0, BoneMatrix);
		break;
	}
	return Points;
}

std::vector<int> GetHitboxesToScan( CBaseEntity* pTarget )
{
    std::vector<int> HitBoxesToScan;
    int HitScanMode = Clientvariables->Ragebot.Hitscan;
    int Aimspot = Clientvariables->Ragebot.AimbotSelection;

    switch( HitScanMode ) {
    case 0:
        break;
    case 1:
        // head/body
        if( Aimspot != 0 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Head );
        else if( Aimspot != 1 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Neck );
        else if( Aimspot != 2 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::UpperChest );
        else if( Aimspot != 3 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Pelvis );
        else if( Aimspot != 4 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Stomach );

        HitBoxesToScan.push_back( (int)CSGOHitboxID::Chest );
        break;
    case 2:
        // basic +(arms, thighs)
        if( Aimspot != 0 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Head );
        else if( Aimspot != 1 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Neck );
        else if( Aimspot != 2 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::UpperChest );
        else if( Aimspot != 3 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Pelvis );
        else if( Aimspot != 4 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Stomach );

        HitBoxesToScan.push_back( (int)CSGOHitboxID::Chest );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::LeftThigh );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::RightThigh );
    case 3:
        // heaps ++(just all the random shit)
        if( Aimspot != 0 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Head );
        else if( Aimspot != 1 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Neck );
        else if( Aimspot != 2 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::UpperChest );
        else if( Aimspot != 3 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Pelvis );
        else if( Aimspot != 4 )
            HitBoxesToScan.push_back( (int)CSGOHitboxID::Stomach );

        HitBoxesToScan.push_back( (int)CSGOHitboxID::Chest );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::LeftThigh );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::RightThigh );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::LeftFoot );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::RightFoot );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::LeftShin );
        HitBoxesToScan.push_back( (int)CSGOHitboxID::RightShin );
    }
    return HitBoxesToScan;
}

Vector RunAimScan( CBaseEntity* pTarget, float &simtime, Vector& origin )
{
    Vector vEyePos = G::LocalPlayer->GetEyePosition();
    auto wpn_data = G::WeaponData;
	static float minimum_damage = 1.f;

	if (Clientvariables->Ragebot.Mindamage == 101) // Enemy HP mode
		minimum_damage = pTarget->GetHealth();
	else
		minimum_damage = Clientvariables->Ragebot.Mindamage;

    bool found = false;

	VMatrix BoneMatrix[ 128 ];

	if (Clientvariables->Ragebot.PositionAdjustment)
	{
		std::vector<tick_record> lby_records, trash_records;

		/*fix for hitscan and multipoints only on the last records*/
		float latest_time = 0.0f;
		tick_record latest_record;

		//do it that way to use the latest nonfakelagging record
		for (int j = g_BacktrackHelper->PlayerRecord[pTarget->Index()].records.size() - 1; j >= 0; j--) //works pretty good for nospread
		{
			tick_record record = g_BacktrackHelper->PlayerRecord[pTarget->Index()].records.at(j);

			float lerptime = g_BacktrackHelper->GetLerpTime();
			float desired_time = record.m_flSimulationTime + lerptime;
			float estimated_time = g_BacktrackHelper->GetEstimateServerTime();
			float SV_MAXUNLAG = 1.0f;
			float latency = g_BacktrackHelper->GetNetworkLatency();
			float m_flLerpTime = g_BacktrackHelper->GetLerpTime();
			float correct = clamp<float>(latency + m_flLerpTime, 0.0f, SV_MAXUNLAG);
			float deltaTime = correct - (estimated_time + lerptime - desired_time);

			if (fabs(deltaTime) > 0.2f)
				continue;

			if (record.m_flSimulationTime == g_BacktrackHelper->PlayerRecord[pTarget->Index()].records.back().m_flSimulationTime) {
				latest_time = g_BacktrackHelper->PlayerRecord[pTarget->Index()].records.at(j).m_flSimulationTime;
				latest_record = g_BacktrackHelper->PlayerRecord[pTarget->Index()].records.at(j);
			}

			if (record.needs_extrapolation)
				continue;

			if (record.bLowerBodyYawUpdated)
				lby_records.emplace_back(record);
			else if (j != g_BacktrackHelper->PlayerRecord[pTarget->Index()].records.size() - 1)
				trash_records.emplace_back(record);
		}
		/*add records to the list, before we reverse the lby record for hardcore taps*/

		/*reverse lby vectors to tap them at their lby from old to new*/
		//std::reverse(lby_records.begin(), lby_records.end());

		/*check for shit to aim at lby, yay*/

		//check if we need to extrapolate, and just fucking do it, as nothing gets extrapoalted otherwise

		bool was_latest_checked_in_lby = false;

		for (int i = 0; i < lby_records.size(); i++) {
			tick_record record = lby_records.at(i);//total_valid_records.at(j);

			if (record.needs_extrapolation)
				continue;

			simtime = record.m_flSimulationTime;
			origin = record.m_vecOrigin;

			bool is_latest_record = record.m_flSimulationTime == latest_record.m_flSimulationTime;

			if (is_latest_record)
				was_latest_checked_in_lby = true;

			for (auto HitBox : MainHitbox(pTarget, record.boneMatrix)) {
				int hitgroup = -1;

				//fix that only latest record gets multipointed OR use the center of the hitbox to do this when using backtrack records
				if (is_latest_record) {

					if (g_pEngineTrace->IsVisible(G::LocalPlayer, vEyePos, HitBox, pTarget, hitgroup)) {
						float modified_damage = wpn_data->iDamage * (float)pow(wpn_data->flRangeModifier, wpn_data->flRange * 0.002);

						ScaleDamage(hitgroup, pTarget, wpn_data->flArmorRatio, modified_damage);

						if (HitBox != Vector(0, 0, 0) && modified_damage >= minimum_damage)
							return HitBox;
					}
				}

				if (is_latest_record) //neded to comment it, as i can't backtrack autowall atm
				{
					static float damage_given = 0.f;

					if (Clientvariables->Ragebot.Autowall && g_Autowall->CanHit(pTarget, HitBox, &damage_given)) {
						if (damage_given >= minimum_damage) {
							if (HitBox != Vector(0, 0, 0))
								return HitBox;
						}
					}
				}
			}
		}

		//only do autowall & hitscan for the latest lagrecord, but do that before we use other records that ain't good
		if (!was_latest_checked_in_lby && !latest_record.needs_extrapolation) {

			was_latest_checked_in_lby = true;

			simtime = latest_record.m_flSimulationTime;
			origin = latest_record.m_vecOrigin;

			for (auto HitBox : MainHitbox(pTarget, latest_record.boneMatrix)) {
				int hitgroup = -1;

				//fix that only latest record gets multipointed OR use the center of the hitbox to do this when using backtrack records

				if (g_pEngineTrace->IsVisible(G::LocalPlayer, vEyePos, HitBox, pTarget, hitgroup)) {
					float modified_damage = wpn_data->iDamage * (float)pow(wpn_data->flRangeModifier, wpn_data->flRange * 0.002);

					ScaleDamage(hitgroup, pTarget, wpn_data->flArmorRatio, modified_damage);

					if (HitBox != Vector(0, 0, 0) && modified_damage >= minimum_damage)
						return HitBox;
				}

				static float damage_given = 0.f;

				if (Clientvariables->Ragebot.Autowall && g_Autowall->CanHit(pTarget, HitBox, &damage_given)) {
					if (damage_given >= minimum_damage) {
						if (HitBox != Vector(0, 0, 0))
							return HitBox;
					}
				}
			}
		}

		//hitscan and autowall only for latest record before trashrecords
		if (!latest_record.needs_extrapolation) {
			Vector vPoint;
			simtime = latest_record.m_flSimulationTime;
			origin = latest_record.m_vecOrigin;
			for (auto HitboxID : GetHitboxesToScan(pTarget)) {
				std::vector<Vector> Points = GameUtils::GetMultiplePointsForHitbox(pTarget, HitboxID, latest_record.boneMatrix);
				for (int k = 0; k < Points.size(); k++) {

					vPoint = Points.at(k);
					float damage = 0.f;

					int hitgroup = -1;
					if (g_pEngineTrace->IsVisible(G::LocalPlayer, vEyePos, vPoint, pTarget, hitgroup)) {
						float modified_damage = wpn_data->iDamage * (float)pow(wpn_data->flRangeModifier, wpn_data->flRange * 0.002);

						ScaleDamage(hitgroup, pTarget, wpn_data->flArmorRatio, modified_damage);



						if (vPoint != Vector(0, 0, 0) && modified_damage >= minimum_damage)
							return vPoint;
					}

					if (k == 0) //fixes that only the center gets autowall
					{
						static float damage_given = 0.f;

						if (Clientvariables->Ragebot.Autowall && Clientvariables->Ragebot.AutowallHitscan && g_Autowall->CanHit(pTarget, vPoint, &damage_given)) {
							if (damage_given >= minimum_damage) {
								if (vPoint != Vector(0, 0, 0))
									return vPoint;
							}
						}
					}
				}
			}
		}


		//check for other records
		for (int j = 0; j < trash_records.size(); j++) {
			tick_record record = trash_records.at(j);//total_valid_records.at(j);

			if (record.needs_extrapolation)
				continue;

			//*(Vector*)((DWORD)pTarget + 0x110) = record.m_vecVelocity;


			simtime = record.m_flSimulationTime;
			origin = record.m_vecOrigin;

			int count = 0; //fix that only center gets autowalled

						   //don't aim at lby records, as we tested for them before

			for (auto HitBox : MainHitbox(pTarget, record.boneMatrix)) {
				int hitgroup = -1;

				//fix that only latest record gets multipointed OR use the center of the hitbox to do this when using backtrack records
				if (g_pEngineTrace->IsVisible(G::LocalPlayer, vEyePos, HitBox, pTarget, hitgroup)) {
					float modified_damage = wpn_data->iDamage * (float)pow(wpn_data->flRangeModifier, wpn_data->flRange * 0.002);

					ScaleDamage(hitgroup, pTarget, wpn_data->flArmorRatio, modified_damage);

					if (HitBox != Vector(0, 0, 0) && modified_damage >= minimum_damage)
						return HitBox;
				}
			}
		}

        if( latest_record.needs_extrapolation ) {
            Vector position = latest_record.m_vecOrigin;
            float old_simtime = latest_record.m_flSimulationTime;;
            Vector extr_position = position;
            simtime = latest_record.m_flSimulationTime;
            g_BacktrackHelper->ExtrapolatePosition( pTarget, extr_position, simtime, latest_record.m_vecVelocity );

            Msg( "Extrapolating... OldVec(%f, %f, %f) - NewVec(%f, %f, %f)\n", position.x, position.y, position.z, extr_position.x, extr_position.y, extr_position.z );

            for( auto HitBox : MainHitbox( pTarget, /*BoneMatrix*/latest_record.boneMatrix ) ) {
                int hitgroup = -1;

                HitBox -= position;
                HitBox += extr_position;

                if( g_pEngineTrace->IsVisible( G::LocalPlayer, vEyePos, HitBox, pTarget, hitgroup ) ) {
                    if( HitBox != Vector( 0, 0, 0 ))
                        return HitBox;
                }

            }
        }
    }
    else {
        int* array = reinterpret_cast<int*>( offys.dwOcclusionArray );
        *(int*)( (uintptr_t)pTarget + offys.nWriteableBones ) = 0;
        *(int*)( (uintptr_t)pTarget + offys.bDidCheckForOcclusion ) = array[ 1 ];



        pTarget->SetupBones( BoneMatrix, 128, 0x100, g_pGlobals->curtime );
        simtime = pTarget->GetSimulationTime();

        origin = pTarget->GetOrigin();

        for( auto HitBox : MainHitbox( pTarget, BoneMatrix ) ) {
            if( g_pEngineTrace->IsVisible( G::LocalPlayer, vEyePos, HitBox, pTarget ) ) {
                if( HitBox != Vector( 0, 0, 0 ) )
                    return HitBox;
            }
			static float damage_given = 0.f;
            if(Clientvariables->Ragebot.Autowall && g_Autowall->CanHit( pTarget, HitBox, &damage_given ) ) {
				if (damage_given >= minimum_damage) {
					if (HitBox != Vector(0, 0, 0))
						return HitBox;
				}
            }
        }
        for( auto HitboxID : GetHitboxesToScan( pTarget ) ) {

            Vector vPoint;

            std::vector<Vector> Points = GameUtils::GetMultiplePointsForHitbox( pTarget, HitboxID, BoneMatrix );
            for( int k = 0; k < Points.size(); k++ ) {

                vPoint = Points.at( k );
                float damage = 0.f;

                int hitgroup = -1;
                if( g_pEngineTrace->IsVisible( G::LocalPlayer, vEyePos, vPoint, pTarget, hitgroup ) ) {
                    if( vPoint != Vector( 0, 0, 0 ))
                        return vPoint;
                }

				static float damage_given = 0.f;

				if (Clientvariables->Ragebot.Autowall && Clientvariables->Ragebot.AutowallHitscan && g_Autowall->CanHit(pTarget, vPoint, &damage_given)) {
					if (damage_given >= minimum_damage) {
						if (vPoint != Vector(0, 0, 0))
							return vPoint;
					}
				}
            }
        }
    }
    return Vector( 0, 0, 0 );
}


CAimbot* g_Aimbot = new CAimbot;
void CAimbot::DropTarget()
{
    target_index = -1;
    best_distance = 99999.f;
    aimbotted_in_current_tick = false;
    fired_in_that_tick = false;
    current_aim_position = Vector();
    pTarget = nullptr;
}

void CAimbot::Run()
{
    if( !Clientvariables->Ragebot.EnableAimbot)
        return;

    QAngle view; g_pEngine->GetViewAngles( view );
    CBaseCombatWeapon* pWeapon = G::LocalPlayer->GetWeapon();

    if( !pWeapon || pWeapon->Clip1() == 0 || pWeapon->IsMiscWeapon() || !GameUtils::IsAbleToShoot() )
        return;

    for( int i = 1; i < 64; ++i ) {
        if( i == g_pEngine->GetLocalPlayer() )
            continue;

        CBaseEntity* target = g_pEntitylist->GetClientEntity( i );

        if( !target->IsValidTarget() )
            continue;

		G::Target = target;

        float fov = GameUtils::GetFoV( view, G::LocalPlayer->GetEyePosition(), target->GetEyePosition(), false );

        if( fov > 180.f)
            continue;

        float simtime = 0;
        Vector minus_origin = Vector( 0, 0, 0 );
        Vector aim_position = RunAimScan( target, simtime, minus_origin );

        if( aim_position == Vector( 0, 0, 0 ) )
            continue;

        float selection_value = 0;
        switch( Clientvariables->Ragebot.AimbotSelection ) {
        case 0:
            selection_value = fov;
            break;
        case 2:
            selection_value = Math::RandomFloat( 0, 100 );
            break;
        case 3:
            selection_value = target->GetVelocity().Length();
            break;
        case 4:
            selection_value = target->GetHealth();
            break;

        default:
            break;
        }

        if( best_distance >= selection_value && aim_position != Vector( 0, 0, 0 ) ) {
            best_distance = selection_value;


            target_index = i;
            current_aim_position = aim_position;
            pTarget = target;
            current_aim_simulationtime = simtime;
            current_aim_player_origin = minus_origin;
        }
    }
    if( target_index != -1 && current_aim_position != Vector( 0, 0, 0 ) && pTarget ) {
        aimbotted_in_current_tick = true;
        QAngle aim = GameUtils::CalculateAngle( G::LocalPlayer->GetEyePosition(), current_aim_position );
        aim.y = Math::NormalizeYaw( aim.y );


        G::UserCmd->viewangles = aim;

        if(!Clientvariables->Ragebot.SilentAimbot)
            g_pEngine->SetViewAngles( G::UserCmd->viewangles );

		if (Clientvariables->Ragebot.AutomaticFire)
		{
			if (Clientvariables->Ragebot.AutomaticScope && pWeapon->IsScopeable() && !G::LocalPlayer->IsScoped())
				G::UserCmd->buttons |= IN_ATTACK2;
			else
			{
				if (Clientvariables->Ragebot.Minhitchance == 0 || HitChance(pTarget, pWeapon, aim, Clientvariables->Ragebot.Minhitchance))
				{
					G::UserCmd->buttons |= IN_ATTACK;
					this->fired_in_that_tick = true;
				}

			}
		}

        if(Clientvariables->Ragebot.PositionAdjustment)
            G::UserCmd->tick_count = TIME_TO_TICKS( current_aim_simulationtime ) + TIME_TO_TICKS( g_BacktrackHelper->GetLerpTime() );// for backtracking
    }
}

void CAimbot::CompensateInaccuracies()
{
	if (G::UserCmd->buttons & IN_ATTACK)
	{
		if (Clientvariables->Ragebot.NoRecoil)
		{
			ConVar* recoilscale = g_pCvar->FindVar("weapon_recoil_scale");

			if (recoilscale) {
				QAngle qPunchAngles = G::LocalPlayer->GetPunchAngle();
				QAngle qAimAngles = G::UserCmd->viewangles;
				qAimAngles -= qPunchAngles * recoilscale->GetFloat();
				G::UserCmd->viewangles = qAimAngles;
			}
		}
	}
}

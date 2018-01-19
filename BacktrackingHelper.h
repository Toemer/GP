#pragma once
struct tick_record
{
	Vector m_vecOrigin;//0 //4 // 8
	bool bLowerBodyYawUpdated;
	bool bFakelagging;
	float m_flSimulationTime; //12
	float m_flAnimTime;
	int m_nSequence;//16
	float m_flCycle; //20
	Vector m_angEyeAngles;//x->24
						  //y->28
						  //z->32
	float m_flUpdateTime;//36
	Vector m_vecVelocity;//40 //44 //48
	float m_flPoseParameter[24];//52
	float backtrack_time;

	bool needs_extrapolation = false;
	/*my addition*/
	VMatrix boneMatrix[128];
};
struct player_record
{
	//std::vector<tick_record> records;
	std::vector<tick_record> records;
	Vector EyeAngles;
	float LowerBodyYawTarget;
	int Flags;
	float unknown;
};
struct simulation_record
{
	Vector origin;
	Vector velocity;
	Vector acceleration;
	float simulation_time;
	float simulation_time_increasment_per_tick;
	float update_time;

};
class CBacktrackHelper
{
public:
	player_record PlayerRecord[64];
	simulation_record SimRecord[64][7];
	float GetLerpTime();

	float GetNetworkLatency();
	int GetLatencyTicks();
	float GetEstimateServerTime();
	void UpdateBacktrackRecords(CBaseEntity* pPlayer);
	void UpdateExtrapolationRecords(CBaseEntity* pPlayer);
	void ExtrapolatePosition(CBaseEntity* pPlayer, Vector& position, float &simtime, Vector velocity);
}; extern CBacktrackHelper* g_BacktrackHelper;

struct lbyRecords
{
	int tick_count;
	float lby;
	Vector headPosition;
};
struct backtrackData
{
	float simtime;
	Vector hitboxPos;
};

class BackTrack
{
	int latest_tick;
	bool IsTickValid(int tick);
	void UpdateRecord(int i);
public:
	lbyRecords records[64];
	bool RunLBYBackTrack(int i, CUserCmd* cmd, QAngle& aimPoint);
	void Update(int tick_count);
};

extern BackTrack* g_backtrack;


class c_backtrack
{
public:
	player_record s_player_record[64];
	int GetDesiredTickCount(float flTargetTime);
	float GetNetworkLatency();
	int GetLatencyTicks();
	int GetEstimateServerTickCount();
	float GetEstimateServerTime();
	void UpdateRecords(CBaseEntity* pPlayer);
	void RestoreRecords(CBaseEntity* pPlayer);
};

extern c_backtrack* g_backtracking;
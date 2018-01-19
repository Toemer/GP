#pragma once

class ClientVariables
{
public:

	struct Ragebot
	{
		bool EnableAimbot = false;
		int AimbotSelection = 0;
		bool AutomaticFire = false;
		bool AutomaticScope = false;
		bool SilentAimbot = false;
		bool NoRecoil = false;
		bool NoSpread = false;
		bool PositionAdjustment = false;
		int Preferbodyaim = 0;
		int Hitbox = 0;
		int Hitscan = 0;
		bool AutowallHitscan = false;
		bool Autowall = false;
		int Multipoint = 0.f;
		float Headscale = 0.0f;
		float Bodyscale = 0.0f;
		int Mindamage = 1.f;
		bool Hitchance = false;
		int Minhitchance = 0.f;
		bool AutomaticResolver = false;
		int ResolverStepAngle = 25;
		bool FriendlyFire = false;
		bool Quickstop = false;


		
		
	} Ragebot;

	struct Antiaim
	{
		bool AntiaimEnable = false;
		int Pitch = 0;

		int Yaw = 0;
		int YawAdd = 0;
		int YawRunning = 0;
		int YawRunningAdd = 0;
		int FakeYaw = 0;
		int FakeYawAdd = 0;

		int Fakewalk = 0;

		int Freestanding = 0;
		int FreestandingDelta = 0;

		int AtPlayer = 0;
		bool Disableaa = false;
		int Jitterrange = 0.f;
		bool Randomizejitter = false;
		int AntiaimMode = 0;
	} Antiaim;

	struct Visuals
	{
		bool EspEnable = false;
		bool EnemyOnly = false;
		bool BoundingBox = false;
		bool Bones = false;
		bool Health = false;
		bool Armor = false;
		bool Flags = false;
		bool Fake = false;
		bool Dlight = false;
		bool Name = false;
		bool Weapon = false;
		bool Ammo = false;
		bool AllItems = false;
		bool Rank = false;
		bool Wins = false;
		bool Glow = false;
		bool LineofSight = false;
		bool SnapLines = false;
		bool GrenadePrediction = false;
		int Crosshair = 0;
		bool SpreadCrosshair = false;
		bool RecoilCrosshair = false;
		bool FartherESP = false;

		//Cbase/filers
		int DroppedWeapons = 0;
		bool Hostage = false;
		bool ThrownNades = false;
		bool LocalPlayer = false;
		bool BulletTracers = false;
		bool Bomb = false;
		bool Spectators = false;
		bool OutOfPOVArrows = false;
		bool DamageIndicators = false;


		//Effects/world
		bool nightmode = false;
		int Skybox = 0;
		int FlashbangAlpha = 0;
		bool Nosmoke = false;
		bool Noscope = false;
		bool RemoveParticles = false;
		bool Novisrevoil = false;
		bool Hitmarker = false;
		bool ChamsEnable = false;
		int ChamsStyle = 0;
		int FakeAngleGhost = 0;
		bool Chamsenemyonly = false;
		bool ChamsPlayer = false;
		bool ChamsPlayerWall = false;
		bool ChamsHands = false;
		bool ChamsHandsWireframe = false;
		bool WeaponWireframe = false;
		bool WeaponChams = false;
	} Visuals;

	struct Misc
	{
		bool AntiUT = false;
		int PlayerFOV = 0.f;
		int PlayerViewmodel = 0.f;
		int TPangles = 0;
		int TPKey = 0;
		int MenuKey = 0x2d;
		int WalkbotSet = 0x2d;
		int WalkbotDelete = 0x2d;
		int WalkbotStart = 0x2d;
		int Clantag = 0;
		bool AutoJump = false;
		bool AutoStrafe = false;
		bool AutoAccept = false;
		bool Prespeed = false;
		int Retrack = 0.f;
		int PrespeedKey = 0;
		bool FakelagEnable = false;
		bool FakelagOnground = false;
		int FakelagMode = 0;
		int FakelagAmount = 0.f;
		bool FakelagShoot = false;
		int ConfigSelection = 0;
		bool Walkbot = false;
		bool WalkbotBunnyhop = false;
		//int WalkbotSetPoint = 0;
		//int WalkbotDeletePoint = 0;
		//int WalkbotStart = 0;
		bool FakewalkEnable = false;
		int FakewalkKey = 0;
	} Misc;

	struct Skins
	{
		bool Enabled;
		int Knife;
		int gloves;
		int KnifeSkin;
		int AK47Skin;
		int M4A1SSkin;
		int M4A4Skin;
		int AUGSkin;
		int FAMASSkin;
		int AWPSkin;
		int SSG08Skin;
		int SCAR20Skin;
		int P90Skin;
		int UMP45Skin;
		int GlockSkin;
		int USPSkin;
		int DeagleSkin;
		int tec9Skin;
		int P2000Skin;
		int P250Skin;
		int CZ75Skin;
		int RevolverSkin;
		int DualSkins;
		int FiveSevenSkin;
	} Skinchanger;

	struct CPlayerlist
	{
		bool bEnabled;
		int iPlayer;
		char* szPlayers[64] = {
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" "
		};
	} Playerlist;

	struct NigColors
	{
		float MenuColor[3] = { 1.08f,1.95f,0.18f };
		float BoundingBox[3] = { 0.30f,1.45f,1.50f };

		float PlayerChams[3] = { 0.30f,1.45f,1.50f };
		float PlayerChamsWall[3] = { 0.30f,1.45f,1.50f };
		float Skeletons[3] = { 2.55f,2.55f,2.55f };
		float Bulletracer[3] = { 2.44f,1.66f,2.31f };

		float WireframeHand[3] = { 0.30f,1.45f,1.50f };
		float ChamsHand[3] = { 0.30f,1.45f,1.50f };
		float ChamsWeapon[3] = { 0.30f,1.45f,1.50f };
		float WireframeWeapon[3] = { 0.30f,1.45f,1.50f };
		float Glow[3] = { 0.30f,1.45f,1.50f };
		float DroppedWeapon[3] = { 0.30f,1.45f,1.50f };
		float Bomb[3] = { 0.30f,1.45f,1.50f };
		float PlantedBomb[3] = { 0.30f,1.45f,1.50f };
		float Hostage[3] = { 0.30f,1.45f,1.50f };
		float GrenadePrediction[3] = { 0.30f,1.45f,1.50f };
		float FakeAngleGhost[3] = { 0.30f,1.45f,1.50f };
		float SpreadCrosshair[3] = { 0.30f,1.45f,1.50f };
		float Snaplines[3] = { 0.30f,1.45f,1.50f };
		float DamageIndicator[3] = { 0.30f,1.45f,1.50f };

	}Colors;
};

extern ClientVariables* Clientvariables;

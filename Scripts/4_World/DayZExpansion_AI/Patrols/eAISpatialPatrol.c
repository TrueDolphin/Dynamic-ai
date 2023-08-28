/*
13/8/2023
spatial patrol
*/
class eAISpatialPatrol : eAIPatrol
{
	private static int m_NumberOfSpatialPatrols;

	vector m_Position;
	autoptr array<vector> m_Waypoints;
	eAIWaypointBehavior m_WaypointBehaviour;
	float m_MinimumRadius;
	float m_MaximumRadius;
	float m_DespawnRadius;
	float m_MovementSpeedLimit;
	float m_MovementThreatSpeedLimit;
	int m_NumberOfAI;
	int m_RespawnTime;
	int m_DespawnTime;
	string m_Loadout;
	ref eAIFaction m_Faction;
	ref eAIFormation m_Formation;
	ref eAIGroup m_Group;
	bool m_CanBeLooted;
	bool m_UnlimitedReload;
	float m_SniperProneDistanceThreshold;
	float m_AccuracyMin;
	float m_AccuracyMax;
	float m_ThreatDistanceLimit;
	float m_DamageMultiplier;

	string m_GroupName;
	float m_TimeSinceLastSpawn;
	bool m_CanSpawn;
	private bool m_WasGroupDestroyed;

	//additionals
	ref Spatial_Groups m_Spatial_Groups;
	PlayerBase m_Hunted;
	int m_lootcheck;
	int m_Location = 0;
	int m_Huntmode;

	void eAISpatialPatrol() {
		GetSpatialSettings().PullRef(m_Spatial_Groups);	
	}//Spatial settings reference
	static eAISpatialPatrol CreateEx(vector pos, array<vector> waypoints, eAIWaypointBehavior behaviour, string loadout = "", int count = 1, int respawnTime = 600, int despawnTime = 600, eAIFaction faction = null, eAIFormation formation = null, PlayerBase player = null, float minR = 300, float maxR = 800, float despawnR = 880, int HuntMode = 1, float threatspeedLimit = 3.0, int lootcheck = 1, bool unlimitedReload = false) {
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0("eAISpatialPatrol", "Create");
		#endif

		eAISpatialPatrol patrol;	
		Class.CastTo(patrol, ((typename)eAISpatialPatrol).Spawn());
		patrol.m_Position = pos;
		patrol.m_Waypoints = waypoints;
		patrol.m_WaypointBehaviour = behaviour;
		patrol.m_NumberOfAI = count;
		patrol.m_Loadout = loadout;
		patrol.m_RespawnTime = respawnTime;
		patrol.m_DespawnTime = despawnTime;
		patrol.m_MinimumRadius = minR;
		patrol.m_MaximumRadius = maxR;
		patrol.m_DespawnRadius = despawnR;
		patrol.m_MovementSpeedLimit = 2.0;
		patrol.m_MovementThreatSpeedLimit = 3.0;
		patrol.m_Faction = faction;
		patrol.m_Formation = formation;
		patrol.m_CanBeLooted = CheckLootable(lootcheck);
		patrol.m_lootcheck = lootcheck;
		patrol.m_UnlimitedReload = unlimitedReload;
		patrol.m_Hunted = player;
		patrol.m_CanSpawn = true;
		patrol.m_Location = 0;
		patrol.m_Huntmode = HuntMode;
		GetSpatialSettings().PullRef(patrol.m_Spatial_Groups);
		if (patrol.m_Faction == null) patrol.m_Faction = new eAIFactionCivilian();
		if (patrol.m_Formation == null) patrol.m_Formation = new eAIFormationVee();
		patrol.Start();
		return patrol;
		}//edited

	static eAISpatialPatrol Create(vector pos, array<vector> waypoints, eAIWaypointBehavior behaviour, string loadout = "", int count = 1, int respawnTime = 600, int despawnTime = 600, eAIFaction faction = null, eAIFormation formation = null, PlayerBase player = null, float minR = 300, float maxR = 800, int HuntMode = 1, float threatspeedLimit = 3.0, int lootcheck = 1, bool unlimitedReload = false) {
		return CreateEx(pos, waypoints, behaviour, loadout, count, respawnTime, despawnTime, faction, null, player, minR, maxR, maxR * 1.1, HuntMode, threatspeedLimit, lootcheck, unlimitedReload);
		}//edited

	void SetAccuracy(float accuracyMin, float accuracyMax) {
		m_AccuracyMin = accuracyMin;
		m_AccuracyMax = accuracyMax;
		}

	void SetThreatDistanceLimit(float distance) {
		m_ThreatDistanceLimit = distance;
		}

	void SetDamageMultiplier(float multiplier) {
		m_DamageMultiplier = multiplier;
		}

	void SetGroupName(string name) {
		m_GroupName = name;
		}

	void SetHunted(PlayerBase p){
		m_Hunted = p;
	}//for movement checks
	void SetSniperProneDistanceThreshold(float distance) {
		m_SniperProneDistanceThreshold = distance;
		}

	eAIGroup GetGroup(){
		return m_Group;
		}//null

	void SetLocation(){
		m_Location = 1;
		}//for location checks

	private eAIBase SpawnAI(vector pos) {
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "SpawnAI");
		#endif

		pos = ExpansionAIPatrol.GetPlacementPosition(pos);

		eAIBase ai;
		if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), pos))) return null;

		ai.SetPosition(pos);

		if ( m_Loadout == "" )
			m_Loadout = m_Faction.GetDefaultLoadout();

		ExpansionHumanLoadout.Apply(ai, m_Loadout, false);	
		ai.SetMovementSpeedLimits(m_MovementSpeedLimit, m_MovementThreatSpeedLimit);
		ai.Expansion_SetCanBeLooted(CheckMemberLootable(ai, m_lootcheck));
		ai.eAI_SetUnlimitedReload(m_UnlimitedReload);
		ai.eAI_SetAccuracy(m_AccuracyMin, m_AccuracyMax);
		ai.eAI_SetThreatDistanceLimit(m_ThreatDistanceLimit);
		ai.eAI_SetDamageMultiplier(m_DamageMultiplier);
		ai.eAI_SetSniperProneDistanceThreshold(m_SniperProneDistanceThreshold);

		return ai;
		}//edited

	bool CheckMemberLootable(eAIBase ai = null, int lootcheck = 0) {
		switch (lootcheck) {
			case 1:
				return true;
			case 2:
				//random
				int r = Math.RandomIntInclusive(0, 1);
				return r;
			case 3:
				//leader only
				if (ai.GetGroup().GetLeader() == ai) return true;
				return false;
		}
		return false;
	}//CheckMemberLootable(ai, m_lootcheck);
	static bool CheckLootable(int lootcheck = 0) {
		if (lootcheck > 2) lootcheck = 0;
		switch (lootcheck) {
			case 0:
			case 1:
				return lootcheck;
			case 2:
				//random
				int r = Math.RandomIntInclusive(0, 1);
				return r;
		}
		return false;
	}//CheckLootable(m_lootcheck);
	bool WasGroupDestroyed() {
		if (!m_Group)
			return false;

		if (m_WasGroupDestroyed)
			return true;

		for (int i = 0; i < m_Group.Count(); ++i)
		{
			DayZPlayerImplement member = m_Group.GetMember(i);
			if (member && member.IsInherited(PlayerBase) && member.IsAlive())
			{
				return false;
			}
		}

		m_WasGroupDestroyed = true;

		if (m_NumberOfSpatialPatrols)
			m_NumberOfSpatialPatrols--;

		return true;
		}

	void Spawn() {
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "Spatial Spawn");
		#endif

		if (m_Group) return;

		if (GetExpansionSettings().GetLog().AIPatrol)
		{
			string name = m_GroupName;
			if (name == string.Empty)
				name = m_Faction.ClassName().Substring(10, m_Faction.ClassName().Length() - 10);
            GetExpansionSettings().GetLog().PrintLog("[Spatial AI] Spawning " + m_NumberOfAI + " " + name + " bots at " + m_Position);
        }
		m_TimeSinceLastSpawn = 0;
		m_CanSpawn = false;
		m_WasGroupDestroyed = false;

		eAIBase ai = SpawnAI(m_Position);
		m_Group = ai.GetGroup();
		m_Group.SetFaction(m_Faction);
		m_Group.SetFormation(m_Formation);
		m_Group.SetWaypointBehaviour(m_WaypointBehaviour);
		m_Group.SetName(m_GroupName);

		for (int idx = 0; idx < m_Waypoints.Count(); ++idx)
		{
			m_Group.AddWaypoint(m_Waypoints[idx]);
			if (m_Waypoints[idx] == m_Position)
			{
				m_Group.m_CurrentWaypointIndex = idx;
				//if (Math.RandomIntInclusive(0, 1)) m_Group.m_BackTracking = true;
			}
		}

		Spatial_Movement(m_Huntmode);

		for (int i = 0; i < m_NumberOfAI; ++i)
		{
			ai = SpawnAI(m_Formation.ToWorld(m_Formation.GetPosition(i)));
			ai.SetGroup(m_Group);
			ai.eAI_SetAccuracy(-1, -1);
		}
		++m_NumberOfSpatialPatrols;
		}//edited

	void Despawn(bool deferDespawnUntilLoosingAggro = false) {
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "Spatial Despawn");
		#endif

		m_TimeSinceLastSpawn = 0;

		if (m_Group)
		{
			m_Group.ClearAI(true, deferDespawnUntilLoosingAggro);
			m_Group = null;
		}

		if (!m_WasGroupDestroyed && m_NumberOfSpatialPatrols) m_NumberOfSpatialPatrols--;
		if (this) this.Delete();
		}//edited

	override void OnUpdate() {
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "Spatial OnUpdate");
		#endif

		if ( WasGroupDestroyed() && m_RespawnTime < 0 )
		{
			return;
		}

		if (!m_CanSpawn && (!m_Group || m_WasGroupDestroyed))
		{
			m_TimeSinceLastSpawn += eAIPatrol.UPDATE_RATE_IN_SECONDS;
			//m_CanSpawn = m_RespawnTime > -1 && m_TimeSinceLastSpawn >= m_RespawnTime;
			//! https://feedback.bistudio.com/T173348
			m_CanSpawn = false;
			if (m_RespawnTime > -1 && m_TimeSinceLastSpawn >= m_RespawnTime) m_CanSpawn = true;
		}

		if (!m_Group)
		{
			if (!m_CanSpawn) return;
			int maxPatrols = m_Spatial_Groups.MaxAI;
			if (maxPatrols > -1 && m_NumberOfSpatialPatrols >= maxPatrols) return;
		}

		//! CE API is only avaliable after game is loaded
		if (!GetCEApi()) return;

		vector patrolPos = m_Position;
		DayZPlayerImplement leader = null;
		if (m_Group && m_Group.GetLeader())
		{
			leader = m_Group.GetLeader();
			patrolPos = leader.GetPosition();
		}

		if (m_Group)
		{
			if (GetCEApi().AvoidPlayer(patrolPos, m_DespawnRadius))
			{
				m_TimeSinceLastSpawn += eAIPatrol.UPDATE_RATE_IN_SECONDS;
				if (m_TimeSinceLastSpawn >= m_DespawnTime)
					Despawn();
			}
		}
		else
		{
			if (!GetCEApi().AvoidPlayer(patrolPos, m_MaximumRadius) && GetCEApi().AvoidPlayer(patrolPos, m_MinimumRadius))
			{
			Spawn();	
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Despawn, m_Spatial_Groups.CleanupTimer, false, true);
			} 
		}
		}//edited

    void Spatial_Movement(int m_Mode) {
		eAIBase ai = eAIBase.Cast(m_Group.GetLeader());
		PlayerBase player = m_Hunted;
		if (!player || !m_Group || !ai) return;
		if (!m_Mode) m_Mode = m_Spatial_Groups.HuntMode;
		int i;
		float c = (m_Spatial_Groups.EngageTimer / 2500) + 1;
		m_Group.ClearWaypoints();
		switch (m_Mode) {
			case 1: 
				//actively hunts player
				m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 5, 10));
				player.GetTargetInformation().AddAI(ai, m_Spatial_Groups.EngageTimer);
				break;
			case 2: 
				//last known location
				m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 5, 10));
			break;
			case 3: 
				//halt
			break;
			case 4: 
				//stay around spawnpos
				for (i = 0; i < c; ++i) m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 70, 80));
			break;
			case 5: 
				//mix of 4 and 6 sorta
				for (i = 0; i < c; ++i) Spatial_PointGen(ai, m_Group, player);
			break;
			case 6: 
				//follows player
				m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 50, 55));
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(TrailingGroup, 15000, false, m_Group, player);
			break;
    	}
	}//Spatial_Movement(ai, group);
	void Spatial_PointGen(eAIBase ai, eAIGroup AiGroup, PlayerBase player) {
		int d = Math.RandomIntInclusive(0, 100);
		if (d < 16) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 120));
		if (d > 15 && d < 95) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 80, 200));
		if (d > 94) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 10, 20));
	}//Spatial_PointGen(ai, AiGroup, player);
	 //! https://feedback.bistudio.com/T173348 - readded null checks
	void TrailingGroup(eAIGroup AiGroup, PlayerBase player, vector pos = "0 0 0", int timer = 10000) {
		//Print("Trailing trigger" + this);
		if (!player || !AiGroup) return;
		if (pos == player.GetPosition()) 
			AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 30, 55));
		
		if (!player || !AiGroup) return;
		EntityAI lead = AiGroup.GetLeader();
		if (!lead) return;
		if (vector.Distance(player.GetPosition(), AiGroup.GetLeader().GetPosition()) > 140) {
			AiGroup.ClearWaypoints();
			AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 80, 100));
		} else AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 80, 100));
		
		if (!player) return;
		pos = player.GetPosition();
		if (!player || !AiGroup) return;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TrailingGroup, timer, false, AiGroup, player, pos);
	}//TrailingGroup(ai, player);
	override void Debug() {
		///super.Debug();
		Print("=======Dynamic Debug========");
		Print(m_Hunted);
		Print(m_Group);
		Print(m_NumberOfAI);
		Print(WasGroupDestroyed());
		Print("=======Dynamic Debug========");
		}//edited
};
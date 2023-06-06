//! because too many things are private..
//! i wanted to change the exp log to [Spatial AI] for the spawn() function

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
	ref Spatial_Groups m_Spatial_Groups;
	bool m_CanBeLooted;
	bool m_UnlimitedReload;
	float m_SniperProneDistanceThreshold;
	float m_AccuracyMin;
	float m_AccuracyMax;
	float m_ThreatDistanceLimit;
	float m_DamageMultiplier;
	PlayerBase m_Hunted;

	ref eAIGroup m_Group;
	eAIBase TrueLead;
	int m_lootcheck;
	int m_Location;
	string m_GroupName;
	float m_TimeSinceLastSpawn;
	bool m_CanSpawn;
	private bool m_WasGroupDestroyed;

	static eAISpatialPatrol CreateEx(vector pos, array<vector> waypoints, eAIWaypointBehavior behaviour, string loadout = "", int count = 1, int respawnTime = 600, int despawnTime = 600, eAIFaction faction = null, eAIFormation formation = null, bool autoStart = true, float minR = 300, float maxR = 800, float despawnR = 880, float speedLimit = 3.0, float threatspeedLimit = 3.0, int lootcheck = 1, bool unlimitedReload = false/*, float accuracyMin = -1, float accuracyMax = -1*/)
	{
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
		patrol.m_MovementSpeedLimit = speedLimit;
		patrol.m_MovementThreatSpeedLimit = threatspeedLimit;
		patrol.m_Faction = faction;
		patrol.m_Formation = formation;
		patrol.m_CanBeLooted = true;
		patrol.m_lootcheck = lootcheck;
		patrol.m_UnlimitedReload = unlimitedReload;
		patrol.m_CanSpawn = true;
		if (patrol.m_Faction == null) patrol.m_Faction = new eAIFactionCivilian();
		if (patrol.m_Formation == null) patrol.m_Formation = new eAIFormationVee();
		if (autoStart) patrol.Start();
		return patrol;
	}

	static eAISpatialPatrol Create(vector pos, array<vector> waypoints, eAIWaypointBehavior behaviour, string loadout = "", int count = 1, int respawnTime = 600, eAIFaction faction = null, bool autoStart = true, float minR = 300, float maxR = 800, float speedLimit = 3.0, float threatspeedLimit = 3.0, int lootcheck = 1, bool unlimitedReload = false)
	{
		return CreateEx(pos, waypoints, behaviour, loadout, count, respawnTime, 600, faction, null, autoStart, minR, maxR, maxR * 1.1, speedLimit, threatspeedLimit, lootcheck, unlimitedReload);
	}

	void SetAccuracy(float accuracyMin, float accuracyMax)
	{
		m_AccuracyMin = accuracyMin;
		m_AccuracyMax = accuracyMax;
	}

	void SetThreatDistanceLimit(float distance)
	{
		m_ThreatDistanceLimit = distance;
	}

	void SetDamageMultiplier(float multiplier)
	{
		m_DamageMultiplier = multiplier;
	}

	void SetGroupName(string name)
	{
		m_GroupName = name;
	}
	void SetHunted(PlayerBase p){
		m_Hunted = p;
	}

	void SetSniperProneDistanceThreshold(float distance)
	{
		m_SniperProneDistanceThreshold = distance;
	}

	eAIGroup GetGroup(){
		return m_Group;
	}

	void SetLocation(){
		m_Location = 1;
	}
	private eAIBase SpawnAI(vector pos)
	{
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
		GetSpatialSettings().PullRef(m_Spatial_Groups);		
		ai.SetMovementSpeedLimits(m_MovementSpeedLimit, m_MovementThreatSpeedLimit);
		ai.eAI_SetUnlimitedReload(m_UnlimitedReload);
		ai.eAI_SetAccuracy(m_AccuracyMin, m_AccuracyMax);
		ai.eAI_SetThreatDistanceLimit(m_ThreatDistanceLimit);
		ai.eAI_SetDamageMultiplier(m_DamageMultiplier);
		ai.eAI_SetSniperProneDistanceThreshold(m_SniperProneDistanceThreshold);

		return ai;
	}

	//sigh
	bool CheckMemberLootable(eAIBase ai, int lootcheck) {
		switch (lootcheck) {
			case 0:
			case 1:
				//true-false
				return lootcheck;
				break;
			case 2:
				//ranfom
				int r = Math.RandomIntInclusive(0, 1);
				return r;
				break;
			case 3:
				//leader only
				if (ai.GetGroup().GetLeader() == ai) {
				return true;
				} else {
				return false;
				}
				break;
		}
		return false;
	}

	bool WasGroupDestroyed()
	{
		if (!m_Group)
			return false;

		if (m_WasGroupDestroyed)
			return true;

		for (int i = 0; i < m_Group.Count(); i++)
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

	void Spawn()
	{
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
		TrueLead = ai;
		m_Group = ai.GetGroup();
		m_Group.SetFaction(m_Faction);
		m_Group.SetFormation(m_Formation);
		m_Group.SetWaypointBehaviour(m_WaypointBehaviour);
		m_Group.SetName(m_GroupName);

		

		for (int idx = 0; idx < m_Waypoints.Count(); idx++)
		{
			m_Group.AddWaypoint(m_Waypoints[idx]);
			if (m_Waypoints[idx] == m_Position)
			{
				m_Group.m_CurrentWaypointIndex = idx;
				if (Math.RandomIntInclusive(0, 1))
					m_Group.m_BackTracking = true;
			}
		}

		for (int i = 1; i < m_NumberOfAI; i++)
		{
			ai = SpawnAI(m_Formation.ToWorld(m_Formation.GetPosition(i)));
			ai.SetGroup(m_Group);
			ai.Expansion_SetCanBeLooted(CheckMemberLootable(ai, m_lootcheck));
			ai.eAI_SetAccuracy(-1, -1);
		}
		m_NumberOfSpatialPatrols++;
      GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.Despawn, m_Spatial_Groups.CleanupTimer, false);
	}

	void Despawn(bool deferDespawnUntilLoosingAggro = false)
	{
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "Spatial Despawn");
		#endif

		m_TimeSinceLastSpawn = 0;

		if (m_Group)
		{
			m_Group.ClearAI(true, deferDespawnUntilLoosingAggro);
			m_Group = null;
		}

		if (!m_WasGroupDestroyed && m_NumberOfSpatialPatrols)
			m_NumberOfSpatialPatrols--;
	}

	override void OnUpdate()
	{
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
			m_CanSpawn = m_RespawnTime > -1 && m_TimeSinceLastSpawn >= m_RespawnTime;
		}

		if (!m_Group)
		{
			if (!m_CanSpawn)
			{
				return;
			}

			int maxPatrols = GetExpansionSettings().GetAI().MaximumDynamicPatrols;
			if (maxPatrols > -1 && m_NumberOfSpatialPatrols >= maxPatrols)
			{
				return;
			}
		}

		//! CE API is only avaialble after game is loaded
		if (!GetCEApi())
			return;

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
			}
		}
	}

  //Spatial_Movement(ai, player)
  void Spatial_Movement() {
	PlayerBase player = m_Hunted;
	eAIBase ai = TrueLead;
    eAIGroup AiGroup = ai.GetGroup();
    if (!AiGroup) return;
    AiGroup.ClearWaypoints();
    int m_Mode;
    if (player.CheckZone() == true) {
      m_Mode = player.Spatial_HuntMode();
    } else {
      m_Mode = m_Spatial_Groups.HuntMode;
    }
	if (m_Location == 1) m_Mode = 3;
    switch (m_Mode) {
    case 1: {
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 0, 3));
      player.GetTargetInformation().AddAI(ai, m_Spatial_Groups.EngageTimer);
      break;
    }
    case 2: {
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 0, 3));
      break;
    }
    case 3: {
      /*
      just spawn, dont chase unless standard internal contitions met.
      */
      break;
    }
    case 4: {
      //mostly irrelevant - generic waypoints do better
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 70, 80));
      break;
    }
    case 5: {
      float c = m_Spatial_Groups.EngageTimer / 2500;
      for (int i = 0; i < c; i++) {
        int d = Math.RandomIntInclusive(0, 100);
        if (d < 16) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 120));
        if (d > 15 && d < 95) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 80, 200));
        if (d > 94) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 10, 20));
      }
    }
    case 6: {
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 50, 55));
      GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TrailingGroup, 15000, false, ai, player, Vector(0, 0, 0), 15000);
    }
    }
  }


  void SetGroupAccuracy(eAIGroup group) {
    if (!group) return;
    for (int i = 0; i < group.Count(); i++) {
      eAIBase ai = eAIBase.Cast(group.GetMember(i));
      if (!ai) return;
      ai.eAI_SetAccuracy(-1, -1);
    }
  }

  //TrailingGroup(ai, player, Vector(0, 0, 0), 15000)
  void TrailingGroup(eAIBase ai, PlayerBase player, vector pos, int timer) {
    if (!player || !ai) return;
    eAIGroup AiGroup = eAIGroup.Cast(ai.GetGroup());
    if (!AiGroup) AiGroup = eAIGroup.GetGroupByLeader(ai);
    if (pos == player.GetPosition()) {
      if (player && ai) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 30, 55));
    }
    if (vector.Distance(player.GetPosition(), ai.GetPosition()) > 140) {
      if (player && ai) {
        AiGroup.ClearWaypoints();
        AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 80, 100));
      }
    }
    if (player) pos = player.GetPosition();
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TrailingGroup, timer, false, ai, player, pos, timer);
  }

	override void Debug()
	{
		super.Debug();
		
		Print(m_Group);
		Print(m_TimeSinceLastSpawn);
		Print(m_CanSpawn);
		Print(m_NumberOfAI);
		Print(WasGroupDestroyed());
	}
};

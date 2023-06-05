class Location_Trigger: CylinderTrigger
{       

    string Spatial_Name;
    float Spatial_TriggerRadius;
    string Spatial_ZoneLoadout;
    int Spatial_MinCount;
    int Spatial_MaxCount;
    int Spatial_HuntMode;
    int SpawnCount;
    string Spatial_Faction;
    vector Spatial_TriggerPosition;
    vector Spatial_SpawnPosition;
    ref Spatial_Groups m_Spatial_Groups;
    eAISpatialPatrol dynPatrol;
    int i_PlayerCount;

    void Location_Trigger(){
     GetSpatialSettings().PullRef(m_Spatial_Groups);  
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnCheck, m_Spatial_Groups.Location_Timer, true); 
    }

    void Spatial_SetData(string a, float b, string c, int d, int e, int f, string g, vector h, vector i){
        Spatial_Name = a;
        Spatial_TriggerRadius = b;
        Spatial_ZoneLoadout = c;
        Spatial_MinCount = d;
        Spatial_MaxCount = e;
        Spatial_HuntMode = f;
        Spatial_Faction = g;
        Spatial_TriggerPosition = h;
        Spatial_SpawnPosition = i;
        
    }

    void SpawnCheck(){
        i_PlayerCount = m_insiders.Count();
        if (dynPatrol) return;
        if (i_PlayerCount < 1) return;
            SpawnCount = Math.RandomIntInclusive(Spatial_MinCount, Spatial_MaxCount);
            Spatial_Spawn(SpawnCount, Spatial_Faction, Spatial_ZoneLoadout, Spatial_Name);        
    }

    override void Enter(TriggerInsider insider)
    {
        super.Enter(insider);
    }
        
    override void Leave(TriggerInsider insider)
    {
        super.Leave(insider);
    }
    
    override protected bool CanAddObjectAsInsider(Object object)
    {
		if (!super.CanAddObjectAsInsider(object))
		{
			return false;
		}
        return PlayerBase.Cast(object) != null;
    }

  //Spatial_Spawn(player, SpawnCount, faction, loadout)
  void Spatial_Spawn(int bod, string fac, string loa, string GroupName) {
    vector startpos = ValidPos();
    TVectorArray waypoints = {
      ValidPos()
    };
    string Formation = "RANDOM";
    eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
    int mindistradius, maxdistradius, despawnradius;
    mindistradius = 0;
    maxdistradius = 1200;
    despawnradius = 1200;
    bool UnlimitedReload = false;
    dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, loa, bod, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(fac), eAIFormation.Create(Formation), true, mindistradius, maxdistradius, despawnradius, 2, 3, Spatial_Lootable(), UnlimitedReload);
    if (dynPatrol) {
      dynPatrol.SetAccuracy(-1, -1);
      dynPatrol.SetGroupName(GroupName);
      dynPatrol.SetSniperProneDistanceThreshold(maxdistradius * 3);
      eAIGroup group = eAIGroup.Cast(dynPatrol.m_Group);
      if (!group) {
        return;
      }
      eAIBase ai = eAIBase.Cast(group.GetMember(0));
      Spatial_Movement(ai); //custom waypoints gen applied to ai member's group - no leader = no new waypoints gen
      SetGroupAccuracy(group); //sigh
      SetMembersLootable(group); //sigh
      GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.Spatial_PatrolCleanup, m_Spatial_Groups.CleanupTimer, false, dynPatrol, group, bod);
    } else {}
  }

  //no group accuracy setting for after group has init
  void SetGroupAccuracy(eAIGroup group) {
    if (!group) return;
    for (int i = 0; i < group.Count(); i++) {
      eAIBase ai = eAIBase.Cast(group.GetMember(i));
      if (!ai) return;
      ai.eAI_SetAccuracy(-1, -1);
    }
  }

  //sigh - needs changing
  bool Spatial_Lootable() {
    if (m_Spatial_Groups.Lootable < 2) {
      return m_Spatial_Groups.Lootable;
    }
    return true;
  }

  //no group loot setting for after group has init
  void SetMembersLootable(eAIGroup group) {
    if (!group) return;
    for (int i = 0; i < group.Count(); i++) {
      eAIBase ai = eAIBase.Cast(group.GetMember(i));
      if (!ai) return;
      switch (m_Spatial_Groups.Lootable) {
      case 0:
      case 1:
        //true-false
        ai.Expansion_SetCanBeLooted(m_Spatial_Groups.Lootable);
        break;
      case 2:
        //ranfom
        int r = Math.RandomIntInclusive(0, 1);
        ai.Expansion_SetCanBeLooted(r);
        break;
      case 3:
        //leader only
        if (i == 0) {
          ai.Expansion_SetCanBeLooted(true);
        } else {
          ai.Expansion_SetCanBeLooted(false);
        }
        break;
      }
    }
  }

  void Spatial_Movement(eAIBase ai) {
    eAIGroup AiGroup = eAIGroup.Cast(ai.GetGroup());
    if (!AiGroup) return;
    AiGroup.ClearWaypoints();
    if (Spatial_HuntMode >= 6) Spatial_HuntMode = 5;
    switch (Spatial_HuntMode) {
    case 1: {
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_SpawnPosition, 0, 3));
      break;
    }
    case 2: {
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_TriggerPosition, 0, 3));
      break;
    }
    case 3: {
      break;
    }
    case 4: {
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_SpawnPosition, 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_TriggerPosition, 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_TriggerPosition, 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_SpawnPosition, 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_TriggerPosition, 70, 80));
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_TriggerPosition, 70, 80));
      break;
    }
    case 5: {
      float c = m_Spatial_Groups.EngageTimer / 2500;
      for (int i = 0; i < c; i++) {
        int d = Math.RandomIntInclusive(0, 100);
        if (d < 16) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_SpawnPosition, 70, 120));
        if (d > 15 && d < 95) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_TriggerPosition, 80, 200));
        if (d > 94) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(Spatial_TriggerPosition, 10, 20));
      }
    }
    }
  }

  vector ValidPos() {
    vector pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(Spatial_SpawnPosition, m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
    float x, z;
    x = pos[0];
    z = pos[2];
    int i = 0;
    while (GetGame().SurfaceIsSea(x, z) || GetGame().SurfaceIsPond(x, z)) {
      pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(Spatial_SpawnPosition, m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
      x = pos[0];
      z = pos[2];
    }
    return pos;
  }

  void Spatial_PatrolCleanup(eAISpatialPatrol patrol, eAIGroup group, int count) {
    if (group) group.ClearAI();
    if (patrol) patrol.Delete();
  }

}
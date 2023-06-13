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
    PlayerBase playerInsider = PlayerBase.Cast(m_insiders.Get(0).GetObject());
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
    dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, loa, bod, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(fac), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, 2, 3, m_Spatial_Groups.Lootable, UnlimitedReload);
    if (dynPatrol) {
      dynPatrol.SetGroupName(GroupName);
      dynPatrol.SetSniperProneDistanceThreshold(maxdistradius * 3);
      dynPatrol.SetLocation();
    }
  }

  vector ValidPos() {
    return ExpansionStatic.GetSurfacePosition(Spatial_SpawnPosition);
  }


}
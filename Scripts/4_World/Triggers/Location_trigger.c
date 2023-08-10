class Location_Trigger: CylinderTrigger
{       

  int i_PlayerCount;
  vector Spatial_SpawnPosition;
  Spatial_Location location;
  ref Spatial_Groups m_Spatial_Groups;
  eAISpatialPatrol dynPatrol;
  bool Spatial_TimerCheck;

  void Location_Trigger(){
    GetSpatialSettings().PullRef(m_Spatial_Groups);   
  }

  void Spatial_SetData(Spatial_Location Location){
    location = Location;
  } //changed to class instead of individuals

  void SpawnCheck(){
    if (dynPatrol) return;
    if (Spatial_TimerCheck) return;
    i_PlayerCount = m_insiders.Count();
    if (i_PlayerCount == 0) return;
    int m_Groupid = Math.RandomIntInclusive(0, int.MAX);
    SpatialDebugPrint("LocationID: " + m_Groupid);
    float random = Math.RandomFloat(0.0, 1.0);
    SpatialDebugPrint("Location Chance: " + location.Spatial_Chance + " | random: " + random);
    if (location.Spatial_Chance < random) return;

    int SpawnCount = Math.RandomIntInclusive(location.Spatial_MinCount, location.Spatial_MaxCount);
    if (SpawnCount > 0) {
      Spatial_Spawn(SpawnCount, location);
      Spatial_TimerCheck = true;
      GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Spatial_timer, location.Spatial_Timer, false);
    } else {
      SpatialDebugPrint("Location ai count too low this check");
    }
    SpatialDebugPrint("End LocationID: " + m_Groupid);
  }

  void Spatial_timer(){
    Spatial_TimerCheck = false;
  }

  override void Enter(TriggerInsider insider){
    super.Enter(insider);
    SpawnCheck();
  }
      
  override void Leave(TriggerInsider insider){
    super.Leave(insider);
  }
  
  override protected bool CanAddObjectAsInsider(Object object){
    if (!super.CanAddObjectAsInsider(object)) return false;
    bool ai = eAIBase.Cast(object) != null;
    if (ai) return false;
    return PlayerBase.Cast(object) != null;
  }

  void Spatial_Spawn(int count, Spatial_Location Location){
    if (m_insiders.Count() == 0) return;
    PlayerBase playerInsider = PlayerBase.Cast(m_insiders.Get(0).GetObject());
    if (!playerInsider || playerInsider.IsAI()) return;
    SpatialDebugPrint(playerInsider.GetIdentity().GetName());
    vector startpos = ValidPos();
    TVectorArray waypoints = { ValidPos() };
    string Formation = "RANDOM";
    eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
    if (Location.Spatial_HuntMode == 3) 
      behaviour = typename.StringToEnum(eAIWaypointBehavior, "HALT");
    int mindistradius, maxdistradius, despawnradius;
    mindistradius = 0;
    maxdistradius = 1000;
    despawnradius = 1200;
    playerInsider.Spatial_SetLocationHunt(Location.Spatial_HuntMode);
    dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, Location.Spatial_ZoneLoadout, count, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(Location.Spatial_Faction), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, 2.0, 3.0, Location.Spatial_Lootable, Location.Spatial_UnlimitedReload);
    playerInsider.Spatial_SetLocationHunt(10);
    if (dynPatrol) {
      dynPatrol.SetAccuracy(Location.Spatial_MinAccuracy, Location.Spatial_MaxAccuracy);
      dynPatrol.SetGroupName(Location.Spatial_Name);
      dynPatrol.SetSniperProneDistanceThreshold(0.0);
      dynPatrol.SetLocation();
    }
  } //Spatial_Spawn(player, SpawnCount, faction, loadout, groupname, lootable)

  vector ValidPos(){
    if (m_Spatial_Groups.Locations_Enabled == 2) return location.Spatial_SpawnPosition;
    return ExpansionStatic.GetSurfacePosition(location.Spatial_SpawnPosition);
  }

  void SpatialDebugPrint(string msg) {
    if (m_Spatial_Groups.Spatial_MinTimer == 60000)
      GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
  } //expansion debug print
}
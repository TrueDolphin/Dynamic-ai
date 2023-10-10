//!TODO: change this whole system to avoidplayer
class Location_Trigger: Spatial_TriggerBase
{
    ref Spatial_Location location;
    Notification_Trigger notif_trigger;
    

    void Spatial_SetData(Spatial_Location Location, Notification_Trigger b)
    {
      location = Location;
      notif_trigger = b;
      TriggerName = Location.Spatial_Name;
      TriggerLoadout = Location.Spatial_ZoneLoadout;
      TriggerFaction = Location.Spatial_Faction;
    } //changed to class instead of individuals
    override void SpawnCheck()
    {
      if (!CanSpawn()) return;

      int m_Groupid = Math.RandomIntInclusive(5001, 10000);
      SpatialDebugPrint("LocationID: " + m_Groupid);
      float random = Math.RandomFloat(0.0, 1.0);
      SpatialDebugPrint("Location Chance: " + location.Spatial_Chance + " | random: " + random);
      if (location.Spatial_Chance < random) return;

      int SpawnCount = Math.RandomIntInclusive(location.Spatial_MinCount, location.Spatial_MaxCount);
      if (SpawnCount > 0)
      {
        Spatial_Spawn(SpawnCount);
        Spatial_TimerCheck = true;

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Spatial_timer, location.Spatial_Timer, false);
      } else {
        SpatialDebugPrint("Location ai count too low this check");
      }
      SpatialDebugPrint("End LocationID: " + m_Groupid);
    }

    override void Enter(TriggerInsider insider)
    {
      PlayerBase player = PlayerBase.Cast(insider.GetObject());
      if (player && location)
      {
        player.Spatial_InLocation(true, location.Spatial_HuntMode);
      } 
      super.Enter(insider);
    }
        
    override void Leave(TriggerInsider insider)
    {
      PlayerBase player = PlayerBase.Cast(insider.GetObject());
      if (player)
      {
        player.Spatial_InLocation(false, 0);
      }
      super.Leave(insider);
    }

    override void OnStayStartServerEvent(int nrOfInsiders)
    {
      super.OnStayStartServerEvent(nrOfInsiders);
      SpawnCheck();
    }

    //next plan - split spawn back into Spatial_Group and add a vector array check to Spatial_TriggerPosition
   override void Spatial_Spawn(int count)
    {
      Spatial_Location Location = location;
      if (m_insiders.Count() == 0) return;
      PlayerBase playerInsider = PlayerBase.Cast(m_insiders.Get(0).GetObject());
      if (!playerInsider || playerInsider.IsAI() || !playerInsider.GetIdentity()) return;
      SpatialDebugPrint(playerInsider.GetIdentity().GetName());

      string Formation = "RANDOM";
      eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
      if (Location.Spatial_HuntMode == 3) 
        behaviour = typename.StringToEnum(eAIWaypointBehavior, "HALT");
      vector startpos;
      TVectorArray waypoints;
      eAISpatialPatrol DynPatrol;
      int mindistradius, maxdistradius, despawnradius;
      mindistradius = 0;
      maxdistradius = 1000;
      despawnradius = 1200;

      if (Location.Spatial_SpawnMode == 0) 
      {
        vector interm = Location.Spatial_SpawnPosition.GetRandomElement();
        startpos = ValidPos(m_Spatial_Groups.Locations_Enabled, interm);
        waypoints = { ValidPos(m_Spatial_Groups.Locations_Enabled, interm) };
        DynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, Location.Spatial_ZoneLoadout, count, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(Location.Spatial_Faction), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, Location.Spatial_HuntMode, 3.0, Location.Spatial_Lootable, Location.Spatial_UnlimitedReload);
        if (DynPatrol)
        {
          DynPatrol.SetAccuracy(Location.Spatial_MinAccuracy, Location.Spatial_MaxAccuracy);
          DynPatrol.SetGroupName(Location.Spatial_Name);
          DynPatrol.SetSniperProneDistanceThreshold(0.0);
          DynPatrol.SetLocation();
          dynPatrol.Insert(DynPatrol);

    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, startpos), location.Spatial_Name + " Spawn", m_Spatial_Groups.CleanupTimer);
    #endif
        } 
      }

      if (Location.Spatial_SpawnMode == 1)
      {
        int recount = 0;
        foreach (vector pos : Location.Spatial_SpawnPosition)
        {
          startpos = ValidPos(m_Spatial_Groups.Locations_Enabled, pos);
          waypoints = { ValidPos(m_Spatial_Groups.Locations_Enabled, pos) };
          DynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, Location.Spatial_ZoneLoadout, count, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(Location.Spatial_Faction), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, Location.Spatial_HuntMode, 3.0, Location.Spatial_Lootable, Location.Spatial_UnlimitedReload);
          if (DynPatrol)
          {
            DynPatrol.SetAccuracy(Location.Spatial_MinAccuracy, Location.Spatial_MaxAccuracy);
            DynPatrol.SetGroupName(Location.Spatial_Name);
            DynPatrol.SetSniperProneDistanceThreshold(0.0);
            DynPatrol.SetLocation();
            recount += count;
            dynPatrol.Insert(DynPatrol);
      #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, startpos), location.Spatial_Name + " Spawn", m_Spatial_Groups.CleanupTimer);
      #endif
          }
        }
        count = recount;
      }

      array<ref TriggerInsider> notif = notif_trigger.GetInsiders();
      for (int i = 0; i < notif.Count(); ++i)
      {
        PlayerBase player = PlayerBase.Cast(notif[i].GetObject());
        if (player) Spatial_message(player, count);
      }
    }

}
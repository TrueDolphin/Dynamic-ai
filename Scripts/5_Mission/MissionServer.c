/*
name:TrueDolphin
date:14/3/2023
spatial ai spawns

this is becoming unwieldly again on server performance per check.
triggers helped a lot, but idk what else to do as far as optimisation goes besides notes.
stripped a lot of useless stuff out when moved settings back to 3_Game.
changed over to CreateEx

*/
modded class MissionServer {
  const int SZ_IN_SAFEZONE = 0x0001;
  int m_cur = 0;
  int Spatial_Spawncount = 0;
  int m_Spatial_cur = 0;
  ref array < Man > Spatial_PlayerList = new array < Man > ;
  ref Spatial_Groups m_Spatial_Groups;

  #ifdef EXPANSIONMODSPAWNSELECTION
  private ExpansionRespawnHandlerModule m_RespawnModule;
  #endif

  //init
  void MissionServer() {

    if (GetSpatialSettings().Init() == true) {
      GetSpatialSettings().PullRef(m_Spatial_Groups);
      InitSpatialTriggers();
      SpatialLoggerPrint("Spatial AI Enabled");
      SpatialTimer();
    }
  }

  //timer call for varied check loops
  void SpatialTimer() {
    Spatial_Check();
    int m_cor = Math.RandomIntInclusive(m_Spatial_Groups.Spatial_MinTimer, m_Spatial_Groups.Spatial_MaxTimer);
    SpatialLoggerPrint("Next valid check in: " + m_cor + "ms");
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SpatialTimer, m_cor, false);
  }

  //standard loop through playerlist pulling randomly and removing that from list.
  void Spatial_Check() {
    eAIGroup PlayerGroup;
    GetGame().GetPlayers(Spatial_PlayerList);
    if (Spatial_PlayerList.Count() == 0) return;
    for (int i = 0; i < Spatial_PlayerList.Count(); i++) {
      PlayerBase player = PlayerBase.Cast(Spatial_PlayerList.GetRandomElement());
      Spatial_PlayerList.RemoveItem(player);
      if (m_Spatial_Groups.Chance < Math.RandomFloat(0.0, 1.0)) continue;  
      PlayerGroup = eAIGroup.Cast(player.GetGroup());
      if (!PlayerGroup) PlayerGroup = eAIGroup.GetGroupByLeader(player);
      if (player != player.GetGroup().GetLeader()) continue;
      if (!player || !player.IsAlive() || !player.GetIdentity()) continue;
      #ifdef EXPANSIONMODSPAWNSELECTION
      if (InRespawnMenu(player.GetIdentity())) continue;
      #endif
      //this is shitty..
      if (CanSpawn(player)) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.LocalSpawn, Math.RandomIntInclusive(500, 2000), false, player);
      if (m_Spatial_Groups.PlayerChecks != 0) {
        if (i == m_Spatial_Groups.PlayerChecks) return;
      }
    }
  }

  //checks and overrides
  //this is slow i think. but idk how to optimise this
  bool CanSpawn(PlayerBase player) {
    PlayerIdentity identity = player.GetIdentity();
    bool valid = true;

    #ifdef EXPANSIONMODBASEBUILDING
    if (player.IsInsideOwnTerritory() && !GetSpatialSettings().Spatial_InOwnTerritory()) {
      return false;
    }
    #endif
    //main overrides checks
    if (player.IsInVehicle() && !GetSpatialSettings().Spatial_InVehicle() || player.Expansion_IsInSafeZone() && !GetSpatialSettings().Spatial_IsInSafeZone() || player.IsBleeding() && !GetSpatialSettings().Spatial_IsBleeding() || player.IsRestrained() && !GetSpatialSettings().Spatial_IsRestrained() || player.IsUnconscious() && !GetSpatialSettings().Spatial_IsUnconscious()) {
      return false;
    }

    #ifdef SZDEBUG
    if (player.GetSafeZoneStatus() == SZ_IN_SAFEZONE && !GetSpatialSettings().Spatial_TPSafeZone()) {
      return false;
    }
    #endif

    if (m_Spatial_Groups.Points_Enabled != 0 && player.CheckSafe() == true) {
      return false;
    }

    if (m_Spatial_Groups.Points_Enabled == 2 && !player.CheckZone()) {
      return false;
    }

    if (Spatial_Spawncount > m_Spatial_Groups.MaxAI) {
      return false;
    }

    return valid;
  }

  //create and stuff.
  //moved to custom patrol
  void LocalSpawn(PlayerBase player) {
    if (!player) return;
    m_cur = Math.RandomIntInclusive(0, m_Spatial_Groups.Group.Count() - 1);
    int SpawnCount;
    Spatial_Group group;
    string faction, loadout, name;
    if (player.CheckZone() == true) {
      SpawnCount = Math.RandomIntInclusive(player.Spatial_MinCount, player.Spatial_MaxCount);
      faction = player.Spatial_Faction();
      loadout = player.Spatial_Loadout();
      name = player.Spatial_Name();
    } else {
      group = GetWeightedGroup(m_Spatial_Groups.Group);
      SpawnCount = Math.RandomIntInclusive(group.Spatial_MinCount, group.Spatial_MaxCount);
      faction = group.Spatial_Faction;
      loadout = group.Spatial_Loadout;
      name = group.Spatial_Name;
    }
    if (SpawnCount > 0) {
      if (m_Spatial_Groups.GroupDifficulty == 1){
        eAIGroup PlayerGroup;
        PlayerGroup = eAIGroup.Cast(player.GetGroup());
        if (!PlayerGroup) PlayerGroup = eAIGroup.GetGroupByLeader(player);
        if (PlayerGroup.Count() > 1) SpawnCount += (PlayerGroup.Count() - 1);
      }
      Spatial_Spawncount += SpawnCount;
      Spatial_message(player, m_Spatial_Groups.MessageType, SpawnCount, faction, loadout);
      Spatial_Spawn(player, SpawnCount, faction, loadout, name);
    }
  }

  //expansion lightweight weighted group calcs
  Spatial_Group GetWeightedGroup(array < ref Spatial_Group > groups, array < float > weights = NULL) {
    array < float > weights_T = weights;
    if (weights_T == NULL) {
      weights_T = new array < float > ;
      for (int i = 0; i < groups.Count(); i++) {
        weights_T.Insert(groups[i].Spatial_Weight);
      }
    }
    int index = ExpansionStatic.GetWeightedRandom(weights_T);
    if (index > -1)
      return groups[index];
    //! Should not happen
    Print("[Spatial_Group] GetWeightedGroup: All Groups have a 'Weight' of zero. Selecting pure random group instead.");
    return groups.GetRandomElement();
  }

  //could be scuffed
  vector ValidPos(PlayerBase player) {
    vector pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
    float x, z;
    x = pos[0];
    z = pos[2];
    int i = 0;
    while (GetGame().SurfaceIsSea(x, z) || GetGame().SurfaceIsPond(x, z)) {
      pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
      x = pos[0];
      z = pos[2];
      //i++; 
      //SpatialLoggerPrint(i.ToString());
    }
    //SpatialLoggerPrint("ValidPos LoopCount :" + i.ToString());
    return pos;
  }

  //Hunt parse
  //Spatial_Movement(ai, player)
  void Spatial_Movement(eAIBase ai, PlayerBase player) {
    eAIGroup AiGroup = eAIGroup.Cast(ai.GetGroup());
    if (!AiGroup) return;
    AiGroup.ClearWaypoints();
    int m_Mode;
    if (player.CheckZone() == true) {
      m_Mode = player.Spatial_HuntMode();
    } else {
      m_Mode = m_Spatial_Groups.HuntMode;
    }

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
      // just spawn, dont chase unless standard internal contitions met.

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

  //less code than triggers.
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

  //Spatial_Spawn(player, SpawnCount, faction, loadout)
  void Spatial_Spawn(PlayerBase player, int bod, string fac, string loa, string GroupName) {
    vector startpos = ValidPos(player);
    TVectorArray waypoints = {
      ValidPos(player)
    };
    string Formation = "RANDOM";
    eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
    int mindistradius, maxdistradius, despawnradius;
    mindistradius = 0;
    maxdistradius = 1200;
    despawnradius = 1200;
    bool UnlimitedReload = false;
    auto dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, loa, bod, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(fac), eAIFormation.Create(Formation), true, mindistradius, maxdistradius, despawnradius, 2, 3, false, UnlimitedReload);
    if (dynPatrol) {
      dynPatrol.SetAccuracy(-1, -1);
      dynPatrol.SetGroupName(GroupName);
      dynPatrol.SetSniperProneDistanceThreshold(maxdistradius * 3);
      eAIGroup group = eAIGroup.Cast(dynPatrol.m_Group);
      if (!group) {
        return;
      }
      eAIBase ai = eAIBase.Cast(group.GetMember(0));
      Spatial_Movement(ai, player); //custom waypoints gen applied to ai member's group - no leader = no new waypoints gen
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

  //trigger zone initialisation
  //fine i guess
  void InitSpatialTriggers() {
    if (m_Spatial_Groups.Points_Enabled == 1 || m_Spatial_Groups.Points_Enabled == 2) {
      SpatialLoggerPrint("points Enabled");
      foreach(Spatial_Point points: m_Spatial_Groups.Point) {
        Spatial_Trigger spatial_trigger = Spatial_Trigger.Cast(GetGame().CreateObjectEx("Spatial_Trigger", points.Spatial_Position, ECE_NONE));
        spatial_trigger.SetCollisionCylinder(points.Spatial_Radius, points.Spatial_Radius / 2);
        spatial_trigger.Spatial_SetData(points.Spatial_Safe, points.Spatial_Faction, points.Spatial_ZoneLoadout, points.Spatial_MinCount, points.Spatial_MaxCount, points.Spatial_HuntMode, points.Spatial_Name);
        SpatialLoggerPrint("Trigger at location: " + points.Spatial_Position + " - Radius: " + points.Spatial_Radius);
        SpatialLoggerPrint("Safe: " + points.Spatial_Safe + " - Faction: " + points.Spatial_Faction + " - Loadout: " + points.Spatial_ZoneLoadout + " - counts: " + points.Spatial_MinCount + ":" + points.Spatial_MaxCount);
      }
    } else {
      SpatialLoggerPrint("points Disabled");
    }

    if (m_Spatial_Groups.Locations_Enabled == 1) {
      SpatialLoggerPrint("Locations Enabled");
      foreach(Spatial_Location location: m_Spatial_Groups.Location) {
        Location_Trigger location_trigger = Location_Trigger.Cast(GetGame().CreateObjectEx("Location_Trigger", location.Spatial_TriggerPosition, ECE_NONE));
        location_trigger.SetCollisionCylinder(location.Spatial_TriggerRadius, location.Spatial_TriggerRadius / 2);
        location_trigger.Spatial_SetData(location.Spatial_Name, location.Spatial_TriggerRadius, location.Spatial_ZoneLoadout, location.Spatial_MinCount, location.Spatial_MaxCount, location.Spatial_HuntMode, location.Spatial_Faction, location.Spatial_TriggerPosition, location.Spatial_SpawnPosition);
        SpatialLoggerPrint("Trigger at location: " + location.Spatial_TriggerPosition + " - Radius: " + location.Spatial_TriggerRadius + " - Spawn location: " + location.Spatial_SpawnPosition);
        SpatialLoggerPrint("Faction: " + location.Spatial_Faction + " - Loadout: " + location.Spatial_ZoneLoadout + " - counts: " + location.Spatial_MinCount + ":" + location.Spatial_MaxCount);
      }
    } else {
      SpatialLoggerPrint("Locations Disabled");
    }
  }

  //chat message or vanilla notification
  //changed to format. tidy up.
  void Spatial_message(PlayerBase player, int msg_no, int SpawnCount, string faction, string loadout) {
    if (!player) return;
    string message = string.Format("Player: %1 Number: %2, Faction name: %3, Loadout: %4", player.GetIdentity().GetName(), SpawnCount, faction, loadout);
    if (msg_no == 0) {
      SpatialLoggerPrint(message);
    } else if (msg_no == 1) {
      WarningMessage(player, string.Format("%1 %2", SpawnCount, m_Spatial_Groups.MessageText));
      SpatialLoggerPrint(message);
    } else if (msg_no == 2) {
      WarningMessage(player, m_Spatial_Groups.MessageText);
      SpatialLoggerPrint(message);
    } else if (msg_no == 3) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, m_Spatial_Groups.MessageTitle, string.Format("%1 %2", SpawnCount, m_Spatial_Groups.MessageText), "set:dayz_gui image:tutorials");
      SpatialLoggerPrint(message);
    } else if (msg_no == 4) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, m_Spatial_Groups.MessageTitle, m_Spatial_Groups.MessageText, "set:dayz_gui image:tutorials");
      SpatialLoggerPrint(message);
    }
  }

  // Ingame chat message
  void WarningMessage(PlayerBase player, string message) {
    if ((player) && (message != "")) {
      Param1 < string > Msgparam;
      Msgparam = new Param1 < string > (message);
      GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
    }
  }

  //expansion logging
  void SpatialLoggerPrint(string msg) {
    if (GetExpansionSettings().GetLog().AIGeneral)
      GetExpansionSettings().GetLog().PrintLog("[Spatial AI] " + msg);
  }

  //required
  void Spatial_PatrolCleanup(eAISpatialPatrol patrol, eAIGroup group, int count) {
    Spatial_Spawncount -= count;
    if (group) group.ClearAI();
    if (patrol) patrol.Delete();
  }

  #ifdef EXPANSIONMODSPAWNSELECTION
  //LM's code altered to a bool in class
  bool InRespawnMenu(PlayerIdentity identity) {
    CF_Modules < ExpansionRespawnHandlerModule > .Get(m_RespawnModule);
    if (m_RespawnModule && m_RespawnModule.IsPlayerInSpawnSelect(identity)) {
      return true;
    }
    return false;
  }
  #endif

};
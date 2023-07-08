/*
name:TrueDolphin
date:8/7/2023
spatial ai spawns

triggers helped a lot, but idk what else to do as far as optimisation goes besides notes.
stripped a lot of useless stuff out when moved settings back to 3_Game.

*/
modded class MissionServer {
  const int SZ_IN_SAFEZONE = 0x0001;
  int m_cur = 0;
  int m_Spatial_cur = 0;
  ref array < Man > Spatial_PlayerList = new array < Man > ;
  ref Spatial_Groups m_Spatial_Groups;

  #ifdef EXPANSIONMODSPAWNSELECTION
    private ExpansionRespawnHandlerModule m_RespawnModule;
  #endif


  void MissionServer() {

    if (GetSpatialSettings().Init() == true) {
      GetSpatialSettings().PullRef(m_Spatial_Groups);
      InitSpatialTriggers();
      SpatialLoggerPrint("Spatial AI Enabled");
      if (m_Spatial_Groups.Spatial_MinTimer == 60000) SpatialLoggerPrint("Spatial Debug Mode on");
      SpatialTimer();
    }
  } //constructor

  void SpatialTimer() {
    Spatial_Check();
    int m_cor = Math.RandomIntInclusive(m_Spatial_Groups.Spatial_MinTimer, m_Spatial_Groups.Spatial_MaxTimer);
    SpatialLoggerPrint("Next valid check in: " + m_cor + "ms");
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SpatialTimer, m_cor, false);
  } //timer call for varied check loops

  void Spatial_Check() {
    eAIGroup PlayerGroup;
    GetGame().GetPlayers(Spatial_PlayerList);
    if (Spatial_PlayerList.Count() == 0) return;
    for (int i = 0; i < Spatial_PlayerList.Count(); i++) {
      PlayerBase player = PlayerBase.Cast(Spatial_PlayerList.GetRandomElement());
      Spatial_PlayerList.RemoveItem(player);

      if (m_Spatial_Groups.PlayerChecks > -1){
        PlayerGroup = eAIGroup.Cast(player.GetGroup());
        if (!PlayerGroup) PlayerGroup = eAIGroup.GetGroupByLeader(player);
        if (player != player.GetGroup().GetLeader()) continue;
      }
      if (!player || !player.IsAlive() || !player.GetIdentity()) continue;
      #ifdef EXPANSIONMODSPAWNSELECTION
      if (InRespawnMenu(player.GetIdentity())) continue;
      #endif
      //this is shitty..
      if (CanSpawn(player)) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.LocalSpawn, Math.RandomIntInclusive(500, 2000), false, player);
      
      if (m_Spatial_Groups.PlayerChecks > 0){
        if (i == m_Spatial_Groups.PlayerChecks) return;
      } else if (m_Spatial_Groups.PlayerChecks < 0){
        if (i == Math.AbsInt(m_Spatial_Groups.PlayerChecks)) return;
      }
    }
  } //standard loop through playerlist pulling randomly and removing that from list.

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

    return valid;
  } //checks and overrides

  void LocalSpawn(PlayerBase player) {
    if (!player) return;

    int m_Groupid = Math.RandomIntInclusive(0, 5000);
    SpatialDebugPrint("GroupID: " + m_Groupid);

    m_cur = Math.RandomIntInclusive(0, m_Spatial_Groups.Group.Count() - 1);
    int SpawnCount, lootable;
    Spatial_Group group;
    string faction, loadout, name;
    float chance;
    if (player.CheckZone() == true) {
      SpawnCount = Math.RandomIntInclusive(player.Spatial_MinCount, player.Spatial_MaxCount);
      faction = player.Spatial_Faction();
      loadout = player.Spatial_Loadout();
      name = player.Spatial_Name();
      lootable = player.Spatial_Lootable();
      chance = player.Spatial_Chance();
      //ammo = player.Spatial_UnlimitedReload()
    } else {
      group = GetWeightedGroup(m_Spatial_Groups.Group);
      SpawnCount = Math.RandomIntInclusive(group.Spatial_MinCount, group.Spatial_MaxCount);
      faction = group.Spatial_Faction;
      loadout = group.Spatial_Loadout;
      name = group.Spatial_Name;
      lootable = group.Spatial_Lootable;
      chance = group.Spatial_Chance;
      //ammo = group.Spatial_UnlimitedReload
    }

    float random = Math.RandomFloat(0.0, 1.0);
    SpatialDebugPrint("Chance: " + chance + " | random: " + random);
    if (chance < random) return;

    if (SpawnCount > 0) {
      if (m_Spatial_Groups.GroupDifficulty == 1) {
        eAIGroup PlayerGroup;
        PlayerGroup = eAIGroup.Cast(player.GetGroup());
        if (!PlayerGroup) PlayerGroup = eAIGroup.GetGroupByLeader(player);
        if (PlayerGroup.Count() > 1) SpawnCount += (PlayerGroup.Count() - 1);
      }
    Spatial_message(player, m_Spatial_Groups.MessageType, SpawnCount, faction, loadout);
    Spatial_Spawn(player, SpawnCount, faction, loadout, name, lootable /*, ammo*/);
    } else {
      SpatialDebugPrint("group/point ai count too low this check");
    }

    SpatialDebugPrint("End GroupID: " + m_Groupid);

  } //create and stuff.

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
  } //expansion lightweight weighted group calcs

  vector ValidPos(PlayerBase player) {
    vector pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
    float x, z;
    x = pos[0];
    z = pos[2];
    for (int i = 0; i < 50; i++) {
      if (GetGame().SurfaceIsSea(x, z) || GetGame().SurfaceIsPond(x, z)) {
        pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
        x = pos[0];
        z = pos[2];
      } else {
          i = 50;
      }
    }
    return pos;
  } //could be scuffed

  void Spatial_Spawn(PlayerBase player, int bod, string fac, string loa, string GroupName, int Lootable /*, bool UnlimitedReload*/ ) {
    vector startpos = ValidPos(player);
    TVectorArray waypoints = { ValidPos(player) };
    string Formation = "RANDOM";
    eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
    int mindistradius, maxdistradius, despawnradius;
    mindistradius = 0;
    maxdistradius = 1200;
    despawnradius = 1200;
    bool UnlimitedReload = false; //Remove - prepped
    auto dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, loa, bod, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(fac), eAIFormation.Create(Formation), player, mindistradius, maxdistradius, despawnradius, 2.0, 3.0, Lootable, UnlimitedReload);
    if (dynPatrol) {
      dynPatrol.SetGroupName(GroupName);
      dynPatrol.SetSniperProneDistanceThreshold(0.0);
      dynPatrol.SetHunted(player);
    }
  } //Spatial_Spawn(player, SpawnCount, faction, loadout)

  void InitSpatialTriggers() {
    if (m_Spatial_Groups.Points_Enabled == 1 || m_Spatial_Groups.Points_Enabled == 2) {
      SpatialLoggerPrint("points Enabled");
      foreach(Spatial_Point points: m_Spatial_Groups.Point) {
        Spatial_Trigger spatial_trigger = Spatial_Trigger.Cast(GetGame().CreateObjectEx("Spatial_Trigger", points.Spatial_Position, ECE_NONE));
        spatial_trigger.SetCollisionCylinder(points.Spatial_Radius, points.Spatial_Radius / 2);
        spatial_trigger.Spatial_SetData(points.Spatial_Safe, points.Spatial_Faction, points.Spatial_ZoneLoadout, points.Spatial_MinCount, points.Spatial_MaxCount, points.Spatial_HuntMode, points.Spatial_Name, points.Spatial_Lootable, points.Spatial_Chance /*, points.Spatial_UnlimitedReload*/);
        SpatialLoggerPrint("Trigger at location: " + points.Spatial_Position + " - Radius: " + points.Spatial_Radius);
        SpatialLoggerPrint("Safe: " + points.Spatial_Safe + " - Faction: " + points.Spatial_Faction + " - Loadout: " + points.Spatial_ZoneLoadout + " - counts: " + points.Spatial_MinCount + ":" + points.Spatial_MaxCount);
      }
    } else {
      SpatialLoggerPrint("points Disabled");
    }

    if (m_Spatial_Groups.Locations_Enabled != 0) {
      SpatialLoggerPrint("Locations Enabled");
      foreach(Spatial_Location location: m_Spatial_Groups.Location) {
        Location_Trigger location_trigger = Location_Trigger.Cast(GetGame().CreateObjectEx("Location_Trigger", location.Spatial_TriggerPosition, ECE_NONE));
        location_trigger.SetCollisionCylinder(location.Spatial_TriggerRadius, location.Spatial_TriggerRadius / 2);
        location_trigger.Spatial_SetData(location.Spatial_Name, location.Spatial_TriggerRadius, location.Spatial_ZoneLoadout, location.Spatial_MinCount, location.Spatial_MaxCount, location.Spatial_HuntMode, location.Spatial_Faction, location.Spatial_TriggerPosition, location.Spatial_SpawnPosition, location.Spatial_Lootable, location.Spatial_Timer, location.Spatial_Chance /*, location.Spatial_UnlimitedReload*/);
        SpatialLoggerPrint("Trigger at location: " + location.Spatial_TriggerPosition + " - Radius: " + location.Spatial_TriggerRadius + " - Spawn location: " + location.Spatial_SpawnPosition);
        SpatialLoggerPrint("Faction: " + location.Spatial_Faction + " - Loadout: " + location.Spatial_ZoneLoadout + " - counts: " + location.Spatial_MinCount + ":" + location.Spatial_MaxCount);
      }
    } else {
      SpatialLoggerPrint("Locations Disabled");
    }
  } //trigger zone initialisation

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
  } //chat message or vanilla notification

  void WarningMessage(PlayerBase player, string message) {
    if ((player) && (message != "")) {
      Param1 < string > Msgparam;
      Msgparam = new Param1 < string > (message);
      GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
    }
  } // Ingame chat message

  void SpatialLoggerPrint(string msg) {
    if (GetExpansionSettings().GetLog().AIGeneral)
      GetExpansionSettings().GetLog().PrintLog("[Spatial AI] " + msg);
  } //expansion logging

  void SpatialDebugPrint(string msg) {
    if (m_Spatial_Groups.Spatial_MinTimer == 60000)
      GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
  } //expansion debug print


  #ifdef EXPANSIONMODSPAWNSELECTION

    bool InRespawnMenu(PlayerIdentity identity) {
      CF_Modules < ExpansionRespawnHandlerModule > .Get(m_RespawnModule);
      if (m_RespawnModule && m_RespawnModule.IsPlayerInSpawnSelect(identity)) {
        return true;
      }
      return false;
    } //LM's code altered to a bool in class
  #endif

};
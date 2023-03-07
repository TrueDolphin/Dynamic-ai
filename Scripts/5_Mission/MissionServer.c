/*
name:TrueDolphin
date:07/03/2023
dynamic ai spawns

this is becoming unwieldly again on server performance per check.
triggers helped a lot, but idk what else to do as far as optimisation goes besides notes.
stripped a lot of useless stuff out when moved settings back to 3_Game.
changed over to CreateEx

*/
modded class MissionServer {
  ref array < ref ExpansionAIPatrol > Patrols;
  const int SZ_IN_SAFEZONE = 0x0001;
  int m_cur = 0;
  int Dynamic_Spawncount = 0;
  int m_Dynamic_cur = 0;
  ref array < Man > Dynamic_PlayerList = new array < Man > ;
  ref Dynamic_Groups m_Dynamic_Groups;

  ref ExpansionAIPatrolManager AIPatrolManager;

  #ifdef EXPANSIONMODSPAWNSELECTION
  private ExpansionRespawnHandlerModule m_RespawnModule;
  #endif

  void MissionServer() {

    if (GetDynamicSettings().Init() == true) {
      GetDynamicSettings().PullRef(m_Dynamic_Groups);
      InitDynamicTriggers();
      LoggerDynPrint("Dynamic AI Enabled");
      DynamicTimer();
      AIPatrolManager = new ExpansionAIPatrolManager;
    }
  }

  //timer call for varied check loops
  void DynamicTimer() {
    Dynamic_Check();
    int m_cor = Math.RandomIntInclusive(m_Dynamic_Groups.Dynamic_MinTimer, m_Dynamic_Groups.Dynamic_MaxTimer);
    LoggerDynPrint("Next valid check in: " + m_cor + "ms");
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DynamicTimer, m_cor, false);
  }

  //step 2 check
  //standard loop through playerlist pulling randomly and removing that from list.
  void Dynamic_Check() {
    GetGame().GetPlayers(Dynamic_PlayerList);
    if (Dynamic_PlayerList.Count() == 0) return;
    for (int i = 0; i < Dynamic_PlayerList.Count(); i++) {
      PlayerBase player = PlayerBase.Cast(Dynamic_PlayerList.GetRandomElement());
      Dynamic_PlayerList.RemoveItem(player);
      if (!player || !player.IsAlive() || !player.GetIdentity()) continue;
      #ifdef EXPANSIONMODSPAWNSELECTION
      if (InRespawnMenu(player.GetIdentity())) continue;
      #endif
      //this is shitty..)
      if (CanSpawn(player)) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.LocalSpawn, Math.RandomIntInclusive(500, 2000), false, player);
      if (m_Dynamic_Groups.PlayerChecks != 0) {
        if (i == m_Dynamic_Groups.PlayerChecks) return;
      }
    }
  }

  //checks and overrides
  //this is slow i think. but idk how to optimise this
  bool CanSpawn(PlayerBase player) {
    PlayerIdentity identity = player.GetIdentity();
    bool valid = true;

    #ifdef EXPANSIONMODBASEBUILDING
    if (player.IsInsideOwnTerritory() && !GetDynamicSettings().Dynamic_InOwnTerritory()) {
      return false;
    }
    #endif
    //main overrides checks
    if (player.IsInVehicle() && !GetDynamicSettings().Dynamic_InVehicle() || player.Expansion_IsInSafeZone() && !GetDynamicSettings().Dynamic_IsInSafeZone() || player.IsBleeding() && !GetDynamicSettings().Dynamic_IsBleeding() || player.IsRestrained() && !GetDynamicSettings().Dynamic_IsRestrained() || player.IsUnconscious() && !GetDynamicSettings().Dynamic_IsUnconscious()) {
      return false;
    }

    #ifdef SZDEBUG
    if (player.GetSafeZoneStatus() == SZ_IN_SAFEZONE && !GetDynamicSettings().Dynamic_TPSafeZone()) {
      return false;
    }
    #endif

    if (m_Dynamic_Groups.Points_Enabled == 1 && player.CheckSafe()) {
      return false;
    }
    if (Dynamic_Spawncount > m_Dynamic_Groups.MaxAI) {
      return false;
    }

    return valid;
  }

  //create and stuff.
  //moved to patrols
  void LocalSpawn(PlayerBase player) {
    if (!player) return;
    m_cur = Math.RandomIntInclusive(0, m_Dynamic_Groups.Group.Count() - 1);
    int SpawnCount;
    string faction, loadout;
    vector m_pos;
    if (player.CheckZone() == true) {
      SpawnCount = Math.RandomIntInclusive(player.Dynamic_MinCount, player.Dynamic_MaxCount);
      faction = player.Dynamic_Faction();
      loadout = player.Dynamic_Loadout();
    } else {
      SpawnCount = Math.RandomIntInclusive(m_Dynamic_Groups.Group[m_cur].Dynamic_MinCount, m_Dynamic_Groups.Group[m_cur].Dynamic_MaxCount);
      faction = m_Dynamic_Groups.Group[m_cur].Dynamic_Faction;
      loadout = m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout;
    }
    m_pos = ValidPos(player);
    if (SpawnCount > 0) {
      if (!m_pos) return;
      Dynamic_Spawncount += SpawnCount;
      Dynamic_message(player, m_Dynamic_Groups.MessageType, SpawnCount, faction, loadout);
      Dynamic_Spawn(player, SpawnCount, faction, loadout);
    }
  }

  //could be scuffed
  vector ValidPos(PlayerBase player) {
    vector pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Dynamic_Groups.MinDistance, m_Dynamic_Groups.MaxDistance)));
    float x, z;
    x = pos[0];
    z = pos[2];
    int i = 0;
    while (GetGame().SurfaceIsSea(x, z) || GetGame().SurfaceIsPond(x, z)) {
      pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Dynamic_Groups.MinDistance, m_Dynamic_Groups.MaxDistance)));
      x = pos[0];
      z = pos[2];
      //i++; 
      //LoggerDynPrint(i.ToString());
    }
    //LoggerDynPrint("ValidPos LoopCount :" + i.ToString());
    return pos;
  }

  //Hunt parse
  void Dynamic_Movement(eAIBase ai, PlayerBase player) {
    eAIGroup AiGroup = eAIGroup.Cast(ai.GetGroup());
    if (!group) return;
    AiGroup.ClearWaypoints();
    switch (m_Dynamic_Groups.HuntMode) {
    case 1: {
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 0, 3));
      player.GetTargetInformation().AddAI(ai, m_Dynamic_Groups.EngageTimer);
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
      float c = m_Dynamic_Groups.EngageTimer / 2500;
      for (int i = 0; i < c; i++) {
        int d = Math.RandomIntInclusive(0, 100);
        if (d < 16) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 120));
        if (d > 15 && d < 95) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 80, 200));
        if (d > 94) ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 10, 20));
      }
    }
    case 6: {
      //curious idea..
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 50, 55));
      GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TrailingGroup, 15000, false, ai, player, Vector(0, 0, 0), 15000);
    }
    }
  }

  //less code than triggers.
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

  //Dynamic_Spawn(player, SpawnCount, faction, loadout)
  void Dynamic_Spawn(PlayerBase player, int bod, string fac, string loa) {
    vector startpos = ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 120);
    TVectorArray waypoints = {
      ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 120)
    };
    string Formation = "RANDOM";
    eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
    int mindistradius, maxdistradius, despawnradius;
    mindistradius = 0;
    maxdistradius = 1200;
    despawnradius = 1200;
    bool UnlimitedReload = false;
    auto dynPatrol = eAIDynamicPatrol.CreateEx(startpos, waypoints, behaviour, loa, bod, m_Dynamic_Groups.CleanupTimer + 500, m_Dynamic_Groups.CleanupTimer - 500, eAIFaction.Create(fac), eAIFormation.Create(Formation), true, mindistradius, maxdistradius, despawnradius, 2, 3, false, UnlimitedReload);
    if (dynPatrol) {
      dynPatrol.SetAccuracy(-1, -1);
      eAIGroup group = eAIGroup.Cast(dynPatrol.m_Group);
      if (!group) {
        return;
      }
      eAIBase ai = eAIBase.Cast(group.GetMember(0));
      Dynamic_Movement(ai, player); //custom waypoint gen applied to ai member's group - no leader = no new waypoint gen
      SetGroupAccuracy(group); //sigh
      SetMembersLootable(group); //sigh
      GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.Dynamic_PatrolCleanup, m_Dynamic_Groups.CleanupTimer, false, dynPatrol, group, bod);
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
      switch (m_Dynamic_Groups.Lootable) {
      case 0:
        ai.Expansion_SetCanBeLooted(m_Dynamic_Groups.Lootable);
        break;
      case 1:
        ai.Expansion_SetCanBeLooted(m_Dynamic_Groups.Lootable);
        break;
      case 2:
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
  void InitDynamicTriggers() {
    if (m_Dynamic_Groups.Points_Enabled == 1) {
      LoggerDynPrint("Points Enabled");
      foreach(Dynamic_Point point: m_Dynamic_Groups.Point) {
        Dynamic_Trigger dynamic_trigger = Dynamic_Trigger.Cast(GetGame().CreateObjectEx("Dynamic_Trigger", point.Dynamic_Position, ECE_NONE));
        dynamic_trigger.SetCollisionCylinder(point.Dynamic_Radius, point.Dynamic_Radius / 2);
        dynamic_trigger.Dynamic_SetData(point.Dynamic_Safe, point.Dynamic_Faction, point.Dynamic_ZoneLoadout, point.Dynamic_MinCount, point.Dynamic_MaxCount);
        LoggerDynPrint("Trigger at location: " + point.Dynamic_Position + " - Radius: " + point.Dynamic_Radius);
        LoggerDynPrint("Safe: " + point.Dynamic_Safe + " - Faction: " + point.Dynamic_Faction + " - Loadout: " + point.Dynamic_ZoneLoadout + " - counts: " + point.Dynamic_MinCount + ":" + point.Dynamic_MaxCount);
      }
    } else {
      LoggerDynPrint("Points Disabled");
    }

  }

  //chat message or vanilla notification
  //changed to format. tidy up.
  void Dynamic_message(PlayerBase player, int msg_no, int SpawnCount, string faction, string loadout) {
    if (!player) return;
    string message = string.Format("Player: %1 Number: %2, Faction name: %3, Loadout: %4", player.GetIdentity().GetName(), SpawnCount, faction, loadout);
    if (msg_no == 0) {
      LoggerDynPrint(message);
    } else if (msg_no == 1) {
      WarningMessage(player, string.Format("%1 %2", SpawnCount, m_Dynamic_Groups.MessageText));
      LoggerDynPrint(message);
    } else if (msg_no == 2) {
      WarningMessage(player, m_Dynamic_Groups.MessageText);
      LoggerDynPrint(message);
    } else if (msg_no == 3) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, m_Dynamic_Groups.MessageTitle, string.Format("$1 %2", SpawnCount, m_Dynamic_Groups.MessageText), "set:dayz_gui image:tutorials");
      LoggerDynPrint(message);
    } else if (msg_no == 4) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, m_Dynamic_Groups.MessageTitle, m_Dynamic_Groups.MessageText, "set:dayz_gui image:tutorials");
      LoggerDynPrint(message);
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
  void LoggerDynPrint(string msg) {
    if (GetExpansionSettings().GetLog().AIGeneral)
      GetExpansionSettings().GetLog().PrintLog("[Dynamic AI] " + msg);
  }

  //required
  void Dynamic_PatrolCleanup(eAIDynamicPatrol patrol, eAIGroup group, int count) {
    Dynamic_Spawncount -= count;
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
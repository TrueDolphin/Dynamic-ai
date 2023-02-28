/*
name:TrueDolphin
date:15/2/2023
dynamic ai spawns

this is becoming unwieldly again on server performance per check.
triggers helped a lot, but idk what else to do as far as optimisation goes besides notes.
stripped a lot of useless stuff out when moved settings back to 3_Game.

*/
modded class MissionServer {
  const int SZ_IN_SAFEZONE = 0x0001;
  int m_cur = 0;
  int Dynamic_Spawncount = 0;
  int m_Dynamic_cur = 0;
  ref array < Man > Dynamic_PlayerList = new array < Man > ;
  ref Dynamic_Groups m_Dynamic_Groups;

  #ifdef EXPANSIONMODSPAWNSELECTION
  private ExpansionRespawnHandlerModule m_RespawnModule;
  #endif

  void MissionServer() {

    if (GetDynamicSettings().Init() == true) {
      GetDynamicSettings().PullRef(m_Dynamic_Groups);
      InitDynamicTriggers();
      LoggerDynPrint("Dynamic AI Enabled");
      DynamicTimer();
    }
  }


  //timer call for varied check loops
  void DynamicTimer() {
    Dynamic2Check();
    int m_cor = Math.RandomIntInclusive(m_Dynamic_Groups.Dynamic_MinTimer, m_Dynamic_Groups.Dynamic_MaxTimer);
    LoggerDynPrint("Next valid check in: " + m_cor + "ms");
    GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.DynamicTimer, m_cor, false);
  }

  //step 2 check
  //standard loop through playerlist pulling randomly and removing that from list.
  void Dynamic2Check() {
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
      if (CanSpawn(player)) GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.LocalSpawn, Math.RandomIntInclusive(500, 2000), false, player);
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

    //isnt as expected..
    //LoggerDynPrint("player :" + player.GetIdentity().GetName() + " lifetime :" + player.GetLifeSpanState());

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

  //create and faction stuff
  //it's a mess
  void LocalSpawn(PlayerBase player) {
    if (!player) return;
    m_cur = Math.RandomIntInclusive(0, m_Dynamic_Groups.Group.Count() - 1);
    int SpawnCount;
    vector m_pos;
    eAIGroup AiGroup;
    if (player.CheckZone() == true) {
      SpawnCount = Math.RandomIntInclusive(player.Dynamic_MinCount, player.Dynamic_MaxCount);
    } else {
      SpawnCount = Math.RandomIntInclusive(m_Dynamic_Groups.Group[m_cur].Dynamic_MinCount, m_Dynamic_Groups.Group[m_cur].Dynamic_MaxCount);
    }
    m_pos = ValidPos(player);
    if (SpawnCount > 0) {
      if (!m_pos) return;
      eAIBase sentry;
      sentry = SpawnAI_Dynamic((ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(m_pos, 0, 2))), player);
      Dynamic_Movement(sentry, player);
      AiGroup = sentry.GetGroup();
      if (!AiGroup) AiGroup = eAIGroup.GetGroupByLeader(sentry);
      if (player.CheckZone() == true) {
        sentry.GetGroup().SetFaction(eAIFaction.Create(player.Dynamic_Faction()));
        ExpansionHumanLoadout.Apply(sentry, player.Dynamic_Loadout(), true);
      } else {
        sentry.GetGroup().SetFaction(eAIFaction.Create(m_Dynamic_Groups.Group[m_cur].Dynamic_Faction));
        ExpansionHumanLoadout.Apply(sentry, m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout, true);
      }
      GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.RemoveGroup, m_Dynamic_Groups.CleanupTimer, false, sentry, SpawnCount);
    }
    if (SpawnCount > 1) {
      for (int i = 1; i < SpawnCount; i++) {
        m_Dynamic_cur = i;
        eAIBase sentry2;
        sentry2 = SpawnAI_Dynamic((ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(m_pos, 0, 2))), player);
        sentry2.SetGroup(AiGroup, false);
        if (player.CheckZone() == true) {
          ExpansionHumanLoadout.Apply(sentry2, player.Dynamic_Loadout(), true);
        } else {
          ExpansionHumanLoadout.Apply(sentry2, m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout, true);
        }
      }
    }
    if (SpawnCount != 0) {
      Dynamic_Spawncount += SpawnCount;
      Dynamic_message(player, m_Dynamic_Groups.MessageType, SpawnCount);
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

  //dirty cleanup
  void RemoveGroup(eAIBase target, int count) {
    Dynamic_Spawncount -= count;
    if (target) {
      eAIGroup group = target.GetGroup();
      if (group) group.ClearAI();
    }
  }

  // generic sentry spawn and positional stuff
  eAIBase SpawnAI_Dynamic(vector pos, PlayerBase player) {
    eAIBase ai;
    if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), pos))) return null;
    Dynamic_LootCheck(ai);
    ai.eAI_SetAccuracy(0, 0);
    return ai;
  }

  //Hunt parse
  void Dynamic_Movement(eAIBase ai, PlayerBase player) {
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
      thread TrailingGroup(ai, player, player.GetPosition(), 15000);
    }
    }
  }

  //less code than triggers.
  void TrailingGroup(eAIBase ai, PlayerBase player, vector pos, int timer) {
    Sleep(timer);
    if (!player || !ai) return;
    eAIGroup AiGroup = eAIGroup.Cast(ai.GetGroup());
    if (!AiGroup) AiGroup = eAIGroup.GetGroupByLeader(ai);
    if (pos == player.GetPosition()){
      if (player && ai) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 30, 55));
    }
    if (vector.Distance(player.GetPosition(), ai.GetPosition()) > 100) {
      if (player && ai) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 120));
    }
    if (player) pos = player.GetPosition();
    thread TrailingGroup(ai, player, pos, timer);
  }

  //Lootable parse
  //could probably be neater.
  void Dynamic_LootCheck(eAIBase ai) {
    switch (m_Dynamic_Groups.Lootable) {
    case 0: {
      ai.Expansion_SetCanBeLooted(false);
      break;
    }
    case 1: {
      ai.Expansion_SetCanBeLooted(true);
      break;
    }
    case 2: {
      int i = Math.RandomIntInclusive(0, 1);
      if (i == 1) {
        ai.Expansion_SetCanBeLooted(true);
      } else {
        ai.Expansion_SetCanBeLooted(false);
      }
      break;
    }
    case 3: {
      if (m_Dynamic_cur == 1) {
        ai.Expansion_SetCanBeLooted(true);
      } else {
        ai.Expansion_SetCanBeLooted(false);
      }
      break;
    }
    }
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
  //change to switch reverse order string building
  void Dynamic_message(PlayerBase player, int msg_no, int SpawnCount) {
    if (!player) return;
    if (msg_no == 0) {
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 1) {
      WarningMessage(player, SpawnCount.ToString() + " " + m_Dynamic_Groups.MessageText);
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 2) {
      WarningMessage(player, m_Dynamic_Groups.MessageText);
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 3) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, m_Dynamic_Groups.MessageTitle, SpawnCount.ToString() + " " + m_Dynamic_Groups.MessageText, "set:dayz_gui image:tutorials");
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 4) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, m_Dynamic_Groups.MessageTitle, m_Dynamic_Groups.MessageText, "set:dayz_gui image:tutorials");
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
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

};
//name:TrueDolphin
//date:2/2/2023
//dynamic ai spawns

modded class MissionServer {
  private const static string EXP_DYNAMIC_FOLDER = "$profile:ExpansionMod\\AI\\Dynamic\\";
  private const static string EXP_AI_DYNAMIC_SETTINGS = EXP_DYNAMIC_FOLDER + "DynamicSettings.json";
  const int SZ_IN_SAFEZONE = 0x0001;
  int m_cur = 0;
  int Dynamic_Timer;
  int Dynamic_MinCount;
  int Dynamic_MaxCount;
  int Dynamic_MinDistance;
  int Dynamic_MaxDistance;
  int Dynamic_Total = -1;
  int EngageTimer;
  int CleanupTimer;
  int MessageType;
  int PointSafe;
  bool Dynamic_Version = true;
  bool Dynamic_InVehicle;
  bool Dynamic_IsBleeding;
  bool Dynamic_IsRestrained;
  bool Dynamic_IsUnconscious;
  bool Dynamic_IsInSafeZone;
  bool Dynamic_TPSafeZone;
  bool Dynamic_InOwnTerritory;
  string MessageText;
  string MessageTitle;
  string Dynamic_Loadout;
  string Dynamic_Faction;

  ref Dynamic_Groups m_Dynamic_Groups;

  #ifdef EXPANSIONMODSPAWNSELECTION
  private ExpansionRespawnHandlerModule m_RespawnModule;
  #endif

  void MissionServer() {
    InitDynamicSettings();
    initDynamicTriggers();
    if (Dynamic_Total < 0) {
      Print("Dynamic AI Config Error - Disabled");
      LoggerDynPrint("No Dynamic Groups valid."); //Sadface box
    } else {
      if (Dynamic_Version == true) {
        Print("Dynamic AI Enabled");
        DynamicTimer();
      }
    }
  }

  //timer call for varied check loops
  void DynamicTimer() {
    GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DynamicTimer);
    DynamicPlayerCheck();
    int m_cor;
    m_cor = Math.RandomIntInclusive(m_Dynamic_Groups.Dynamic_MinTimer, m_Dynamic_Groups.Dynamic_MaxTimer);
    LoggerDynPrint("Next valid check in: " + m_cor + "ms");
    GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.DynamicTimer, m_cor, false); // horaaah.
  }

  //loop for playerlist
  void DynamicPlayerCheck() {
    for (int i = 0; i < m_Players.Count(); i++) {
      PlayerBase player = PlayerBase.Cast(m_Players[i]);
      if (!player || !player.IsAlive() || !player.GetIdentity()) continue;
      
      #ifdef EXPANSIONMODSPAWNSELECTION
      if (InRespawnMenu(player.GetIdentity())) continue;
      #endif
      //shitty load balancing
      if (ValidPlayer(player)) GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.LocalSpawn, Math.RandomIntInclusive(500, 2000), false, player);
    }
  }
  //checks and overrides for player
  bool ValidPlayer(PlayerBase player) {
    PlayerIdentity identity = player.GetIdentity();
    bool valid = true;

    #ifdef EXPANSIONMODBASEBUILDING
    if (player.IsInsideOwnTerritory() != Dynamic_InOwnTerritory) {
      return false;
    }
    #endif

    if (player.IsInVehicle() != Dynamic_InVehicle || player.Expansion_IsInSafeZone() != Dynamic_IsInSafeZone || player.IsBleeding() != Dynamic_IsBleeding || player.IsRestrained() != Dynamic_IsRestrained || player.IsUnconscious() != Dynamic_IsUnconscious) {
      return false;
    }

    #ifdef SZDEBUG
    if (player.GetSafeZoneStatus() == SZ_IN_SAFEZONE && Dynamic_TPSafeZone == false) {
      return false;
    }
    #endif

    if (m_Dynamic_Groups.Points_Enabled == 1 && player.CheckSafe()) {
      //LoggerDynPrint("debugger - player safe");
      return false;
    }

    return valid;
  }

  //create and faction stuff
  void LocalSpawn(PlayerBase player) {
    m_cur = Math.RandomIntInclusive(0, Dynamic_Total);
    int SpawnCount;
    if (player.CheckZone() == true){
      SpawnCount = Math.RandomIntInclusive(player.Dynamic_MinCount, player.Dynamic_MaxCount);
    } else {
      SpawnCount = Math.RandomIntInclusive(m_Dynamic_Groups.Group[m_cur].Dynamic_MinCount, m_Dynamic_Groups.Group[m_cur].Dynamic_MaxCount);
    }
    vector m_pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), Dynamic_MinDistance, Dynamic_MaxDistance)));
    for (int i = 0; i < SpawnCount; i++) {
      eAIBase sentry;
      if (!player) return;
      sentry = SpawnAI_Dynamic((ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(m_pos, 0, 2))), player);
        if (player.CheckZone() == true) {
          sentry.GetGroup().SetFaction(eAIFaction.Create(player.Dynamic_Faction()));
        } else {
          sentry.GetGroup().SetFaction(eAIFaction.Create(m_Dynamic_Groups.Group[m_cur].Dynamic_Faction));
        }
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.RemoveGroup, CleanupTimer, false, sentry);
    }
    if (SpawnCount != 0) {
      Dynamic_message(player, MessageType, SpawnCount);
    }
  }

  //chat message or vanilla notification
  void Dynamic_message(PlayerBase player, int msg_no, int SpawnCount) {
    if (msg_no == 0) {
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 1) {
      WarningMessage(player, SpawnCount.ToString() + " " + MessageText);
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 2) {
      WarningMessage(player, MessageText);
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 3) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, MessageTitle, SpawnCount.ToString() + " " + MessageText, "set:dayz_gui image:tutorials");
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
    if (msg_no == 4) {
      NotificationSystem.SendNotificationToPlayerExtended(player, 5, MessageTitle, MessageText, "set:dayz_gui image:tutorials");
      LoggerDynPrint("Player: " + player.GetIdentity().GetName() + " Number: " + SpawnCount.ToString() + ", Faction name: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Faction + ", Loadout: " + m_Dynamic_Groups.Group[m_cur].Dynamic_Loadout);
    }
  }

  //dirty cleanup
  void RemoveGroup(eAIBase target) {
    if (target) {
      eAIGroup group = target.GetGroup();
      if (group) group.ClearAI();
    }
  }

  // generic sentry spawn, loadout set and positional stuff
  eAIBase SpawnAI_Dynamic(vector pos, PlayerBase player) {
    eAIBase ai;
    if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), pos))) return null;
    if (player.CheckZone() == true) {
      ExpansionHumanLoadout.Apply(ai, player.Dynamic_Loadout(), true);
      ai.Expansion_SetCanBeLooted(m_Dynamic_Groups.Lootable);
      ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 0, 3));
      if (m_Dynamic_Groups.HuntMode == 1) player.GetTargetInformation().AddAI(ai, EngageTimer); 
      return ai;
    }
    ai.Expansion_SetCanBeLooted(m_Dynamic_Groups.Lootable);
    ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 0, 3));
    if (m_Dynamic_Groups.HuntMode == 1) player.GetTargetInformation().AddAI(ai, EngageTimer);
    return ai;
  }
  // Ingame chat message
  void WarningMessage(PlayerBase player, string message) {
    if ((player) && (message != "")) {
      Param1 < string > Msgparam;
      Msgparam = new Param1 < string > (message);
      GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
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

  //file stuff
  void InitDynamicSettings() {
    //LoggerDynPrint("Dynamic Config Loader Start");
    if (!FileExist(EXP_AI_DYNAMIC_SETTINGS)) {
      if (!FileExist(EXP_DYNAMIC_FOLDER))
        MakeDirectory(EXP_DYNAMIC_FOLDER);

      LoggerDynPrint("WARNING: Couldn't find config file !");
      LoggerDynPrint("Dynamic config will be located in: " + EXP_DYNAMIC_FOLDER);
      DefaultDynamicSettings(m_Dynamic_Groups);
      JsonFileLoader < Dynamic_Groups > .JsonSaveFile(EXP_AI_DYNAMIC_SETTINGS, m_Dynamic_Groups);
    } else {
      m_Dynamic_Groups = new Dynamic_Groups();
      JsonFileLoader < Dynamic_Groups > .JsonLoadFile(EXP_AI_DYNAMIC_SETTINGS, m_Dynamic_Groups);
      LoggerDynPrint("Loading config (" + EXP_AI_DYNAMIC_SETTINGS + ")");
    }

    if (m_Dynamic_Groups.Version != 10) { // dont like this. change it.
      LoggerDynPrint("Settings File Out of date. Please delete and restart server.");
      Dynamic_Version = false;
      return;
    }

    if (m_Dynamic_Groups.Dynamic_MinTimer < 300000) { // global minimum time of 5 mins 
      LoggerDynPrint("Timer Set too low. Defaulting to 5 minutes");
      m_Dynamic_Groups.Dynamic_MinTimer = 300000;
    }

    if (m_Dynamic_Groups.Dynamic_MaxTimer < m_Dynamic_Groups.Dynamic_MinTimer) { 
      LoggerDynPrint("Max Timer set lower than min timer, setting to the same.");
      m_Dynamic_Groups.Dynamic_MaxTimer = m_Dynamic_Groups.Dynamic_MinTimer;
    }

    if (m_Dynamic_Groups.MinDistance < 120) {
      LoggerDynPrint("Minimum Distance too low. setting to 120m");
      m_Dynamic_Groups.MinDistance = 120;
    }

    Dynamic_MinDistance = m_Dynamic_Groups.MinDistance;

    if (m_Dynamic_Groups.MaxDistance < m_Dynamic_Groups.MinDistance) {
      LoggerDynPrint("Max distance under min distance. setting +20m");
      m_Dynamic_Groups.MaxDistance = m_Dynamic_Groups.MinDistance + 20;
    }

    Dynamic_MaxDistance = m_Dynamic_Groups.MaxDistance;

    if (m_Dynamic_Groups.HuntMode != 0 && m_Dynamic_Groups.HuntMode != 1) {
      LoggerDynPrint("HuntMode setting wrong. setting to default.");
      m_Dynamic_Groups.HuntMode = 1;
    }

    if (m_Dynamic_Groups.Points_Enabled == 0) {
      LoggerDynPrint("points disabled");
    } else if (m_Dynamic_Groups.Points_Enabled == 1) {
      LoggerDynPrint("points enabled");
      foreach(Dynamic_Point point: m_Dynamic_Groups.Point) {

        if (point.Dynamic_Radius < 0) {
          LoggerDynPrint("Radius on group incorrect, setting to 100m");
          point.Dynamic_Radius = 100;
        }
        if (!FileExist("$profile:ExpansionMod\\Loadouts\\" + point.Dynamic_ZoneLoadout)) {
          LoggerDynPrint("Loadout Not Found: " + point.Dynamic_ZoneLoadout);
          point.Dynamic_ZoneLoadout = "HumanLoadout.json";
        }

        if (point.Dynamic_Safe != 1 && point.Dynamic_Safe != 0) {
          LoggerDynPrint("Zone safe value incorrect. setting to safe");
          point.Dynamic_Safe = 1;
        }

        eAIFaction faction = eAIFaction.Create(point.Dynamic_Faction);
			  if (!faction)
			  {
        LoggerDynPrint("Faction not correct: " + point.Dynamic_Faction);
				point.Dynamic_Faction = "Raiders";
			  }
      }

    } else {
      LoggerDynPrint("error, disabling points");
      m_Dynamic_Groups.Points_Enabled = 0;
    }

    if (m_Dynamic_Groups.EngageTimer < 300000) { // global minimum time of 5 mins
      LoggerDynPrint("Minimum engagement too low. setting to 5m");
      m_Dynamic_Groups.EngageTimer = 300000;
    }

    EngageTimer = m_Dynamic_Groups.EngageTimer;

    if (m_Dynamic_Groups.CleanupTimer < m_Dynamic_Groups.EngageTimer) {
      LoggerDynPrint("Cleanup timer under engage timer. setting +1m");
      m_Dynamic_Groups.CleanupTimer = m_Dynamic_Groups.EngageTimer + 60000;
    }

    CleanupTimer = m_Dynamic_Groups.CleanupTimer;

    if (m_Dynamic_Groups.MessageType != 0 && m_Dynamic_Groups.MessageType != 1 && m_Dynamic_Groups.MessageType != 2 && m_Dynamic_Groups.MessageType != 3 && m_Dynamic_Groups.MessageType != 4) {
      LoggerDynPrint("Message type error. disabling.");
      m_Dynamic_Groups.MessageType = 0;
    }

    MessageType = m_Dynamic_Groups.MessageType;

    if (!m_Dynamic_Groups.MessageTitle) {
      LoggerDynPrint("Notification title error. setting default.");
      m_Dynamic_Groups.MessageTitle = "Dynamic AI";
    } else if (m_Dynamic_Groups.MessageTitle == " ") {
      LoggerDynPrint("Notification title error. setting default.");
      m_Dynamic_Groups.MessageTitle = "Dynamic AI";
    }

    MessageTitle = m_Dynamic_Groups.MessageTitle;

    if (!m_Dynamic_Groups.MessageText) {
      LoggerDynPrint("Message text error. setting default.");
      m_Dynamic_Groups.MessageText = "AI Spotted in the Area. Be Careful.";
    } else if (m_Dynamic_Groups.MessageText == " ") {
      LoggerDynPrint("Message text error. setting default.");
      m_Dynamic_Groups.MessageText = "AI Spotted in the Area. Be Careful.";
    }
    MessageText = m_Dynamic_Groups.MessageText;

    if (m_Dynamic_Groups.Lootable != true || m_Dynamic_Groups.Lootable != false){
      m_Dynamic_Groups.Lootable = false;
      LoggerDynPrint("lootable check error. setting default to false.");
    }

    if (m_Dynamic_Groups.Dynamic_InVehicle != true && m_Dynamic_Groups.Dynamic_InVehicle != false) {
      LoggerDynPrint("ignore vehicle check error - default off");
      m_Dynamic_Groups.Dynamic_InVehicle = false;
    }
    Dynamic_InVehicle = m_Dynamic_Groups.Dynamic_InVehicle;

    if (m_Dynamic_Groups.Dynamic_IsBleeding != true && m_Dynamic_Groups.Dynamic_IsBleeding != false) {
      LoggerDynPrint("ignore bleeding check error - default off");
      m_Dynamic_Groups.Dynamic_IsBleeding = false;
    }
    Dynamic_IsBleeding = m_Dynamic_Groups.Dynamic_IsBleeding;

    if (m_Dynamic_Groups.Dynamic_IsRestrained != true && m_Dynamic_Groups.Dynamic_IsRestrained != false) {
      LoggerDynPrint("ignore restrained check error - default off");
      m_Dynamic_Groups.Dynamic_IsRestrained = false;
    }
    Dynamic_IsRestrained = m_Dynamic_Groups.Dynamic_IsRestrained;

    if (m_Dynamic_Groups.Dynamic_IsUnconscious != true && m_Dynamic_Groups.Dynamic_IsUnconscious != false) {
      LoggerDynPrint("ignore Unconscious check error - default off");
      m_Dynamic_Groups.Dynamic_IsUnconscious = false;
    }
    Dynamic_IsUnconscious = m_Dynamic_Groups.Dynamic_IsUnconscious;

    if (m_Dynamic_Groups.Dynamic_IsInSafeZone != true && m_Dynamic_Groups.Dynamic_IsInSafeZone != false) {
      LoggerDynPrint("ignore expansion SafeZone check error - default off");
      m_Dynamic_Groups.Dynamic_IsInSafeZone = false;
    }
    Dynamic_IsInSafeZone = m_Dynamic_Groups.Dynamic_IsInSafeZone;

    if (m_Dynamic_Groups.Dynamic_TPSafeZone != true && m_Dynamic_Groups.Dynamic_TPSafeZone != false) {
      LoggerDynPrint("ignore traderplus SafeZone check error - default off");
      m_Dynamic_Groups.Dynamic_TPSafeZone = false;
    }
    Dynamic_TPSafeZone = m_Dynamic_Groups.Dynamic_TPSafeZone;

    if (m_Dynamic_Groups.Dynamic_InOwnTerritory != true && m_Dynamic_Groups.Dynamic_InOwnTerritory != false) {
      LoggerDynPrint("ignore expansion own territory check error - default off");
      m_Dynamic_Groups.Dynamic_InOwnTerritory = false;
    }
    Dynamic_InOwnTerritory = m_Dynamic_Groups.Dynamic_InOwnTerritory;

    foreach(Dynamic_Group group: m_Dynamic_Groups.Group) {

      if (group.Dynamic_MinCount > group.Dynamic_MaxCount) {
        LoggerDynPrint("Minimum cant be more than maximum: " + group.Dynamic_MinCount);
        continue;
      }
      if (group.Dynamic_MaxCount < 1) {
        LoggerDynPrint("Maximum count cant be less than 0: " + group.Dynamic_MaxCount);
        continue;
      }
      if (!FileExist("$profile:ExpansionMod\\Loadouts\\" + group.Dynamic_Loadout)) {
        LoggerDynPrint("Loadout Not Found: " + group.Dynamic_Loadout);
        continue;
      }
        eAIFaction faction2 = eAIFaction.Create(group.Dynamic_Faction);
			  if (!faction2)
			  {
        LoggerDynPrint("Faction not correct: " + group.Dynamic_Faction);
				group.Dynamic_Faction = "Raiders";
			  }
      Dynamic_Total += 1;
    }
    //LoggerDynPrint("Dynamic Config Loader End");
  }

  void initDynamicTriggers(){
    if (m_Dynamic_Groups.Points_Enabled == 1) {
      foreach(Dynamic_Point point: m_Dynamic_Groups.Point) {
        Dynamic_Trigger dynamic_trigger = Dynamic_Trigger.Cast(GetGame().CreateObjectEx("Dynamic_Trigger", point.Dynamic_Position, ECE_NONE));
        dynamic_trigger.SetCollisionCylinder(point.Dynamic_Radius, point.Dynamic_Radius / 2);
        dynamic_trigger.Dynamic_SetData(point.Dynamic_Safe, point.Dynamic_Faction, point.Dynamic_ZoneLoadout, point.Dynamic_MinCount, point.Dynamic_MaxCount);
        LoggerDynPrint("Trigger at location: " + point.Dynamic_Position + " - Radius: " + point.Dynamic_Radius);
        LoggerDynPrint("Safe: " + point.Dynamic_Safe + " - Faction: " + point.Dynamic_Faction + " - Loadout: " + point.Dynamic_ZoneLoadout + " - counts: " + point.Dynamic_MinCount + ":" + point.Dynamic_MaxCount);
      }
    }
  }
  
  //expansion logging
  void LoggerDynPrint(string msg) {
    if (GetExpansionSettings().GetLog().AIGeneral)
      GetExpansionSettings().GetLog().PrintLog("[Dynamic AI] " + msg);
  }

  //generate default array data
  void DefaultDynamicSettings(out Dynamic_Groups Data) {
    Data = new Dynamic_Groups();
    Data.Group.Insert(new Dynamic_Group(0, 3, "WestLoadout.json", "Shamans"));
    Data.Group.Insert(new Dynamic_Group(0, 3, "HumanLoadout.json", "Shamans"));
    Data.Group.Insert(new Dynamic_Group(0, 3, "EastLoadout.json", "Shamans"));

    Data.Point.Insert(new Dynamic_Point(1, 50, "HumanLoadout.json", 0, 4, "Shamans", "0.0 0.0 0.0"));
    Data.Point.Insert(new Dynamic_Point(0, 100, "HumanLoadout.json", 0, 5, "West", "0.0 0.0 0.0"));
    Data.Point.Insert(new Dynamic_Point(0, 150, "HumanLoadout.json", 3, 4, "East", "0.0 0.0 0.0"));
  }
}

class Dynamic_Groups {
  int Version = 10;
  int Dynamic_MinTimer = 1200000;
  int Dynamic_MaxTimer = 1200000;
  int MinDistance = 140;
  int MaxDistance = 220;
  int HuntMode = 1;
  int Points_Enabled = 0;
  int EngageTimer = 300000;
  int CleanupTimer = 360000;
  int MessageType = 1;
  string MessageTitle = "Dynamic AI";
  string MessageText = "AI Spotted in the Area. Be Careful.";
  bool Lootable = false;
  bool Dynamic_InVehicle = false;
  bool Dynamic_IsBleeding = false;
  bool Dynamic_IsRestrained = false;
  bool Dynamic_IsUnconscious = false;
  bool Dynamic_IsInSafeZone = false;
  bool Dynamic_TPSafeZone = false;
  bool Dynamic_InOwnTerritory = false;
  ref array < ref Dynamic_Group > Group;
  ref array < ref Dynamic_Point > Point;
  void Dynamic_Groups() {
    Group = new array < ref Dynamic_Group > ;
    Point = new array < ref Dynamic_Point > ;
  }
}

class Dynamic_Group {
  int Dynamic_MinCount;
  int Dynamic_MaxCount;
  string Dynamic_Loadout;
  string Dynamic_Faction;
  void Dynamic_Group(int a, int b, string c, string d) {
    Dynamic_MinCount = a;
    Dynamic_MaxCount = b;
    Dynamic_Loadout = c;
    Dynamic_Faction = d;
  }
}

class Dynamic_Point {
  float Dynamic_Radius;
  string Dynamic_ZoneLoadout;
  string Dynamic_Faction;
  int Dynamic_MinCount;
  int Dynamic_MaxCount;
  bool Dynamic_Safe;
  vector Dynamic_Position;
  void Dynamic_Point(bool a, float b, string c, int d, int e, string f, vector g) {
    Dynamic_Safe = a;
    Dynamic_Radius = b;
    Dynamic_ZoneLoadout = c;
    Dynamic_MinCount = d;
    Dynamic_MaxCount = e;
    Dynamic_Faction = f;
    Dynamic_Position = g;
  }
};
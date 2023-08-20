  //change this whole system to avoidplayer

  
class Location_Trigger: CylinderTrigger
{       

  int i_PlayerCount;
  bool Spatial_TimerCheck;
  vector Spatial_SpawnPosition;

  eAISpatialPatrol dynPatrol;
  Spatial_Location location;
  Spatial_Notification notification;
  Notification_Trigger notif_trigger;

  ref Spatial_Groups m_Spatial_Groups;
  array<ref TriggerInsider> notif;

  void Location_Trigger(){
    GetSpatialSettings().PullRef(m_Spatial_Groups);   
    }

  void Spatial_SetData(Spatial_Location Location, Notification_Trigger b){
    location = Location;
    notif_trigger = b;
  } //changed to class instead of individuals
  void SetNotification(Spatial_Notification a){
    notification = a;
    }
  void SpawnCheck(){
    if (dynPatrol || Spatial_TimerCheck || m_insiders.Count() == 0) return;

    int m_Groupid = Math.RandomIntInclusive(0, int.MAX);
    SpatialDebugPrint("LocationID: " + m_Groupid);
    float random = Math.RandomFloat(0.0, 1.0);
    SpatialDebugPrint("Location Chance: " + location.Spatial_Chance + " | random: " + random);
    if (location.Spatial_Chance < random) return;

    int SpawnCount = Math.RandomIntInclusive(location.Spatial_MinCount, location.Spatial_MaxCount);
    if (SpawnCount > 0) {
      Spatial_Spawn(SpawnCount, location);
      Spatial_TimerCheck = true;
      GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Spatial_timer, location.Spatial_Timer, false);
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
    notif = notif_trigger.GetInsiders();
    foreach(TriggerInsider insider : notif) {
      PlayerBase player = PlayerBase.Cast(insider.GetObject());
      if (player) Spatial_message(player, count);
    }
   }

  void Spatial_message(PlayerBase player, int SpawnCount) {
      if (!player) return;
      string title, text, faction, loadout;
      int msg_no = notification.MessageType;
      title = notification.MessageTitle;
      text = notification.MessageText.GetRandomElement();
      faction = location.Spatial_Faction;
      loadout = location.Spatial_ZoneLoadout;
      
      string message = string.Format("Player: %1 Number: %2, Faction name: %3, Loadout: %4", player.GetIdentity().GetName(), SpawnCount, faction, loadout);
      if (msg_no == 0) {
        SpatialLoggerPrint(message);
      } else if (msg_no == 1) {
        Spatial_WarningMessage(player, string.Format("%1 %2", SpawnCount, text));
        SpatialLoggerPrint(message);
      } else if (msg_no == 2) {
        Spatial_WarningMessage(player, text);
        SpatialLoggerPrint(message);
      } else if (msg_no == 3) {
        NotificationSystem.SendNotificationToPlayerExtended(player, 5, title, string.Format("%1 %2", SpawnCount, text), "set:dayz_gui image:tutorials");
        SpatialLoggerPrint(message);
      } else if (msg_no == 4) {
        NotificationSystem.SendNotificationToPlayerExtended(player, 5, title, text, "set:dayz_gui image:tutorials");
        SpatialLoggerPrint(message);
      } else if (msg_no == 5 && player.Spatial_HasGPSReceiver()) {
        NotificationSystem.SendNotificationToPlayerExtended(player, 5, title, text, "set:dayz_gui image:tutorials");
        SpatialLoggerPrint(message);
      }
  } //chat message or vanilla notification
  vector ValidPos(){
    if (m_Spatial_Groups.Locations_Enabled == 2) return location.Spatial_SpawnPosition;
    return ExpansionStatic.GetSurfacePosition(location.Spatial_SpawnPosition);
    }

  bool ValidSpawn(){
			if (!GetCEApi().AvoidPlayer(location.Spatial_SpawnPosition, 5))
        return true;
		  return false;
    }
  void Spatial_WarningMessage(PlayerBase player, string message) {
    if ((player) && (message != "")) {
      Param1 < string > Msgparam;
      Msgparam = new Param1 < string > (message);
      GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
    }
  } //Ingame chat message
  void SpatialDebugPrint(string msg) {
    if (GetSpatialSettings().Spatial_Debug())
      GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
  } //expansion debug print
  void SpatialLoggerPrint(string msg) {
    GetExpansionSettings().GetLog().PrintLog("[Spatial AI] " + msg);
  } //expansion logging
}

  class SpatialAI {

    const int SZ_IN_SAFEZONE = 0x0001;
    int m_cur = 0;
    int m_Spatial_cur = 0;
    ref array < Man > Spatial_PlayerList;
    ref Spatial_Groups m_Spatial_Groups;
    ref Spatial_Players m_Spatial_Players;
    ref Spatial_Notifications m_Spatial_Notifications;

    #ifdef EXPANSIONMODSPAWNSELECTION
      private ExpansionRespawnHandlerModule m_RespawnModule;
      
      bool InRespawnMenu(PlayerIdentity identity) {
        CF_Modules < ExpansionRespawnHandlerModule > .Get(m_RespawnModule);
        if (m_RespawnModule && m_RespawnModule.IsPlayerInSpawnSelect(identity)) {
          return true;
        }
        return false;
      } //LM's code altered to a bool in class
    #endif

    void SpatialAI(){
      SpatialLoggerPrint("Spatial AI Date: 15/8/2023 R3");
      GetSpatialSettings().PullRef(m_Spatial_Groups);
      SpatialPlayerSettings().PullRef(m_Spatial_Players);
      Spatial_NotificationSettings().PullRef(m_Spatial_Notifications);
    } //constructor

    void Spatial_Check(array<Man> m_Players) {
      SpatialDebugPrint("Spatial::Check - Start");
        // If there are no players in the list, return early.
        if (m_Players.Count() == 0){
        SpatialDebugPrint("Spatial::Check - no players");
        return;
        }

        int playerChecks = m_Spatial_Groups.PlayerChecks;
        bool checkOnlyLeader = playerChecks >= 0;

        // Make sure playerChecks is positive, even if it was initially negative.
        playerChecks = Math.AbsInt(playerChecks);
        int playercount = m_Players.Count() + 1;
        int minAge = m_Spatial_Groups.MinimumAge;
        bool checkMinAge = minAge > 0;

        for (int i = 0; i < playercount; ++i) 
        {
          SpatialDebugPrint("Spatial::Check - loop :" + i + " out of " + playercount);
          PlayerBase player = PlayerBase.Cast(m_Players.GetRandomElement());
          if (!player || !player.IsAlive() || !player.GetIdentity()) {
            SpatialDebugPrint("Spatial::Check - invalid player");
            continue;
          }

          m_Players.RemoveItem(player);

          if (checkOnlyLeader)
          {
              eAIGroup PlayerGroup = eAIGroup.Cast(player.GetGroup());
              // If the player is not in an ai group or isnt leader, skip to the next iteration.
              if (!PlayerGroup || player != player.GetGroup().GetLeader()){
                  SpatialDebugPrint("Spatial::Check - player not leader of group");
                  continue;
              } 

              #ifdef EXPANSIONMODGROUPS
                ExpansionPartyData party = ExpansionPartyData.Cast(player.Expansion_GetParty());
                // If the player is not in a party or isnt leader, skip to the next iteration.
                if (!party || player.GetIdentity().GetId() != party.GetOwnerUID()){
                    SpatialDebugPrint("Spatial::Check - player not leader of party");
                    continue;
                }
              #endif
          }

          #ifdef EXPANSIONMODSPAWNSELECTION
              // If the player is in the respawn menu, skip to the next iteration.
              if (InRespawnMenu(player.GetIdentity())){
                  SpatialDebugPrint("Spatial::Check - player in respawn");
                  continue;
              }
          #endif

          if (checkMinAge) {
            if (!player.Spatial_CheckAge(minAge)){
            SpatialDebugPrint("Spatial::Check - user not old enough");
            continue;
            }
          } 
          if (Spatial_CanSpawn(player))
            Spatial_LocalSpawn(player);

          if (playerChecks != 0)
            if (i == playerChecks) return;
        }
      SpatialDebugPrint("Spatial::Check - End");
    } //Standard loop through the player list, selecting random players and removing them from the list. #refactored by LieutenantMaster

    bool Spatial_CanSpawn(PlayerBase player) {
      SpatialDebugPrint("Spatial::CanSpawn - Start");
      if (player.IsBleeding() && !GetSpatialSettings().Spatial_IsBleeding())
        return false;
      if (player.IsInVehicle() && !GetSpatialSettings().Spatial_InVehicle())
        return false;
      if (player.IsUnconscious() && !GetSpatialSettings().Spatial_IsUnconscious())
        return false;
      if (player.IsRestrained() && !GetSpatialSettings().Spatial_IsRestrained())
        return false;
      #ifdef EXPANSIONMODBASEBUILDING
        // Check if player is inside their own territory and its override
        if (player.IsInsideOwnTerritory() && !GetSpatialSettings().Spatial_InOwnTerritory())
          return false;
      #endif
      if (player.Expansion_IsInSafeZone() && !GetSpatialSettings().Spatial_IsInSafeZone())
        return false;

      #ifdef SZDEBUG
        // Check for specific safe zone debug status and its override
        if (player.GetSafeZoneStatus() == SZ_IN_SAFEZONE && !GetSpatialSettings().Spatial_TPSafeZone())
          return false;
      #endif
      if (m_Spatial_Groups.Points_Enabled == 2 && !player.Spatial_CheckZone())
        return false;
      if (m_Spatial_Groups.Points_Enabled != 0 && player.Spatial_CheckSafe())
        return false;
      SpatialDebugPrint("Spatial::CanSpawn - End");
      return true;
    } //checks and overrides

    void Spatial_LocalSpawn(PlayerBase player) {
      SpatialDebugPrint("Spatial::LocalSpawn - Start");
      if (!player) return;

      int m_Groupid = Math.RandomIntInclusive(0, 5000);
      SpatialDebugPrint("GroupID: " + m_Groupid);

      m_cur = Math.RandomIntInclusive(0, m_Spatial_Groups.Group.Count() - 1);
      int SpawnCount;
      Spatial_Group group;

      if (player.Spatial_CheckZone()) group = player.GetSpatialGroup();
      else group = Spatial_GetWeightedGroup(m_Spatial_Groups.Group);

      if (!group){
        SpatialDebugPrint("Spatial::LocalSpawn - Error - No Group.");
        return;
      }

      SpawnCount = Math.RandomIntInclusive(group.Spatial_MinCount, group.Spatial_MaxCount);
      float random = Math.RandomFloat(0.0, 1.0);
      SpatialDebugPrint("Chance: " + group.Spatial_Chance + " | random: " + random);

      if (group.Spatial_Chance < random) return;

      if (SpawnCount > 0) {
        if (m_Spatial_Groups.GroupDifficulty == 1) {
          int groupcount = 0;
          int partycount = 0;
          int totalcount;
          #ifdef EXPANSIONMODGROUPS
            ExpansionPartyData party = ExpansionPartyData.Cast(player.Expansion_GetParty());
            array<ref ExpansionPartyPlayerData> PartyMembers = party.GetPlayers();
            partycount = PartyMembers.Count();
          #endif
          totalcount = Math.Max(groupcount, partycount);
          eAIGroup PlayerGroup = eAIGroup.Cast(player.GetGroup());
          if (!PlayerGroup) PlayerGroup = eAIGroup.GetGroupByLeader(player);
          if (totalcount > 1) SpawnCount += (totalcount - 1);
        }
      Spatial_Spawn(player, SpawnCount, group);
      } else SpatialDebugPrint("group/point ai count too low this check");
      
      SpatialDebugPrint("End GroupID: " + m_Groupid);
      SpatialDebugPrint("Spatial::LocalSpawn - End");
    } //create and stuff.

    Spatial_Group Spatial_GetWeightedGroup(array < ref Spatial_Group > groups, array < float > weights = NULL) {
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

    bool Spatial_ValidPos(PlayerBase player, out vector pos) {
      pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
      for (int i = 0; i < 50; i++) {
        if (GetGame().SurfaceIsSea(pos[0], pos[2]) || GetGame().SurfaceIsPond(pos[0], pos[2])) {
          pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
        } else i = 50;
      }
      if (GetGame().SurfaceIsSea(pos[0], pos[2]) || GetGame().SurfaceIsPond(pos[0], pos[2])) return false;
      return true;
    } //fixed

    void Spatial_Spawn(PlayerBase player, int bod, Spatial_Group Group) {
      SpatialDebugPrint("Spatial::Spawn - Start");
      vector startpos, waypointpos;
      if (!Spatial_ValidPos(player, startpos)){
        SpatialLoggerPrint("Valid spawnpos not found. not spawning.");
        return;
      }
      if (!Spatial_ValidPos(player, waypointpos)){
        SpatialLoggerPrint("Valid waypointpos not found. using startpos.");
        waypointpos = startpos;
      }
      TVectorArray waypoints = { waypointpos };
      string Formation = "RANDOM";
      eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
      if (player.Spatial_CheckZone()) {
        if (player.Spatial_HuntMode() == 3)
          behaviour = typename.StringToEnum(eAIWaypointBehavior, "HALT");
      }
      int mindistradius, maxdistradius, despawnradius;
      mindistradius = 0;
      maxdistradius = 1200;
      despawnradius = 1200;
      auto dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, Group.Spatial_Loadout, bod, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(Group.Spatial_Faction), eAIFormation.Create(Formation), player, mindistradius, maxdistradius, despawnradius, 2.0, 3.0, Group.Spatial_Lootable, Group.Spatial_UnlimitedReload);
      if (dynPatrol) {
        dynPatrol.SetGroupName(Group.Spatial_Name);
        dynPatrol.SetAccuracy(Group.Spatial_MinAccuracy, Group.Spatial_MaxAccuracy);
        dynPatrol.SetSniperProneDistanceThreshold(0.0);
        dynPatrol.SetHunted(player);
      }

      if (player.Spatial_CheckZone())
        Spatial_message(player, bod, Group, player.Spatial_notification());
      else {
        Spatial_Notification notification = new Spatial_Notification( "Default", m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText});
        Spatial_message(player, bod, Group, notification);
      }
      SpatialDebugPrint("Spatial::Spawn - End");
    } //Spatial_Spawn(player, SpawnCount, group)

    void InitSpatialTriggers() {
          SpatialDebugPrint("Spatial::Triggers - Start");
      if (m_Spatial_Groups.Points_Enabled == 1 || m_Spatial_Groups.Points_Enabled == 2) {
        SpatialLoggerPrint("points Enabled");
        foreach(Spatial_Point points: m_Spatial_Groups.Point) {
          Spatial_Trigger spatial_trigger = Spatial_Trigger.Cast(GetGame().CreateObjectEx("Spatial_Trigger", points.Spatial_Position, ECE_NONE));
          spatial_trigger.SetCollisionCylinder(points.Spatial_Radius, points.Spatial_Radius / 2);
          spatial_trigger.SetSpatialPoint(points);
          SetNotificationPoint(spatial_trigger, points);
          SpatialLoggerPrint("Trigger at point: " + points.Spatial_Position + " - Radius: " + points.Spatial_Radius);
          SpatialLoggerPrint("Safe: " + points.Spatial_Safe + " - Faction: " + points.Spatial_Faction + " - Loadout: " + points.Spatial_ZoneLoadout + " - counts: " + points.Spatial_MinCount + ":" + points.Spatial_MaxCount);
        }
      } else SpatialLoggerPrint("points Disabled");
      
      if (m_Spatial_Groups.Locations_Enabled != 0) {
        SpatialLoggerPrint("Locations Enabled");
        foreach(Spatial_Location location: m_Spatial_Groups.Location) {
          Location_Trigger location_trigger = Location_Trigger.Cast(GetGame().CreateObjectEx("Location_Trigger", location.Spatial_TriggerPosition, ECE_NONE));
          location_trigger.SetCollisionCylinder(location.Spatial_TriggerRadius, location.Spatial_TriggerRadius / 2);
          location_trigger.Spatial_SetData(location);
          SetNotificationLocation(location_trigger, location);
          SpatialLoggerPrint("Trigger at location: " + location.Spatial_TriggerPosition + " - Radius: " + location.Spatial_TriggerRadius + " - Spawn location: " + location.Spatial_SpawnPosition);
          SpatialLoggerPrint("Faction: " + location.Spatial_Faction + " - Loadout: " + location.Spatial_ZoneLoadout + " - counts: " + location.Spatial_MinCount + ":" + location.Spatial_MaxCount);
        }
      } else SpatialLoggerPrint("Locations Disabled");
      SpatialDebugPrint("Spatial::Triggers - End");
    } //trigger zone initialisation

    void SetNotificationPoint(Spatial_Trigger trigger, Spatial_Point point){
      bool found = false;
        foreach(Spatial_Notification notification: m_Spatial_Notifications.notification) {
          if (notification.Spatial_Name == point.Spatial_Name) {
            trigger.SetNotification(notification);
            found = true;
          }
        }
      if (!found){
        trigger.SetNotification(new Spatial_Notification( "Default", m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText}));
      }
      }

    void SetNotificationLocation(Location_Trigger trigger, Spatial_Location location){
      bool found = false;
        foreach(Spatial_Notification notification: m_Spatial_Notifications.notification) {
          if (notification.Spatial_Name == location.Spatial_Name) {
            trigger.SetNotification(notification);
            found = true;
          }
        }
      if (!found){
        trigger.SetNotification(new Spatial_Notification( "Default", m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText}));
      }
      }

    void Spatial_message(PlayerBase player, int SpawnCount, Spatial_Group group, Spatial_Notification notification) {
      if (!player) return;
      string title, text, faction, loadout;
      int msg_no = notification.MessageType;
      title = notification.MessageTitle;
      text = notification.MessageText.GetRandomElement();
      faction = group.Spatial_Faction;
      loadout = group.Spatial_Loadout;
      
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

    void Spatial_WarningMessage(PlayerBase player, string message) {
      if ((player) && (message != "")) {
        Param1 < string > Msgparam;
        Msgparam = new Param1 < string > (message);
        GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
      }
    } //Ingame chat message

    void SpatialLoggerPrint(string msg) {
        GetExpansionSettings().GetLog().PrintLog("[Spatial AI] " + msg);
    } //expansion logging

    void SpatialDebugPrint(string msg) {
      if (GetSpatialSettings().Spatial_Debug())
        GetExpansionSettings().GetLog().PrintLog(string.Format("%1 %2", "[Spatial Debug] ", msg));
    } //expansion debug print
  }

  
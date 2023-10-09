/*
main class for init/group spawns.
check DateVersion every release.
*/

class SpatialAI {
    const string DateVersion = "Spatial AI Date: 9/10/2023 R29-7";
    const int SZ_IN_SAFEZONE = 0x0001; // TraderPlus compat
    int m_cur = 0;
    ref Spatial_Groups m_Spatial_Groups; // main config
    ref Spatial_Players m_Spatial_Players; // birthday config
    ref Spatial_Notifications m_Spatial_Notifications; // notification config

  #ifdef EXPANSIONMODSPAWNSELECTION
    private ExpansionRespawnHandlerModule m_RespawnModule;

    bool InRespawnMenu(PlayerIdentity identity) 
    {
        CF_Modules < ExpansionRespawnHandlerModule > .Get(m_RespawnModule);
        if (m_RespawnModule && m_RespawnModule.IsPlayerInSpawnSelect(identity)) {
            return true;
        }
        return false;
    } //LM's code altered to a bool in class
  #endif

    void SpatialAI() 
    {
        SpatialLoggerPrint(DateVersion);
    } //constructor - Unexpectedly, runs on external decon
    void Spatial_Init() 
    {
        GetSpatialSettings().PullRef(m_Spatial_Groups);
        SpatialPlayerSettings().PullRef(m_Spatial_Players);
        Spatial_NotificationSettings().PullRef(m_Spatial_Notifications);
    } //run in external constructor
    void Spatial_Check(array < Man > m_Players) 
    {
        SpatialDebugPrint("Spatial::Check - Start");
        // If there are no players in the list, return early.
        if (m_Players.Count() == 0) {
            SpatialDebugPrint("Spatial::Check - no players");
            return;
        }

        int playerChecks = m_Spatial_Groups.PlayerChecks;
        bool checkOnlyLeader = playerChecks >= 0;

        // Make sure playerChecks is positive, even if it was initially negative.
        playerChecks = Math.AbsInt(playerChecks);
        int playercount = m_Players.Count();
        int minAge = m_Spatial_Groups.MinimumAge;
        bool checkMinAge = minAge > 0;
        bool LoopLimit = playerChecks != 0;
        int playersindistance;
        for (int i = 1; i <= playercount; ++i) {
            SpatialDebugPrint("Spatial::Check - loop:" + i + " out of " + playercount);
            PlayerBase player = PlayerBase.Cast(m_Players.GetRandomElement());
            if (!player || !player.IsAlive() || !player.GetIdentity()) {
                SpatialDebugPrint("Spatial::Check - invalid player");
                continue;
            }

            m_Players.RemoveItem(player);

            if (GetGame().GetMission().IsPlayerDisconnecting(player)) {
                SpatialDebugPrint("Spatial::Check - player is disconnecting");
                continue;
            }
            if (checkOnlyLeader) {
                #ifdef EXPANSIONMODGROUPS
                ExpansionPartyData party = ExpansionPartyData.Cast(player.Expansion_GetParty());
                // If the player is not in a party or isnt leader, skip to the next iteration.
                if (!party || player.GetIdentity().GetId() != party.GetOwnerUID()) {
                    SpatialDebugPrint("Spatial::Check - player not leader of party");
                    continue;
                } else if (m_Spatial_Groups.MinimumPlayerDistance != 0) {
                    array < ref ExpansionPartyPlayerData > PartyMembers = party.GetPlayers();
                    int partycount = PartyMembers.Count();
                    playersindistance = GetCEApi().CountPlayersWithinRange(player.GetPosition(), m_Spatial_Groups.MinimumPlayerDistance);
                    //if there's more players than the player's party around, skip to the next iteration.
                    if (playersindistance > partycount + m_Spatial_Groups.MaxSoloPlayers) {
                        SpatialDebugPrint("Spatial::Check - too many players not in party around player");
                        continue;
                    }
                }
                #else
                eAIGroup PlayerGroup = eAIGroup.Cast(player.GetGroup());
                // If the player is not in an ai group or isnt leader, skip to the next iteration.
                if (!PlayerGroup || player != player.GetGroup().GetLeader()) {
                    SpatialDebugPrint("Spatial::Check - player not leader of group");
                    continue;
                } else if (m_Spatial_Groups.MinimumPlayerDistance != 0) {
                    int groupcount = PlayerGroup.Count();
                    playersindistance = GetCEApi().CountPlayersWithinRange(player.GetPosition(), m_Spatial_Groups.MinimumPlayerDistance);
                    //if there's more players than the player's ai around, skip to the next iteration.
                    if (playersindistance > groupcount + m_Spatial_Groups.MaxSoloPlayers) {
                        SpatialDebugPrint("Spatial::Check - too many players not in group around player");
                        continue;
                    }
                }
                #endif
            } else if (m_Spatial_Groups.MinimumPlayerDistance != 0) {
                playersindistance = GetCEApi().CountPlayersWithinRange(player.GetPosition(), m_Spatial_Groups.MinimumPlayerDistance);
                //if there's too many players around, skip to the next iteration.
                if (playersindistance > m_Spatial_Groups.MaxSoloPlayers) {
                    SpatialDebugPrint("Spatial::Check - player skipped, too many solo players around");
                    continue;
                }
            }

            #ifdef EXPANSIONMODSPAWNSELECTION
            // If the player is in the respawn menu, skip to the next iteration.
            if (InRespawnMenu(player.GetIdentity())) {
                SpatialDebugPrint("Spatial::Check - player in respawn");
                continue;
            }
            #endif

            if (checkMinAge) {
                if (!player.Spatial_CheckAge(minAge)) {
                    SpatialDebugPrint("Spatial::Check - user not old enough");
                    continue;
                }
            }
            if (Spatial_CanSpawn(player))
                Spatial_LocalSpawn(player);

            if (LoopLimit)
                if (i == playerChecks) return;
        }
        SpatialDebugPrint("Spatial::Check - End");
    } //Standard loop through the player list, selecting random players and removing them from the list. #refactored by LieutenantMaster
    bool Spatial_CanSpawn(PlayerBase player) 
    {
        SpatialDebugPrint("Spatial::CanSpawn - Start");
        if (player.IsBleeding() && !GetSpatialSettings().Spatial_IsBleeding())
        {
            SpatialDebugPrint("Spatial::Player is Bleeding");
            return false;
        }
        if (player.IsInVehicle() && !GetSpatialSettings().Spatial_InVehicle())
        {
            SpatialDebugPrint("Spatial::Player is In Vehicle");
            return false;
        }
        if (player.IsUnconscious() && !GetSpatialSettings().Spatial_IsUnconscious())
        {
            SpatialDebugPrint("Spatial::Player is Unconscious");
            return false;
        }
        if (player.IsRestrained() && !GetSpatialSettings().Spatial_IsRestrained())
        {
            SpatialDebugPrint("Spatial::Player is Restrained");
            return false;
        }
        #ifdef EXPANSIONMODBASEBUILDING
        // Check if player is inside their own territory and its override
        if (player.IsInsideOwnTerritory() && !GetSpatialSettings().Spatial_InOwnTerritory())
        {
            SpatialDebugPrint("Spatial::Player in own territory");
            return false;
        }
        #endif
        if (player.Expansion_IsInSafeZone() && !GetSpatialSettings().Spatial_IsInSafeZone())
        {
            SpatialDebugPrint("Spatial::Player Expansion Safe");
            return false;
        }

        #ifdef SZDEBUG
        // Check for specific safe zone debug status and its override
        if (player.GetSafeZoneStatus() == SZ_IN_SAFEZONE && !GetSpatialSettings().Spatial_TPSafeZone())
        {
            SpatialDebugPrint("Spatial::Player TraderPlus Safe");
            return false;
        }
        #endif
        if (m_Spatial_Groups.Points_Enabled == 2 && !player.Spatial_CheckZone())
        {
            SpatialDebugPrint("Spatial::Player not in zone");
            return false;
        }
        if (m_Spatial_Groups.Points_Enabled != 0 && player.Spatial_CheckSafe())
        {
            SpatialDebugPrint("Spatial::Player Safe");
            return false;
        }
            
        SpatialDebugPrint("Spatial::CanSpawn - End");
        return true;
    } //checks and overrides - #refactored by LieutenantMaster
    void Spatial_LocalSpawn(PlayerBase player) 
    {
        SpatialDebugPrint("Spatial::LocalSpawn - Start");
        if (!player) return;

        int m_Groupid = Math.RandomIntInclusive(0, 5000);
        SpatialDebugPrint("GroupID: " + m_Groupid);

        m_cur = Math.RandomIntInclusive(0, m_Spatial_Groups.Group.Count() - 1);
        int SpawnCount;
        Spatial_Group group;

        if (player.Spatial_CheckZone())
        {
            group = player.GetSpatialGroup();
        } 
        else 
        {
            switch (m_Spatial_Groups.ActiveHoursEnabled)
            {
                case 2:
                {
                    group = Spatial_GetTimeGroup(m_Spatial_Groups.Group);
                    if (!group) return;
                }
                break;
                case 3:
                {
                    group = Spatial_GetAgeGroup(m_Spatial_Groups.Group, player);
                    if (!group) return;
                }
                break;
                case 0:
                case 1:
                {
                    group = Spatial_GetWeightedGroup(m_Spatial_Groups.Group);
                }
                break;
            }
        }

        if (!group) {
            SpatialDebugPrint("Spatial::LocalSpawn - Error - No Group.");
            return;
        }

        SpawnCount = Math.RandomIntInclusive(group.Spatial_MinCount, group.Spatial_MaxCount);
        float random = Math.RandomFloat(0.0, 1.0);
        SpatialDebugPrint("Chance: " + group.Spatial_Chance + " | random: " + random);

        if (group.Spatial_Chance < random) return;

        if (SpawnCount > 0) {
            if (m_Spatial_Groups.GroupDifficulty == 1) {
                eAIGroup PlayerGroup = eAIGroup.Cast(player.GetGroup());
                int groupnumber = 0;
                int partynumber = 0;
                #ifdef EXPANSIONMODGROUPS

                ExpansionPartyData party = ExpansionPartyData.Cast(player.Expansion_GetParty());
                if (party) {
                    foreach(ExpansionPartyPlayerData partyPlayer: party.GetPlayers()) {
                        if (!partyPlayer || !partyPlayer.UID) continue;
                        PlayerBase online = PlayerBase.GetPlayerByUID(partyPlayer.UID);
                        if (online && online.GetIdentity()) ++partynumber;
                    }
                }

                #endif

                if (!PlayerGroup) groupnumber = 1;
                else groupnumber = PlayerGroup.Count();

                int totalcount = Math.Max(groupnumber, partynumber);
                if (totalcount > 1) SpawnCount += totalcount - 1;
            }
            Spatial_Spawn(player, SpawnCount, group);
        } else SpatialDebugPrint("group/point ai count too low this check");

        SpatialDebugPrint("End GroupID: " + m_Groupid);
        SpatialDebugPrint("Spatial::LocalSpawn - End");
    } //dealing with group/party stuff in a more restricted scope #refactored by wrdg
    Spatial_Group Spatial_GetWeightedGroup(array < ref Spatial_Group > groups, array < float > weights = NULL)
    {
        array < float > weights_T = weights;
        if (weights_T == NULL) {
            weights_T = new array < float > ;
            for (int i = 0; i < groups.Count(); ++i) {
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

    Spatial_Group Spatial_GetTimeGroup(array < ref Spatial_Group > groups) 
    {

        array < ref Spatial_Group > TimedGroups = {};
        Spatial_Notification notification;
        float Spatial_daytime = Spatial_GetTime();

        foreach(Spatial_Group group: groups) {
            foreach(Spatial_Notification notifcheck: m_Spatial_Notifications.notification) {
                if (!notifcheck.Spatial_Name) continue;
                if (notifcheck.Spatial_Name == group.Spatial_Name) {
                    if (Spatial_daytime >= notifcheck.StartTime && Spatial_daytime <= notifcheck.StopTime)
                        TimedGroups.Insert(group);
                }
            }
        }

        if (TimedGroups && TimedGroups.Count() > 0)
            return TimedGroups.GetRandomElement();

        SpatialDebugPrint("[Spatial_Group] No valid group times found.");
        return null;
    } //time based group selection

    Spatial_Group Spatial_GetAgeGroup(array < ref Spatial_Group > groups, PlayerBase player) 
    {

        array < ref Spatial_Group > AgeGroups = {};
        Spatial_Notification notification;
        CF_Date date = CF_Date.Epoch(player.m_Spatial_BirthdayDate());
        int hour =  date.GetHours();
        int minute = Math.IsInRange(date.GetMinutes(), 0, 59);;
        float time;

        if (minute == 0) time += hour;
        else time = (hour + (minute * 0.01));

        foreach(Spatial_Group group: groups) {
            foreach(Spatial_Notification notifcheck: m_Spatial_Notifications.notification) {
                if (!notifcheck.Spatial_Name) continue;
                if (notifcheck.Spatial_Name == group.Spatial_Name) {
                if (time >= notifcheck.AgeTime) AgeGroups.Insert(group);
                }
            }
        }

        if (AgeGroups && AgeGroups.Count() > 0)
            return AgeGroups.GetRandomElement();

        SpatialDebugPrint("[Spatial_Group] No valid group ages found.");
        return null;
    } //time based group selection

    bool Spatial_ValidPos(PlayerBase player, out vector pos) 
    {
        pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
        for (int i = 0; i < 50; ++i) {
            if (GetGame().SurfaceIsSea(pos[0], pos[2]) || GetGame().SurfaceIsPond(pos[0], pos[2])) {
                pos = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(player.GetPosition(), m_Spatial_Groups.MinDistance, m_Spatial_Groups.MaxDistance)));
            } else i = 50;
        }
        if (GetGame().SurfaceIsSea(pos[0], pos[2]) || GetGame().SurfaceIsPond(pos[0], pos[2])) return false;
        return true;
    } //fixed
    void Spatial_Spawn(PlayerBase player, int bod, Spatial_Group Group) 
    {
        SpatialDebugPrint("Spatial::Spawn - Start");
        vector startpos, waypointpos;
        if (!Spatial_ValidPos(player, startpos)) {
            SpatialLoggerPrint("Valid spawnpos not found. not spawning.");
            return;
        }
        if (!Spatial_ValidPos(player, waypointpos)) {
            SpatialLoggerPrint("Valid waypointpos not found. using startpos.");
            waypointpos = startpos;
        }
        TVectorArray waypoints = {
            waypointpos
        };
        int huntmode = m_Spatial_Groups.HuntMode;
        string Formation = "RANDOM";
        eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
        if (player.Spatial_CheckZone()) {
            huntmode = player.Spatial_HuntMode();
            if (player.Spatial_HuntMode() == 3)
                behaviour = typename.StringToEnum(eAIWaypointBehavior, "HALT");
        }
        int mindistradius, maxdistradius, despawnradius;
        mindistradius = 0;
        maxdistradius = 1200;
        despawnradius = 1200;
        auto dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, Group.Spatial_Loadout, bod, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(Group.Spatial_Faction), eAIFormation.Create(Formation), player, mindistradius, maxdistradius, despawnradius, huntmode, 3.0, Group.Spatial_Lootable, Group.Spatial_UnlimitedReload);
        if (dynPatrol) {
            dynPatrol.SetGroupName(Group.Spatial_Name);
            dynPatrol.SetAccuracy(Group.Spatial_MinAccuracy, Group.Spatial_MaxAccuracy);
            dynPatrol.SetSniperProneDistanceThreshold(0.0);
        }

        if (player.Spatial_CheckZone()) {
            Spatial_Message_parse(player, bod, Group, player.Spatial_notification());
        } else {

            Spatial_Notification notification;
            foreach(Spatial_Notification notifcheck: m_Spatial_Notifications.notification) {
                if (!notifcheck.Spatial_Name) continue;
                if (notifcheck.Spatial_Name == Group.Spatial_Name) {
                    notification = notifcheck;
                }
            }

            if (!notification) {
                notification = Spatial_Notification("Default", m_Spatial_Groups.ActiveStartTime, m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, { m_Spatial_Groups.MessageText });
            }
            Spatial_Message_parse(player, bod, Group, notification);
        }
        SpatialDebugPrint("Spatial::Spawn - End");
    } //Spatial_Spawn(player, SpawnCount, group)
    void InitSpatialTriggers() 
    {
        SpatialDebugPrint("Spatial::Triggers - Start");
        int i = 0;
        if (m_Spatial_Groups.Points_Enabled == 1 || m_Spatial_Groups.Points_Enabled == 2) {
            SpatialLoggerPrint("points Enabled");
            for (i = 0; i <= m_Spatial_Groups.Point.Count(); ++i) {
                Spatial_Point points = Spatial_Point.Cast(m_Spatial_Groups.Point[i]);
                if (!points) continue;
                Spatial_Trigger spatial_trigger = Spatial_Trigger.Cast(GetGame().CreateObjectEx("Spatial_Trigger", points.Spatial_Position, ECE_NONE));
                spatial_trigger.SetCollisionCylinder(points.Spatial_Radius, points.Spatial_Radius / 2);
                spatial_trigger.SetSpatialPoint(points);
                vector move0 = points.Spatial_Position;
                move0[1] = move0[1] - points.Spatial_Radius / 4;
                spatial_trigger.SetPosition(move0);
                SetNotificationPoint(spatial_trigger, points);
                SpatialLoggerPrint("Trigger at point: " + points.Spatial_Position + " - Radius: " + points.Spatial_Radius);
                SpatialLoggerPrint("- Safe: " + points.Spatial_Safe + " - Faction: " + points.Spatial_Faction + " - counts: " + points.Spatial_MinCount + ":" + points.Spatial_MaxCount);
                foreach (string loadout : points.Spatial_ZoneLoadout) SpatialLoggerPrint("- loadout added: " + loadout);
            }
        } else SpatialLoggerPrint("points Disabled");

        if (m_Spatial_Groups.Locations_Enabled != 0) {
            SpatialLoggerPrint("Locations Enabled");
            for (i = 0; i <= m_Spatial_Groups.Location.Count(); ++i) {
                Spatial_Location location = Spatial_Location.Cast(m_Spatial_Groups.Location[i]);
                if (!location) continue;
                Location_Trigger location_trigger = Location_Trigger.Cast(GetGame().CreateObjectEx("Location_Trigger", location.Spatial_TriggerPosition, ECE_NONE));
                Notification_Trigger notification_trigger = Notification_Trigger.Cast(GetGame().CreateObjectEx("Notification_Trigger", location.Spatial_TriggerPosition, ECE_NONE));
                location_trigger.SetCollisionCylinder(location.Spatial_TriggerRadius, location.Spatial_TriggerRadius / 2);
                notification_trigger.SetCollisionCylinder(location.Spatial_TriggerRadius * 2, location.Spatial_TriggerRadius);
                SetNotificationLocation(location_trigger, location);
                vector move1 = location.Spatial_TriggerPosition;
                move1[1] = move1[1] - (location.Spatial_TriggerRadius / 4);
                location_trigger.SetPosition(move1);
                notification_trigger.SetPosition(move1);
                location_trigger.Spatial_SetData(location, notification_trigger);
                SpatialLoggerPrint("Trigger at location: " + location.Spatial_TriggerPosition + " - Radius: " + location.Spatial_TriggerRadius);
                foreach (vector locpos : location.Spatial_SpawnPosition) SpatialLoggerPrint("- Spawn location: " + locpos);
                SpatialLoggerPrint("- Notification covering location to Radius: " + location.Spatial_TriggerRadius * 2);
                SpatialLoggerPrint("- Faction: " + location.Spatial_Faction + " - Loadout: " + location.Spatial_ZoneLoadout + " - counts: " + location.Spatial_MinCount + ":" + location.Spatial_MaxCount);

                #ifdef EXPANSIONMODNAVIGATION
                if (GetSpatialSettings().Spatial_Debug())
                    location_trigger.CreateMissionMarker(i, location_trigger.ValidPos(m_Spatial_Groups.Locations_Enabled, location.Spatial_TriggerPosition), location.Spatial_Name, m_Spatial_Groups.CleanupTimer, 0);
                #endif

            }
        } else SpatialLoggerPrint("Locations Disabled");

        if (m_Spatial_Groups.Audio_Enabled != 0) {
            SpatialLoggerPrint("Audio Sensitive Locations Enabled");
            for (i = 0; i <= m_Spatial_Groups.Audio.Count(); ++i) {
                Spatial_Audio audio = Spatial_Audio.Cast(m_Spatial_Groups.Audio[i]);
                if (!audio) continue;
                Audio_trigger audio_trigger = Audio_trigger.Cast(GetGame().CreateObjectEx("Audio_trigger", audio.Spatial_TriggerPosition, ECE_NONE));
                Notification_Trigger notification_trigger2 = Notification_Trigger.Cast(GetGame().CreateObjectEx("Notification_Trigger", audio.Spatial_TriggerPosition, ECE_NONE));
                audio_trigger.SetCollisionCylinder(audio.Spatial_TriggerRadius, audio.Spatial_TriggerRadius / 2);
                notification_trigger2.SetCollisionCylinder(audio.Spatial_TriggerRadius * 2, audio.Spatial_TriggerRadius);
                SetNotificationAudio(audio_trigger, audio);
                vector move2 = audio.Spatial_TriggerPosition;
                move2[1] = move2[1] - (audio.Spatial_TriggerRadius / 4);
                audio_trigger.SetPosition(move2);
                notification_trigger2.SetPosition(move2);
                audio_trigger.Spatial_SetData(audio, notification_trigger2);
                SpatialLoggerPrint("audio Trigger at location: " + audio.Spatial_TriggerPosition + " - Radius: " + audio.Spatial_TriggerRadius);
                foreach (vector audpos : audio.Spatial_SpawnPosition) SpatialLoggerPrint("- Spawn audio location: " + audpos);
                SpatialLoggerPrint("- Notification covering audio location to Radius: " + audio.Spatial_TriggerRadius * 2);
                SpatialLoggerPrint("- Faction: " + audio.Spatial_Faction + " - Loadout: " + audio.Spatial_ZoneLoadout + " - counts: " + audio.Spatial_MinCount + ":" + audio.Spatial_MaxCount);

                #ifdef EXPANSIONMODNAVIGATION
                if (GetSpatialSettings().Spatial_Debug())
                    audio_trigger.CreateMissionMarker(i, audio_trigger.ValidPos(m_Spatial_Groups.Audio_Enabled, audio.Spatial_TriggerPosition), audio.Spatial_Name, m_Spatial_Groups.CleanupTimer, 0);
                #endif

            }
        } else SpatialLoggerPrint("Audio Sensitive Locations Disabled");

        SpatialDebugPrint("Spatial::Triggers - End");
    } //trigger zone initialisation
    void SetNotificationPoint(Spatial_Trigger trigger, Spatial_Point point) 
    {
        bool found = false;
        for (int i = 0; i <= m_Spatial_Notifications.notification.Count(); ++i) {
            Spatial_Notification notification = Spatial_Notification.Cast(m_Spatial_Notifications.notification[i]);
            if (!notification) continue;
            if (notification.Spatial_Name == point.Spatial_Name) {
                trigger.SetNotification(notification);
                found = true;
                i = m_Spatial_Notifications.notification.Count();
            }
        }
        if (!found) {
            trigger.SetNotification(new Spatial_Notification("Default", m_Spatial_Groups.ActiveStartTime, m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, { m_Spatial_Groups.MessageText }));
        }
    } //set notification data to points
    void SetNotificationLocation(Location_Trigger trigger, Spatial_Location location) 
    {
        bool found = false;
        for (int i = 0; i <= m_Spatial_Notifications.notification.Count(); ++i) {
            Spatial_Notification notification = Spatial_Notification.Cast(m_Spatial_Notifications.notification[i]);
            if (!notification) continue;
            if (notification.Spatial_Name == location.Spatial_Name) {
                trigger.SetNotification(notification);
                found = true;
                i = m_Spatial_Notifications.notification.Count();
            }
        }

        if (!found) {
            trigger.SetNotification(new Spatial_Notification("Default", m_Spatial_Groups.ActiveStartTime, m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, { m_Spatial_Groups.MessageText }));
        }
    } //set notification data to Locations
    void SetNotificationAudio(Audio_trigger trigger, Spatial_Audio location) 
    {
        bool found = false;
        for (int i = 0; i <= m_Spatial_Notifications.notification.Count(); ++i) {
            Spatial_Notification notification = Spatial_Notification.Cast(m_Spatial_Notifications.notification[i]);
            if (!notification) continue;
            if (notification.Spatial_Name == location.Spatial_Name) {
                trigger.SetNotification(notification);
                found = true;
                i = m_Spatial_Notifications.notification.Count();
            }
        }

        if (!found) {
            trigger.SetNotification(new Spatial_Notification("Default", m_Spatial_Groups.ActiveStartTime, m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText}));
        }
    } //set notification data to audio locations
    void Spatial_Message_parse(PlayerBase player, int SpawnCount, Spatial_Group group, Spatial_Notification notification) 
    {
        #ifdef EXPANSIONMODGROUPS
        ExpansionPartyData party = ExpansionPartyData.Cast(player.Expansion_GetParty());
        if (party) {
            foreach(ExpansionPartyPlayerData partyPlayer: party.GetPlayers()) {
                if (!partyPlayer || !partyPlayer.UID) continue;
                PlayerBase online = PlayerBase.GetPlayerByUID(partyPlayer.UID);
                if (online && online.GetIdentity()) Spatial_message(online, SpawnCount, group, notification);
            }
        } else {
            Spatial_message(player, SpawnCount, group, notification);
        }
        return;
        #endif
        Spatial_message(player, SpawnCount, group, notification);
    } // deals with party members online #refactored by wrdg
    void Spatial_message(PlayerBase player, int SpawnCount, Spatial_Group group, Spatial_Notification notification) 
    {
        if (!player || !group) return;
        if (!notification) notification = Spatial_Notification("Default", m_Spatial_Groups.ActiveStartTime, m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, { m_Spatial_Groups.MessageText });
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
    void Spatial_WarningMessage(PlayerBase player, string message) 
    {
        if ((player) && (message != "")) {
            Param1 < string > Msgparam;
            Msgparam = new Param1 < string > (message);
            GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
        }
    } //Ingame chat message
    void SpatialLoggerPrint(string msg) 
    {
        GetExpansionSettings().GetLog().PrintLog("[Spatial AI] " + msg);
    } //expansion logging
    void SpatialDebugPrint(string msg) 
    {
        if (GetSpatialSettings().Spatial_Debug())
            GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
    } //expansion debug print
    float Spatial_GetTime() 
    {
        int pass, hour, minute;
        GetGame().GetWorld().GetDate(pass, pass, pass, hour, minute);
        if (minute == 0) return hour;
        return hour + (minute * 0.01);
    } //lightweight time, probably a better method.
}
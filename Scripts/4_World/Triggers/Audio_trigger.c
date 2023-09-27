//!TODO: change this whole system to avoidplayer
class Audio_trigger: Spatial_TriggerBase
{
    int i_PlayerCount;
    bool Spatial_TimerCheck;
    vector Spatial_SpawnPosition;

    eAISpatialPatrol dynPatrol;
    ref Spatial_Audio Audio;
    ref Spatial_Notification notification;
    Notification_Trigger notif_trigger;

    ref Spatial_Groups m_Spatial_Groups;
    autoptr array<ref TriggerInsider> notif;

    void Audio_trigger()
    {
      GetSpatialSettings().PullRef(m_Spatial_Groups);   
    }

    void Spatial_SetData(Spatial_Audio audio, Notification_Trigger b)
    {
      Audio = audio;
      notif_trigger = b;
    } //changed to class instead of individuals
    void SetNotification(Spatial_Notification a)
    {
      notification = a;
    }
    void SpawnCheck()
    {
      if (dynPatrol || Spatial_TimerCheck || m_insiders.Count() == 0) return;

      int m_Groupid = Math.RandomIntInclusive(10001, 15000);
      SpatialDebugPrint("audioID: " + m_Groupid);
      float random = Math.RandomFloat(0.0, 1.0);
      SpatialDebugPrint("Audio Chance: " + Audio.Spatial_Chance + " | random: " + random);
      if (Audio.Spatial_Chance < random) return;

      int SpawnCount = Math.RandomIntInclusive(Audio.Spatial_MinCount, Audio.Spatial_MaxCount);
      if (SpawnCount > 0)
      {
        Spatial_Spawn(SpawnCount, Audio);
        Spatial_TimerCheck = true;

    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          CreateMissionMarker(m_Groupid, ValidPos(m_Spatial_Groups.Audio_Enabled, Audio.Spatial_SpawnPosition), Audio.Spatial_Name + " Spawn", m_Spatial_Groups.CleanupTimer);
    #endif

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Spatial_timer, Audio.Spatial_Timer, false);
      } else {
        SpatialDebugPrint("Audio ai count too low this check");
      }
      SpatialDebugPrint("End audioID: " + m_Groupid);
    }

    void Spatial_timer()
    {
      Spatial_TimerCheck = false;
    }

    override void Enter(TriggerInsider insider)
    {
      PlayerBase player = PlayerBase.Cast(insider.GetObject());
      if (player && Audio)
      {
        player.Spatial_InLocation(true, Audio.Spatial_HuntMode);
      } 
      super.Enter(insider);
    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, player.GetPosition()), Audio.Spatial_Name + " Enter", m_Spatial_Groups.CleanupTimer);
    #endif
    }

    override void Leave(TriggerInsider insider)
    {
      PlayerBase player = PlayerBase.Cast(insider.GetObject());
      if (player)
      {
        player.Spatial_InLocation(false, 0);
      }
      super.Leave(insider);
    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, player.GetPosition()), Audio.Spatial_Name + " Exit", m_Spatial_Groups.CleanupTimer);
    #endif
    }


    //protected void OnStayStartServerEvent(int nrOfInsiders) {}

    override void OnStayStartServerEvent(int nrOfInsiders)
    {
      super.OnStayStartServerEvent(nrOfInsiders);
      if (nrOfInsiders == 0) return;

        int totalnoise = 0;
        if (nrOfInsiders > 0)
        {
          for (int i = 0; i < nrOfInsiders; ++i)
          {
              PlayerBase player = PlayerBase.Cast(m_insiders.Get(i).GetObject());
              if (!player) continue;
              int player_noise = player.Spatial_GetNoise();
              totalnoise += player_noise;
          }
          if (totalnoise > 0)
          {
            SpatialDebugPrint("Player noise in area: " + player_noise);
            if ((totalnoise / nrOfInsiders) > nrOfInsiders + 1)
            {
              SpatialDebugPrint("Spawning due to noise: " + player_noise);
              SpawnCheck();
              return;  
            }
          }
        }
    }

    void Spatial_Spawn(int count, Spatial_Audio audio)
    {
      if (m_insiders.Count() == 0) return;
      PlayerBase playerInsider = PlayerBase.Cast(m_insiders.Get(0).GetObject());
      if (!playerInsider || playerInsider.IsAI() || !playerInsider.GetIdentity()) return;
      SpatialDebugPrint(playerInsider.GetIdentity().GetName());
      vector startpos = ValidPos(m_Spatial_Groups.Audio_Enabled, Audio.Spatial_SpawnPosition);
      TVectorArray waypoints = { ValidPos(m_Spatial_Groups.Audio_Enabled, Audio.Spatial_SpawnPosition) };
      string Formation = "RANDOM";
      eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
      if (audio.Spatial_HuntMode == 3) 
        behaviour = typename.StringToEnum(eAIWaypointBehavior, "HALT");
      int mindistradius, maxdistradius, despawnradius;
      mindistradius = 0;
      maxdistradius = 1000;
      despawnradius = 1200;
      dynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, audio.Spatial_ZoneLoadout, count, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(audio.Spatial_Faction), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, audio.Spatial_HuntMode, 3.0, audio.Spatial_Lootable, audio.Spatial_UnlimitedReload);
      if (dynPatrol)
      {
        dynPatrol.SetAccuracy(audio.Spatial_MinAccuracy, audio.Spatial_MaxAccuracy);
        dynPatrol.SetGroupName(audio.Spatial_Name);
        dynPatrol.SetSniperProneDistanceThreshold(0.0);
      }
      notif = notif_trigger.GetInsiders();
      for (int i = 0; i < notif.Count(); ++i)
      {
        PlayerBase player = PlayerBase.Cast(notif[i].GetObject());
        if (player) Spatial_message(player, count);
      }
    }

    void Spatial_message(PlayerBase player, int SpawnCount)
    {
        if (!player) return;
        if (!notification)
        {
          notification = new Spatial_Notification( "Default", m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText});
        }
        string title, text, faction, loadout;
        int msg_no = notification.MessageType;
        title = notification.MessageTitle;
        text = notification.MessageText.GetRandomElement();
        faction = Audio.Spatial_Faction;
        loadout = Audio.Spatial_ZoneLoadout;
        
        string message = string.Format("Player: %1 Number: %2, Faction name: %3, Loadout: %4", player.GetIdentity().GetName(), SpawnCount, faction, loadout);
        if (msg_no == 0)
        {
          SpatialLoggerPrint(message);
        }else if (msg_no == 1)
        {
          Spatial_WarningMessage(player, string.Format("%1 %2", SpawnCount, text));
          SpatialLoggerPrint(message);
        } else if (msg_no == 2)
        {
          Spatial_WarningMessage(player, text);
          SpatialLoggerPrint(message);
        } else if (msg_no == 3)
        {
          NotificationSystem.SendNotificationToPlayerExtended(player, 5, title, string.Format("%1 %2", SpawnCount, text), "set:dayz_gui image:tutorials");
          SpatialLoggerPrint(message);
        } else if (msg_no == 4)
        {
          NotificationSystem.SendNotificationToPlayerExtended(player, 5, title, text, "set:dayz_gui image:tutorials");
          SpatialLoggerPrint(message);
        } else if (msg_no == 5 && player.Spatial_HasGPSReceiver())
        {
          NotificationSystem.SendNotificationToPlayerExtended(player, 5, title, text, "set:dayz_gui image:tutorials");
          SpatialLoggerPrint(message);
        }
    } //chat message or vanilla notification
}

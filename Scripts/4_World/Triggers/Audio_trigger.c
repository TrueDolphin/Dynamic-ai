//!TODO: change this whole system to avoidplayer
class Audio_trigger: Spatial_TriggerBase
{
    //allowed audio limits
    const int SOUND_WALK = 1;
    const int SOUND_JOG = 2;
    const int SOUND_RUN = 3;

    vector Spatial_SpawnPosition;
    ref Spatial_Audio Audio;
    Notification_Trigger notif_trigger;
    autoptr array<ref TriggerInsider> notif;

    void Spatial_SetData(Spatial_Audio audio, Notification_Trigger b)
    {
      Audio = audio;
      notif_trigger = b;
    } //changed to class instead of individuals

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

    override void Enter(TriggerInsider insider)
    {
      PlayerBase player = PlayerBase.Cast(insider.GetObject());
      if (player && Audio)
      {
        player.Spatial_InLocation(true, Audio.Spatial_HuntMode);
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
      if (nrOfInsiders == 0) return;

        int totalnoise = 0;
        if (nrOfInsiders > 0) //divide by zero
        {
          for (int i = 0; i < nrOfInsiders; ++i)
          {
              PlayerBase player = PlayerBase.Cast(m_insiders.Get(i).GetObject());
              if (!player) continue;
              int player_noise = player.Spatial_GetNoise();
              totalnoise += player_noise;
          }
          if (totalnoise > 0) //divide by zero
          {
            SpatialDebugPrint("Player noise in area: " + player_noise);
            if ((totalnoise / nrOfInsiders) > SOUND_JOG)
            {
              SpatialDebugPrint("Spawning due to noise: " + player_noise);
              SpawnCheck();
              return;  
            }
          }
        }
    }

    //next plan - split spawn back into Spatial_Group and add a vector array check to Spatial_TriggerPosition
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

}
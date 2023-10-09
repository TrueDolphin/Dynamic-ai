//!TODO: change this whole system to avoidplayer
class Audio_trigger: Spatial_TriggerBase
{
    //audio limit references
    const int SOUND_WALK = 1;
    const int SOUND_JOG = 2;
    const int SOUND_RUN = 3;
    const int SOUND_GUNFIRE = 1000;

    ref Spatial_Audio Audio; //trigger data
    Notification_Trigger notif_trigger; //notification trigger itselkf
    autoptr array<ref TriggerInsider> notif; //iterm player array

    void Spatial_SetData(Spatial_Audio audio, Notification_Trigger b)
    {
      Audio = audio;
      notif_trigger = b;
      TriggerName = audio.Spatial_Name;
      TriggerLoadout = audio.Spatial_ZoneLoadout;
      TriggerFaction = audio.Spatial_Faction;
    } //changed to class instead of individuals

    void SpawnCheck()
    {
      if (dynPatrol.Count() > 0 || Spatial_TimerCheck || m_insiders.Count() == 0) return;

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

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Spatial_timer, Audio.Spatial_Timer, false);
      } else {
        SpatialDebugPrint("Audio ai count too low this check");
      }
      SpatialDebugPrint("End audioID: " + m_Groupid);
    } //stack start

    override void Enter(TriggerInsider insider)
    {
      PlayerBase player = PlayerBase.Cast(insider.GetObject());
      if (player && Audio)
      {
        player.Spatial_InLocation(true, Audio.Spatial_HuntMode);
        player.Spatial_Firing(0);
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
      
      if (dynPatrol.Count() > 0 || Spatial_TimerCheck) return;

      if (nrOfInsiders > 0) //divide by zero
      {
        int totalnoise = 0;
        for (int i = 0; i < nrOfInsiders; ++i)
        {
            PlayerBase player = PlayerBase.Cast(m_insiders.Get(i).GetObject());
            if (!player) continue;
            int player_noise = player.Spatial_GetNoise();
            if (player_noise == 1000) 
            {
              totalnoise += SOUND_GUNFIRE;
              SpatialDebugPrint("Loud Noise Detected: " + player_noise);
            }
            else
            {
              player_noise = Math.Round(player_noise * 5);
              Math.Clamp(player_noise, 0, 5);
              totalnoise += player_noise;
            }
        }
        if (totalnoise > 0) //divide by zero
        {
          if ((totalnoise / nrOfInsiders) > Audio.Spatial_Sensitivity)
          {
            SpatialDebugPrint("Spawning due to noise: " + totalnoise);
            SpawnCheck();
            return;  
          }
        }
      }
    } //audio checks

    void Spatial_Spawn(int count, Spatial_Audio audio)
    {
      if (m_insiders.Count() == 0) return;
      PlayerBase playerInsider = PlayerBase.Cast(m_insiders.Get(0).GetObject());
      if (!playerInsider || !playerInsider.GetIdentity() || playerInsider.IsAI()) return;
      SpatialDebugPrint(playerInsider.GetIdentity().GetName());

      string Formation = "RANDOM";
      eAIWaypointBehavior behaviour = typename.StringToEnum(eAIWaypointBehavior, "ALTERNATE");
      if (audio.Spatial_HuntMode == 3) 
        behaviour = typename.StringToEnum(eAIWaypointBehavior, "HALT");
      vector startpos;
      TVectorArray waypoints;
      eAISpatialPatrol DynPatrol;
      int mindistradius, maxdistradius, despawnradius;
      mindistradius = 0;
      maxdistradius = 1000;
      despawnradius = 1200;

      if (Audio.Spatial_SpawnMode == 0) 
      {
        vector interm = Audio.Spatial_SpawnPosition.GetRandomElement();
        startpos = ValidPos(m_Spatial_Groups.Audio_Enabled, interm);
        waypoints = { ValidPos(m_Spatial_Groups.Audio_Enabled, interm) };
        DynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, audio.Spatial_ZoneLoadout, count, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(audio.Spatial_Faction), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, audio.Spatial_HuntMode, 3.0, audio.Spatial_Lootable, audio.Spatial_UnlimitedReload);
        if (DynPatrol)
        {
          DynPatrol.SetAccuracy(audio.Spatial_MinAccuracy, audio.Spatial_MaxAccuracy);
          DynPatrol.SetGroupName(audio.Spatial_Name);
          DynPatrol.SetSniperProneDistanceThreshold(0.0);
          dynPatrol.Insert(DynPatrol);

    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Audio_Enabled, startpos), Audio.Spatial_Name + " Spawn", m_Spatial_Groups.CleanupTimer);
    #endif
        } 
      }

      if (Audio.Spatial_SpawnMode == 1)
      {
        int recount = 0;
        foreach (vector pos : Audio.Spatial_SpawnPosition)
        {
          startpos = ValidPos(m_Spatial_Groups.Audio_Enabled, pos);
          waypoints = { ValidPos(m_Spatial_Groups.Audio_Enabled, pos) };
          DynPatrol = eAISpatialPatrol.CreateEx(startpos, waypoints, behaviour, audio.Spatial_ZoneLoadout, count, m_Spatial_Groups.CleanupTimer + 500, m_Spatial_Groups.CleanupTimer - 500, eAIFaction.Create(audio.Spatial_Faction), eAIFormation.Create(Formation), playerInsider, mindistradius, maxdistradius, despawnradius, audio.Spatial_HuntMode, 3.0, audio.Spatial_Lootable, audio.Spatial_UnlimitedReload);
          if (DynPatrol)
          {
            DynPatrol.SetAccuracy(audio.Spatial_MinAccuracy, audio.Spatial_MaxAccuracy);
            DynPatrol.SetGroupName(audio.Spatial_Name);
            DynPatrol.SetSniperProneDistanceThreshold(0.0);
            recount += count;
            dynPatrol.Insert(DynPatrol);

    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Audio_Enabled, startpos), Audio.Spatial_Name + " Spawn", m_Spatial_Groups.CleanupTimer);
    #endif
          }
        }
        count = recount;
      }

      notif = notif_trigger.GetInsiders();
      for (int i = 0; i < notif.Count(); ++i)
      {
        PlayerBase player = PlayerBase.Cast(notif[i].GetObject());
        if (player) Spatial_message(player, count);
      }
    }
    
}
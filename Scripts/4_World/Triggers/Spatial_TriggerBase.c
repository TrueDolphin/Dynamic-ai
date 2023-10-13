/*
base used for locations and audio.
13/10/2023
*/

class Spatial_TriggerBase: CylinderTrigger
{
    ref Spatial_Notification notification;
    ref Spatial_Groups m_Spatial_Groups;
    ref array<ref eAISpatialPatrol> dynPatrol;
    bool Spatial_TimerCheck;
    string TriggerName, TriggerLoadout, TriggerFaction;
    float PastTime;

#ifdef EXPANSIONMODNAVIGATION
   //declares
	ExpansionMarkerModule m_MarkerModule;
	ExpansionMarkerData m_ServerMarker;


    void CreateMissionMarker(int spawnuid, vector pos, string name, int timer, int remove = 1)
    {
        if (CF_Modules<ExpansionMarkerModule>.Get(m_MarkerModule))
        {
            m_ServerMarker = m_MarkerModule.CreateServerMarker(name, "Territory", pos, ARGB(255, 235, 59, 90), true);
            if (remove == 1)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RemoveMissionMarker, timer, false, m_ServerMarker.GetUID());
        }
    }

    void RemoveMissionMarker(string uid)
    {
      if ( !m_ServerMarker )
      return;
      m_MarkerModule.RemoveServerMarker( uid );
    }

#endif

    void Spatial_TriggerBase()
    {
      dynPatrol = {};
      PastTime = 0;
      GetSpatialSettings().PullRef(m_Spatial_Groups);
    } // main reference pull

    void SpawnCheck(){/*override me*/}
    void Spatial_Spawn(int count){/*override me*/}

    bool CanSpawn()
    {
      
      if (dynPatrol.Count() != 0 || Spatial_TimerCheck == true || m_insiders.Count() == 0)
          return false;

      if (!dynPatrol) dynPatrol = {};

      if (!notification)
        notification = new Spatial_Notification( "Default", m_Spatial_Groups.ActiveStartTime , m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText});
      
      if (m_Spatial_Groups.ActiveHoursEnabled != 0 && m_Spatial_Groups.ActiveHoursEnabled != 3)
      {
          float time = GetTime();
          if (!PastTime || PastTime != time)
          {
            SpatialDebugPrint(TriggerName + ": " + time + " compare: " + notification.StartTime + ";" + notification.StopTime);
             PastTime = time;
          }
          if (time <= notification.StartTime || time >= notification.StopTime) false;
      }
      return true;
    } //init checks
    bool ValidSpawn(vector pos)
    {
          return !GetCEApi().AvoidPlayer(pos, 5);
    } //unused currently
    float GetTime()
    {
      int pass, hour, minute;
      GetGame().GetWorld().GetDate(pass, pass, pass, hour, minute);
      if (minute == 0) return hour;
      return hour + (minute * 0.01);
    } //find better method
    vector ValidPos(int enabled, vector pos)
    {
        if (enabled == 2) return pos;
        return ExpansionStatic.GetSurfacePosition(pos);
    } //parser
    override void OnStayStartServerEvent(int nrOfInsiders)
    {
      super.OnStayStartServerEvent(nrOfInsiders);
      if (!dynPatrol || Spatial_TimerCheck) return;
      if (dynPatrol.Count() != 0)
      {
        int pdx;
        foreach (eAISpatialPatrol patrol : dynPatrol)
        {
          if (patrol && !patrol.GetGroup())
          {
            pdx = dynPatrol.Find(patrol);
		        if (pdx != -1) dynPatrol.Remove(pdx);
            patrol.Despawn();
            SpatialDebugPrint("patrol removed from array");
          }
          if (patrol && patrol.GetGroup())
          {
            int d = patrol.GetGroup().Count();
            eAIGroup group = patrol.GetGroup();
            int e = 0;
            for (int i = 0; i < d; ++i)
            {
              if (group.GetMember(i).IsAlive()) e += 1;
            }

            if (e == 0)
            {
              pdx = dynPatrol.Find(patrol);
              if (pdx != -1) dynPatrol.Remove(pdx);
              patrol.Despawn();
              SpatialDebugPrint("patrol removed from array");
            }
          } 
        }
      }
    } //make this better. jesus.
    override void Enter(TriggerInsider insider)
    {
      super.Enter(insider);
    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          {
            PlayerBase PlayerMarker = PlayerBase.Cast(insider.GetObject());
            CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, PlayerMarker.GetPosition()), TriggerName + " Enter", m_Spatial_Groups.CleanupTimer);
          }
    #endif
    } //marker enter
    override void Leave(TriggerInsider insider)
    {
      super.Leave(insider);
    #ifdef EXPANSIONMODNAVIGATION
          if (GetSpatialSettings().Spatial_Debug())
          {
          PlayerBase PlayerMarker = PlayerBase.Cast(insider.GetObject());
          CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, PlayerMarker.GetPosition()), TriggerName + " Leave", m_Spatial_Groups.CleanupTimer);
          }
    #endif
    } //marker leave
    override protected bool CanAddObjectAsInsider(Object object)
    {
      if (!notification)
        notification = new Spatial_Notification( "Default", m_Spatial_Groups.ActiveStartTime , m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText});
      
      if (!super.CanAddObjectAsInsider(object)) return false;
      PlayerBase player = PlayerBase.Cast(object);
      if (!player || !player.GetIdentity() || player.IsAI()) return false;

      return true;
    } //override for restrictions
    void Spatial_WarningMessage(PlayerBase player, string message)
    {
      if ((player) && (message != ""))
      {
          Param1 < string > Msgparam;
          Msgparam = new Param1 < string > (message);
          GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
      }
    } //Ingame chat message
    void SpatialDebugPrint(string msg)
    {
          if (GetSpatialSettings().Spatial_Debug())
              GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
    } //expansion debug print
    void SpatialLoggerPrint(string msg)
    {
          GetExpansionSettings().GetLog().PrintLog("[Spatial AI] " + msg);
    } //expansion logging
    void Spatial_message(PlayerBase player, int SpawnCount)
    {
        if (!player) return;
        if (!notification)
        {
          notification = new Spatial_Notification( "Default", m_Spatial_Groups.ActiveStartTime , m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText});
        }
        string title, text, faction, loadout;
        int msg_no = notification.MessageType;
        title = notification.MessageTitle;
        text = notification.MessageText.GetRandomElement();
        faction = TriggerFaction;
        loadout = TriggerLoadout;
        
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
    void Spatial_timer()
    {
      Spatial_TimerCheck = false;
    } //calllater timer for respawn
    void SetNotification(Spatial_Notification a)
    {
      notification = a;
    } //notification data
}
/*
used for standard group replacement triggers
4/10/2023
*/

class Spatial_Trigger: CylinderTrigger
{     
    bool Zone_Status;   
    Spatial_Point m_spatial_point;
    ref Spatial_Notification notification;
    ref Spatial_Groups m_Spatial_Groups;
    

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

  vector ValidPos(int enabled, vector pos)
  {
      if (enabled == 2) return pos;
      return ExpansionStatic.GetSurfacePosition(pos);
  }
#endif

    void Spatial_Trigger()
    {
      GetSpatialSettings().PullRef(m_Spatial_Groups);   
    }

    void SetSpatialPoint(Spatial_Point point)
    {
        m_spatial_point = point;
        Zone_Status = point.Spatial_Safe;

      #ifdef EXPANSIONMODNAVIGATION
            if (GetSpatialSettings().Spatial_Debug())
            {
              CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, GetPosition()), m_spatial_point.Spatial_Name, m_Spatial_Groups.CleanupTimer, 0);
            }
      #endif
    } //changed to class instead of individuals

    void SetNotification(Spatial_Notification a)
    {
        notification = a;
    }

    override void OnStayStartServerEvent(int nrOfInsiders)
    {
      super.OnStayStartServerEvent(nrOfInsiders);
      if (nrOfInsiders == 0) return;
      for (int i = 0; i < nrOfInsiders; ++i)
      {
        PlayerBase player = PlayerBase.Cast(m_insiders.Get(i).GetObject());
        if (player)
        {
          if (player.Spatial_CheckZone() || player.Spatial_CheckSafe() == Zone_Status) return;
          player.Spatial_SetInZone(true);
          player.Spatial_SetSafe(Zone_Status);
          player.SetSpatialPoint(m_spatial_point);
          player.Spatial_SetNotification(notification);
        }
      }
    }

    override void Enter(TriggerInsider insider)
    {
        super.Enter(insider);
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (player)
        {
            player.Spatial_SetInZone(true);
            player.Spatial_SetSafe(Zone_Status);
            player.SetSpatialPoint(m_spatial_point);
            player.Spatial_SetNotification(notification);

      #ifdef EXPANSIONMODNAVIGATION
            if (GetSpatialSettings().Spatial_Debug())
            {
              CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, player.GetPosition()), m_spatial_point.Spatial_Name + " Enter", m_Spatial_Groups.CleanupTimer);
            }
      #endif
        }
    }
        
    override void Leave(TriggerInsider insider)
    {
        super.Leave(insider);
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (player)
        {
            player.Spatial_SetSafe(false);
            player.Spatial_SetInZone(false);
      #ifdef EXPANSIONMODNAVIGATION
            if (GetSpatialSettings().Spatial_Debug())
            {
              CreateMissionMarker(0, ValidPos(m_Spatial_Groups.Locations_Enabled, player.GetPosition()), m_spatial_point.Spatial_Name + " Leave", m_Spatial_Groups.CleanupTimer);
            }
      #endif
        }
    }
    
    float GetTime()
    {
      int pass, hour, minute;
      GetGame().GetWorld().GetDate(pass, pass, pass, hour, minute);
      if (minute < 1) return hour;
      return hour + (minute * 0.01);
    }

    override protected bool CanAddObjectAsInsider(Object object)
    {
      if (!notification)
        notification = new Spatial_Notification( "Default", m_Spatial_Groups.ActiveStartTime , m_Spatial_Groups.ActiveStopTime, 0, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText});
      
      if (!super.CanAddObjectAsInsider(object)) return false;
      PlayerBase player = PlayerBase.Cast(object);
      if (!player || !player.GetIdentity() || player.IsAI()) return false;

      if (m_Spatial_Groups.ActiveHoursEnabled != 0 && m_Spatial_Groups.ActiveHoursEnabled != 3)
      {
          float time = GetTime();
          SpatialDebugPrint(m_spatial_point.Spatial_Name + ": " + time + " compare: " + notification.StartTime + ";" + notification.StopTime);
          if (time <= notification.StartTime || time >= notification.StopTime) return false;
      }
      return true;
    }

    void SpatialDebugPrint(string msg)
    {
          if (GetSpatialSettings().Spatial_Debug())
              GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
    } //expansion debug print
}
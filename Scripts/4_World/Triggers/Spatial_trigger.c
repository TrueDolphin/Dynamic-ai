/*
used for standard group replacement triggers
4/10/2023
*/

class Spatial_Trigger: CylinderTrigger
{     
    bool Zone_Status;   
    Spatial_Point m_spatial_point;
    Spatial_Notification notification;
    ref Spatial_Groups m_Spatial_Groups;
    

    void Spatial_Trigger()
    {
      GetSpatialSettings().PullRef(m_Spatial_Groups);   
    }

    void SetSpatialPoint(Spatial_Point point)
    {
        m_spatial_point = point;
        Zone_Status = point.Spatial_Safe;
    } //changed to class instead of individuals

    void SetNotification(Spatial_Notification a)
    {
        notification = a;
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
        }
    }
    
    float GetTime()
    {
      int pass, hour, minute;
      GetGame().GetWorld().GetDate(pass, pass, pass, hour, minute);
      if (minute == 0) return hour;
      return hour + (minute * 0.01);
    }

    override protected bool CanAddObjectAsInsider(Object object)
    {
      if (!notification)
      {
        notification = Spatial_Notification( "Default", m_Spatial_Groups.ActiveStartTime , m_Spatial_Groups.ActiveStopTime, m_Spatial_Groups.MessageType, m_Spatial_Groups.MessageTitle, {m_Spatial_Groups.MessageText});
      } 
      float time = GetTime();
      if (time >= notification.StartTime && time <= notification.StopTime)
      {
        if (!super.CanAddObjectAsInsider(object)) return false;
        PlayerBase player = PlayerBase.Cast(object);
        if (!player || !player.GetIdentity() || player.IsAI()) return false;
        return true;
      }
      return false;
    }
}
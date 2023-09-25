class Spatial_Trigger: CylinderTrigger
{     
    bool Zone_Status;   
    Spatial_Point m_spatial_point;
    Spatial_Notification notification;
    
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
    
  override protected bool CanAddObjectAsInsider(Object object)
  {
    if (!super.CanAddObjectAsInsider(object)) return false;
    PlayerBase player = PlayerBase.Cast(object);
    if (!player || !player.GetIdentity() || player.IsAI()) return false;
    return true;
  }
}
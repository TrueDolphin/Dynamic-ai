class Notification_Trigger: CylinderTrigger
{     

    override void Enter(TriggerInsider insider)
    {
        super.Enter(insider);
    }
        
    override void Leave(TriggerInsider insider)
    {
        super.Leave(insider);
    }
    
  override protected bool CanAddObjectAsInsider(Object object)
  {
    if (!super.CanAddObjectAsInsider(object)) return false;
    PlayerBase player = PlayerBase.Cast(object);
    if (!player || !player.GetIdentity() || player.IsAI()) return false;
    return true;
  }
}
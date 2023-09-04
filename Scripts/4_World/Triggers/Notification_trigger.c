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
        bool ai = eAIBase.Cast(object) != null;
        if (ai) return false;
        return PlayerBase.Cast(object) != null;
    }
}
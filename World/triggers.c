class Dynamic_Trigger: CylinderTrigger
{     
    bool Zone_Status;   
    string Zone_Faction;
    string Zone_Loadout;
    int Dynamic_MinCount;
    int Dynamic_MaxCount;

    void Dynamic_SetData(bool saf, string fac, string lod, int c, int d){
    Zone_Status = saf;
    Zone_Faction = fac;
    Zone_Loadout = lod;
    Dynamic_MinCount = c;
    Dynamic_MaxCount = d;
    }

    override void Enter(TriggerInsider insider)
    {
        super.Enter(insider);
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (player) {
            player.SetInZone(true);
            player.SetSafe(Zone_Status);
            player.Dynamic_SetData(Zone_Faction, Zone_Loadout, Dynamic_MinCount, Dynamic_MaxCount);
        }
    }
        
    override void Leave(TriggerInsider insider)
    {
        super.Leave(insider);
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (player) {
            player.SetSafe(false);
            player.SetInZone(false);
        }
    }
    
    override protected bool CanAddObjectAsInsider(Object object)
    {

		if (!super.CanAddObjectAsInsider(object))
		{
			return false;
		}
        return PlayerBase.Cast(object) != null;
    }
}
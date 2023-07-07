class Spatial_Trigger: CylinderTrigger
{     
    bool Zone_Status;   
    string Zone_Faction, Zone_Loadout, Spatial_Name;
    int Spatial_MinCount, Spatial_MaxCount, Spatial_HuntMode, Spatial_Lootable; 
    float Spatial_Chance;


    void Spatial_SetData(bool saf, string fac, string lod, int c, int d, int e, string f, int g, float h){
        Zone_Status = saf;
        Zone_Faction = fac;
        Zone_Loadout = lod;
        Spatial_MinCount = c;
        Spatial_MaxCount = d;
        Spatial_HuntMode = e;
        Spatial_Name = f;
        Spatial_Lootable = g;
        Spatial_Chance = h;
    }

    override void Enter(TriggerInsider insider)
    {
        super.Enter(insider);
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (player) {
            player.SetInZone(true);
            player.SetSafe(Zone_Status);
            player.Spatial_SetData(Zone_Faction, Zone_Loadout, Spatial_MinCount, Spatial_MaxCount, Spatial_HuntMode, Spatial_Name, Spatial_Lootable, Spatial_Chance);
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

		if (!super.CanAddObjectAsInsider(object)) return false;
        return PlayerBase.Cast(object) != null;
    }
}
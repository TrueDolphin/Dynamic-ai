class Spatial_Trigger: CylinderTrigger
{     
    bool Zone_Status, Spatial_UnlimitedReload;   
    string m_Zone_Faction, m_Zone_Loadout, Spatial_Name;
    int Spatial_MinCount, Spatial_MaxCount, Spatial_HuntMode, Spatial_Lootable; 
    float Spatial_Chance;
    


    void Spatial_SetData(bool saf, string fac, string lod, int c, int d, int e, string f, int g, float h, bool i){
        Zone_Status = saf;
        m_Zone_Faction = fac;
        m_Zone_Loadout = lod;
        Spatial_MinCount = c;
        Spatial_MaxCount = d;
        Spatial_HuntMode = e;
        Spatial_Name = f;
        Spatial_Lootable = g;
        Spatial_Chance = h;
        Spatial_UnlimitedReload = i;
    }

    override void Enter(TriggerInsider insider)
    {
        super.Enter(insider);
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (player) {
            player.Spatial_SetInZone(true);
            player.Spatial_SetSafe(Zone_Status);
            player.Spatial_SetData(m_Zone_Faction, m_Zone_Loadout, Spatial_MinCount, Spatial_MaxCount, Spatial_HuntMode, Spatial_Name, Spatial_Lootable, Spatial_Chance, Spatial_UnlimitedReload);
        }
    }
        
    override void Leave(TriggerInsider insider)
    {
        super.Leave(insider);
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (player) {
            player.Spatial_SetSafe(false);
            player.Spatial_SetInZone(false);
        }
    }
    
    override protected bool CanAddObjectAsInsider(Object object)
    {

		if (!super.CanAddObjectAsInsider(object)) return false;
        return PlayerBase.Cast(object) != null;
    }
}
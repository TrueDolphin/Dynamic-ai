modded class Weapon_Base
{

    override void EEFired(int muzzleType, int mode, string ammoType)
    {
        super.EEFired(muzzleType, mode, ammoType);
        g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Spatial_Firing);
        Spatial_Firing(true); 
        g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Spatial_Firing, 300, false, false);
    }

    void Spatial_Firing(bool a) 
    {
        if (g_Game.IsServer())
        {
            PlayerBase spatial_player = PlayerBase.Cast(GetHierarchyRootPlayer());
            if (spatial_player)
            {
                spatial_player.Spatial_Firing(a);
            }
        }
    }

}
modded class Weapon_Base
{

    override void OnFire(int muzzle_index)
        {
            super.OnFire(muzzle_index);

            if (GetGame().IsServer())
            {
                PlayerBase spatial_player = PlayerBase.Cast(GetHierarchyRootPlayer());
                if (spatial_player)
                {
                    spatial_player.Spatial_Firing(0);
                }
            }
        }//triggers second

    override void EEFired(int muzzleType, int mode, string ammoType)
        {
        super.EEFired(muzzleType, mode, ammoType);

            if (GetGame().IsServer())
            {
                PlayerBase spatial_player = PlayerBase.Cast(GetHierarchyRootPlayer());
                if (spatial_player)
                {
                    spatial_player.Spatial_Firing(5);
                }

            }
        }//triggers first

}
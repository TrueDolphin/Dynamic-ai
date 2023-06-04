/*
altered for whitelist. LM rewrote my over-the-top if checks to this.
*/

modded class ItemBase
{	
    override void Expansion_SetLootable(bool pState)
    {

        if (GetSpatialSettings().Spatial_WhiteList() != NULL) {        
            foreach (string pName : GetSpatialSettings().Spatial_WhiteList()){
            if (!pName) return;
            if (IsKindOf(pName)) pState = true;
            }
        }
        super.Expansion_SetLootable(pState);
    }
}
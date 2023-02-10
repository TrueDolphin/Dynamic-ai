/*
altered for whitelist. LM rewrote my over-the-top if checks to this.
*/

modded class ItemBase
{	
    override void Expansion_SetLootable(bool pState)
    {

        if (GetDynamicSettings().Dynamic_WhiteList() != NULL) {        
            foreach (string pName : GetDynamicSettings().Dynamic_WhiteList()){
            if (!pName) return;
            if (IsKindOf(pName)) pState = true;
            }
        }
        super.Expansion_SetLootable(pState);
    }
}
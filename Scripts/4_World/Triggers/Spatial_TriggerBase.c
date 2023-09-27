class Spatial_TriggerBase: CylinderTrigger
{


#ifdef EXPANSIONMODNAVIGATION
   //declares
	ExpansionMarkerModule m_MarkerModule;
	ExpansionMarkerData m_ServerMarker;


    void CreateMissionMarker(int spawnuid, vector pos, string name, int timer, int remove = 1)
    {
        if (CF_Modules<ExpansionMarkerModule>.Get(m_MarkerModule))
        {
            m_ServerMarker = m_MarkerModule.CreateServerMarker(name, "Territory", pos, ARGB(255, 235, 59, 90), true);
            if (remove == 1)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RemoveMissionMarker, timer, false, m_ServerMarker.GetUID());
        }

    }

	void RemoveMissionMarker(string uid)
	{
		if ( !m_ServerMarker )
		return;
		m_MarkerModule.RemoveServerMarker( uid );
	}
#endif

  vector ValidPos(int enabled, vector pos)
    {
        if (enabled == 2) return pos;
        return ExpansionStatic.GetSurfacePosition(pos);
    }

  bool ValidSpawn(vector pos)
  {
        return !GetCEApi().AvoidPlayer(pos, 5);
  }

  override protected bool CanAddObjectAsInsider(Object object)
  {
        if (!super.CanAddObjectAsInsider(object)) return false;
        PlayerBase player = PlayerBase.Cast(object);
        if (!player || !player.GetIdentity() || player.IsAI()) return false;
        return true;
  }

  void Spatial_WarningMessage(PlayerBase player, string message)
  {
    if ((player) && (message != ""))
    {
        Param1 < string > Msgparam;
        Msgparam = new Param1 < string > (message);
        GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
    }
  } //Ingame chat message

  void SpatialDebugPrint(string msg)
  {
        if (GetSpatialSettings().Spatial_Debug())
            GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
  } //expansion debug print

  void SpatialLoggerPrint(string msg)
  {
        GetExpansionSettings().GetLog().PrintLog("[Spatial AI] " + msg);
  } //expansion logging

}

  /*
  Template turned idea for mirroring internal patrols but with map warnings
  requires navigation so....questionable for native additions.

      #ifdef EXPANSIONMODNAVIGATION
      CreateMissionMarker("Test Position", player.GetPosition(), 5);
	    #endif

	#ifdef EXPANSIONMODNAVIGATION

	private ExpansionMarkerModule m_MarkerModule;
	private ExpansionMarkerData m_ServerMarker;

	void CreateMissionMarker(string markerName, vector location, int timer)
	{
		if (!m_MarkerModule){
      initMarkerModule();
      if (!m_MarkerModule) {
        Print("MarkerModule error");
        return;
      }
    }
		m_ServerMarker = m_MarkerModule.CreateServerMarker( markerName, "Territory", location, ARGB(255, 235, 59, 90), true);
   GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.RemoveMissionMarker, timer, false, m_ServerMarker.GetUID());
				 
	}
  void initMarkerModule(){
    CF_Modules<ExpansionMarkerModule>.Get(m_MarkerModule);
  }

	void RemoveMissionMarker(string uid)
	{
		if ( !m_ServerMarker )
			return;
		m_MarkerModule.RemoveServerMarker( uid );
	}
	#endif
*/
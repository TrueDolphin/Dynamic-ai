
/*
   //declares
	#ifdef EXPANSIONMODNAVIGATION
	ExpansionMarkerModule m_MarkerModule;
	ExpansionMarkerData m_ServerMarker;
	#endif

	//create
    #ifdef EXPANSIONMODNAVIGATION
		if (CF_Modules<ExpansionMarkerModule>.Get(m_MarkerModule))
		m_ServerMarker = m_MarkerModule.CreateServerMarker("AI Spawn", "Territory", ai.GetPosition(), ARGB(255, 235, 59, 90), true);
    #endif

	//remove
	#ifdef EXPANSIONMODNAVIGATION
	GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(removeMissionMarker), 15000, false, m_ServerMarker.GetUID());
	#endif


	#ifdef EXPANSIONMODNAVIGATION
	void RemoveMissionMarker(string uid)
	{
		if ( !m_ServerMarker )
		return;
		m_MarkerModule.RemoveServerMarker( uid );
	}
	#endif
	*/
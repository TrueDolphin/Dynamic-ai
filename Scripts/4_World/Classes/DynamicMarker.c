
/*
  doesnt work. idk why


   //declares
	#ifdef EXPANSIONMODNAVIGATION
	[NonSerialized()]
	ExpansionMarkerModule m_MarkerModule;

	[NonSerialized()]
	ExpansionMarkerData m_ServerMarker;
	#endif

	//create
    #ifdef EXPANSIONMODNAVIGATION
		if (CF_Modules<ExpansionMarkerModule>.Get(m_MarkerModule))
			m_ServerMarker = m_MarkerModule.CreateServerMarker("AI Spawn", "Territory", ai.GetPosition(), ARGB(255, 235, 59, 90), true);
    #endif


	//remove
	#ifdef EXPANSIONMODNAVIGATION
    //thread RemoveMissionMarker(m_ServerMarker.GetUID(), timer);
	#endif

	#ifdef EXPANSIONMODNAVIGATION
	void RemoveMissionMarker(string uid, int timer)
	{
    Sleep(timer);
		if ( !m_ServerMarker )
			return;
		m_MarkerModule.RemoveServerMarker( uid );
	}
	#endif

*/
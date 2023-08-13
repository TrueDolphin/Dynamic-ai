
//polygon zone - wip

class SpatialPatrol : Managed
{
	private static autoptr array<ref SpatialPatrol> m_AllPatrols = new array<ref SpatialPatrol>();
    private ref array<vector> m_PolygonVertices;
	static const float UPDATE_RATE_IN_SECONDS = 5.0;
	
	private ref Timer m_Timer;
	private bool m_IsBeingDestroyed;

	static void DeletePatrol(SpatialPatrol patrol){
		#ifdef EAI_TRACE
		auto trace = CF_Trace_1("SpatialPatrol", "DeletePatrol").Add(patrol);
		#endif

		int index = m_AllPatrols.Find(patrol);
		m_AllPatrols.Remove(index);
	}

    private void SpatialPatrol(array<vector> polygonVertices){
        #ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "SpatialPatrol");
		#endif
        m_PolygonVertices = polygonVertices;
        m_AllPatrols.Insert(this);
    }

	private void ~SpatialPatrol(){
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "~SpatialPatrol");
		#endif

		if (!GetGame())
			return;

		int idx = m_AllPatrols.Find(this);
		if (idx != -1) m_AllPatrols.RemoveOrdered(idx);

		Stop();
	}
	
	static void DebugAll(){
		Print("DebugAll");
		Print(m_AllPatrols.Count());
		foreach (auto patrol : m_AllPatrols)
		{
			patrol.Debug();
		}
	}
	
	static void DeleteAll(){
		m_AllPatrols.Clear();
	}
	
	void Debug(){
		Print(Type());
		Print(m_Timer);
	}

	void Delete(){
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "Delete");
		#endif

		m_IsBeingDestroyed = true;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(DeletePatrol, this);
	}

	bool IsBeingDestroyed(){
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "IsBeingDestroyed");
		#endif

		return m_IsBeingDestroyed;
	}

	void Start(){
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "Start");
		#endif

		//DelayedStart();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedStart, Math.RandomInt(1, 1000), false);
	}

	private void DelayedStart(){
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "DelayedStart");
		#endif

		OnUpdate();

		if (!m_Timer) m_Timer = new Timer(CALL_CATEGORY_GAMEPLAY);
		m_Timer.Run(UPDATE_RATE_IN_SECONDS, this, "OnUpdate", null, true);
	}

	void Stop(){
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "Stop");
		#endif

		if (m_Timer && m_Timer.IsRunning()) m_Timer.Stop();
	}

    void Check(vector position){
        int windingNumber = 0;

        int n = m_PolygonVertices.Count();
        for (int i = 0; i < n; i++)
        {
            vector v1 = m_PolygonVertices[i];
            vector v2 = m_PolygonVertices[(i + 1) % n];

            if (v1[2] <= position[2])
            {
                if (v2[2] > position[2] && isLeft(v1, v2, position) > 0)
                {
                    windingNumber++;
                }
            }
            else
            {
                if (v2[2] <= position[2] && isLeft(v1, v2, position) < 0)
                {
                    windingNumber--;
                }
            }
        }

        s_InsideBuffer[m_Type] = windingNumber != 0;
    }

    private float isLeft(vector v1, vector v2, vector p){
        return (v2[0] - v1[0]) * (p[2] - v1[2]) - (p[0] - v1[0]) * (v2[2] - v1[2]);
    }

	void OnUpdate(){}
};

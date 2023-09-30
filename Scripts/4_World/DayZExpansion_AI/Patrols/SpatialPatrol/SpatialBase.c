/*
moved spatial stuff as much as possible out of the eAISpatialPatrol class
sick of little things breaking because of dumb ideas/silly mistakes.

30/9/2023
*/

class SpatialBase : SpatialPatrol
{

    //compatibility moves
	vector m_Position;
	ref eAIGroup m_Group;

    //additionals
	ref Spatial_Groups m_Spatial_Groups;
	PlayerBase m_Hunted;
	int m_lootcheck;
	int m_Location = 0;
	int m_Huntmode;

	void SpatialBase()
    {
		GetSpatialSettings().PullRef(m_Spatial_Groups);	
	}//Spatial settings reference

    void Spatial_Movement(int m_Mode)
    {
		eAIBase ai = eAIBase.Cast(m_Group.GetLeader());
		PlayerBase player = m_Hunted;
		if (!player || !m_Group || !ai) return;
		if (!m_Mode) m_Mode = m_Spatial_Groups.HuntMode;
		int i;
		float c = (m_Spatial_Groups.EngageTimer / 2500) + 1;
		m_Group.ClearWaypoints();
		m_Group.m_BackTracking = false;
		switch (m_Mode)
    	{
			case 1: 
				m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 0, 3));
				player.GetTargetInformation().AddAI(ai, m_Spatial_Groups.EngageTimer);
				HuntCheck(m_Group, player, vector.Zero, 10000, 2);
			break;
			case 2: 
				//last known location 
				m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 10, 20));
			break;
			case 3: 
				//halt
			break;
			case 4: 
				//stay around spawnpos - extended onupdate
				m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 20, 40));
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(TrailingPos, 10000, true, m_Group, m_Position, 40);
			break;
			case 5: 
				//mix of 4 and 6 sorta
				m_Group.m_BackTracking = true;
				for (i = 0; i <= c; ++i) Spatial_PointGen(ai, m_Group, player);
			break;
			case 6: 
				//follows player
				m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 50, 55));
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(TrailingGroup, 10000, false, m_Group, player, vector.Zero, 10000, 80);
			break;
    	}
	}//Spatial_Movement(ai, group);
	void Spatial_PointGen(eAIBase ai, eAIGroup AiGroup, PlayerBase player)
    {
		int d = Math.RandomIntInclusive(0, 100);
		if (d < 16) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), 70, 120));
		if (d > 15 && d < 95) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 80, 200));
		if (d > 94) AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(ai.GetPosition(), 10, 20));
	}//Spatial_PointGen(ai, AiGroup, player);
	 //! https://feedback.bistudio.com/T173348 - readded null checks
	void TrailingGroup(eAIGroup AiGroup, PlayerBase player, vector pos = vector.Zero, int timer = 10000, int distance = 80)
    {
		//Print("Trailing trigger" + this);
		if (!player || !AiGroup) return;
		int min = distance; //80
		int max = distance * 1.5; //120
		int overdist = distance * 2; //160
		if (pos == player.GetPosition()) 
			AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), (min / 2), (min / 1.6))); //40 50

		if (!player || !AiGroup) return;
		eAIBase lead = eAIBase.Cast(AiGroup.GetLeader());
		if (!lead) return;
		if (vector.Distance(player.GetPosition(), lead.GetPosition()) > overdist)
    	{
			AiGroup.ClearWaypoints();
			AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), min, max));
		} else AiGroup.AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), min, max));
		
		if (!player) return;
		pos = player.GetPosition();
		if (!player || !AiGroup) return;
		if (m_Spatial_Groups.HuntMode == 1)
    	{
			if (player.GetTargetInformation().IsTargettedBy(lead)) return;
		}
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TrailingGroup, timer, false, AiGroup, player, pos, timer, distance);
	}//TrailingGroup(m_Group, player, pos, timer, distance);
	void HuntCheck(eAIGroup AiGroup, PlayerBase player, vector pos = vector.Zero, int timer = 10000, int distance = 20)
    {
			//actively hunts player
			eAIBase lead = eAIBase.Cast(AiGroup.GetLeader());
			if (!player || !AiGroup || !lead) return;
		if (!player.GetTargetInformation().IsTargettedBy(lead))
		{
			player.GetTargetInformation().AddAI(lead, m_Spatial_Groups.EngageTimer);
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(TrailingGroup, timer, false, AiGroup, player, pos, timer, distance);
		}
		else 
		{
			//! Update target found at time if already targeting
			player.GetTargetInformation().Update(m_Group);
		}
	}//HuntCheck(m_Group, player, pos, timer, distance);
	void TrailingPos(eAIGroup AiGroup, vector position, int distance = 20)
    {
		//Print("Trailing trigger" + this);
		if (!AiGroup) return;
		AiGroup.ClearWaypoints();
		vector waypoint = (ExpansionStatic.GetSurfacePosition(ExpansionMath.GetRandomPointInRing(position, distance, (distance * 1.5)));
		AiGroup.AddWaypoint(waypoint); 
	}//TrailingPos(pos, distance, m_Group);

	void CheckLocation(vector pos)
	{
		if (m_Group)
		{
			m_Group.ClearWaypoints();
			m_Group.AddWaypoint(ExpansionMath.GetRandomPointInRing(pos, 0, 6));
		}

	}//audio reasonable location

}
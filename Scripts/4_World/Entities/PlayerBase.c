//CF Dependancies here.

modded class PlayerBase
{
    bool Spatial_InZone;
    bool m_Zone_Safe;

    int Spatial_LocationHunt = 0;
    bool Spatial_InLocation;

    int Spatial_noisepresence = 0;

    int m_Spatial_Birthday;
    bool Spatial_UnlimitedReload;

    ref Spatial_Groups m_Spatial_Groups;
    ref Spatial_Players m_Spatial_Players;
    
    Spatial_Point m_spatial_point;
    Spatial_Notification spatial_notification;

    void PlayerBase()
    {
        GetRPCManager().AddRPC("DayZ-Expansion-AI-Dynamic", "Spatial_GetThreat", this, SingleplayerExecutionType.Both);
#ifdef SERVER
        if (!IsAI())
        {
            SpatialPlayerSettings().PullRef(m_Spatial_Players);   
            GetSpatialSettings().PullRef(m_Spatial_Groups);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Spatial_SetBirthday, 500, false);
        }
#else
    GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Spatial_NoisePresence, 1000, true);

#endif
    } //load file birthday

    override void EEKilled( Object killer )
    {
        #ifdef SERVER
        if (GetIdentity() && !IsAI())
        {
            if (!m_Spatial_Players)
                SpatialPlayerSettings().PullRef(m_Spatial_Players);

            SpatialPlayerSettings().Update_Player(GetIdentity().GetPlainId(), 0); 
        }
        #endif
		super.EEKilled(killer);
	} //set file birthday to 0

    void SetSpatialPoint(Spatial_Point point)
    {
        m_spatial_point = point;
    } //changed to class instead of individuals

    Spatial_Group GetSpatialGroup()
    {
        Spatial_Group group = new Spatial_Group(m_spatial_point.Spatial_MinCount, m_spatial_point.Spatial_MaxCount, 100, m_spatial_point.Spatial_ZoneLoadout, m_spatial_point.Spatial_Faction, m_spatial_point.Spatial_Name, m_spatial_point.Spatial_Lootable, m_spatial_point.Spatial_Chance, m_spatial_point.Spatial_MinAccuracy, m_spatial_point.Spatial_MaxAccuracy, m_spatial_point.Spatial_UnlimitedReload);
        return group;
    }

    void Spatial_SetInZone(bool in)
    {
        Spatial_InZone = in;
    }

    void Spatial_SetSafe(bool a)
    {
        m_Zone_Safe = a;
    }

    void Spatial_InLocation(bool a, int b)
    {
        Spatial_InLocation = a;
        Spatial_LocationHunt = b;
    }

    void Spatial_SetNotification(Spatial_Notification a)
    {
        spatial_notification = a;
    }

    Spatial_Notification Spatial_notification()
    {
        return spatial_notification;
    }
    
    int Spatial_HuntMode()
    {
        return m_spatial_point.Spatial_HuntMode;
    } // point huntmode

    int Spatial_LocationHunt()
    {
        return Spatial_LocationHunt;
    }   //location huntmode

    bool Spatial_CheckSafe()
    {
        return m_Zone_Safe;
    } // is in spatial safezone

    bool Spatial_CheckZone()
    {
        return Spatial_InZone;
    } //is in any spatial zone

    bool Spatial_IsInLocation()
    {
        return Spatial_InLocation;
    } //is in any location

    bool Spatial_HasGPSReceiver()
    {
        if (GetMapNavigationBehaviour())
        {
        int num = GetMapNavigationBehaviour().GetNavigationType();
        if (num > 1) return true;
        }

       GPSReceiver Spatial_HasRecevier = GPSReceiver.Cast(FindAttachmentBySlotName("WalkieTalkie"));
       if (Spatial_HasRecevier && Spatial_HasRecevier.IsTurnedOn()) return true;

        int item_count = GetInventory().GetCargo().GetItemCount();
		for (int i = 0; i < item_count; i++)
		{
			GPSReceiver item = GPSReceiver.Cast(GetInventory().GetCargo().GetItem(i));
            if (!item) continue;
            if (item) return item.IsTurnedOn();
		}

        return false;
    } //wardog's code
    
    void Spatial_SetBirthday()
    {
        if (!GetIdentity()) return;
        if (!m_Spatial_Players) SpatialPlayerSettings().PullRef(m_Spatial_Players);
        int birth = SpatialPlayerSettings().Check_Player(GetIdentity().GetPlainId());
        if (birth != 0)
        {
            m_Spatial_Birthday = birth;
        } 
        else 
        {
            m_Spatial_Birthday = CF_Date.Now().GetTimestamp();
            SpatialPlayerSettings().Update_Player(GetIdentity().GetPlainId(), m_Spatial_Birthday); 
        }
    } //pull ref, check file and get/set birthday, otherwise new and set.

    string Spatial_GetBirthday()
    {
        if (!m_Spatial_Birthday) Spatial_SetBirthday();
        CF_Date date = CF_Date.Epoch(m_Spatial_Birthday);
        return string.Format("%1 %2 %3 @ %4:%5", date.GetDay(), date.GetFullMonthString(), date.GetYear(), date.GetHours().ToStringLen(2), date.GetMinutes().ToStringLen(2));
    } //print date

    int m_Spatial_BirthdayDate()
    {
        if (!m_Spatial_Birthday)
            Spatial_SetBirthday();

        return m_Spatial_Birthday;
    } //get raw timestamp
    
    bool Spatial_CheckAge(int a)
    {
        a = a * 60;
        CF_Date date1 = CF_Date.Now();
        CF_Date date2 = CF_Date.Epoch(m_Spatial_Birthday + a);
        int hoursDiff, minutesDiff;
        date1.CalculateDifference(date2, hoursDiff, minutesDiff);

        if (GetSpatialSettings().Spatial_Debug())
        {
            CF_Date date3 = CF_Date.Epoch(m_Spatial_Birthday);
            float Spatial_Playtime = StatGet("playtime");
            SpatialDebugPrint("player: " + GetIdentity().GetName());
            SpatialDebugPrint("now: " + string.Format("%1 %2 %3 @ %4:%5", date1.GetDay(), date1.GetFullMonthString(), date1.GetYear(), date1.GetHours().ToStringLen(2), date1.GetMinutes().ToStringLen(2)));
            SpatialDebugPrint("Valid Time: " + string.Format("%1 %2 %3 @ %4:%5", date2.GetDay(), date2.GetFullMonthString(), date2.GetYear(), date2.GetHours().ToStringLen(2), date2.GetMinutes().ToStringLen(2)));
            SpatialDebugPrint("birthday: " + string.Format("%1 %2 %3 @ %4:%5", date3.GetDay(), date3.GetFullMonthString(), date3.GetYear(), date3.GetHours().ToStringLen(2), date3.GetMinutes().ToStringLen(2)));
            SpatialDebugPrint("Difference: " + string.Format("hours:%1 - Minutes:%2", hoursDiff, minutesDiff));
            SpatialDebugPrint("Native check: " + string.Format("Playtime:%1", Spatial_Playtime));
            SpatialDebugPrint("Difference calc: " + (a / 60));
        }

        if (hoursDiff > -1 && minutesDiff > -1)
            return true;

        return false;
    } //compare offset to birthday

    int Spatial_GetNoise()
    {
        return Spatial_noisepresence;
    }

    void SpatialDebugPrint(string msg)
    {
        if (GetSpatialSettings().Spatial_Debug())
            GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
    } //expansion debug print

    void Spatial_GetThreat(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) 
    {
        if (type != CallType.Server) return;

        Param1 < int > data;
        if (!ctx.Read(data)) return;
        if (sender == null) return;
        Spatial_noisepresence = data.param1;
    }

    void Spatial_NoisePresence()
    {
        if (this)
        {
            int threat = GetNoisePresenceInAI();
            GetRPCManager().SendRPC("DayZ-Expansion-AI-Dynamic", "Spatial_GetThreat", new Param1<int>(threat), true, NULL);   
        }
    }
};
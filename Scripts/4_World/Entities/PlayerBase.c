modded class PlayerBase
{
    bool Spatial_InZone = false;
    bool m_Zone_Safe = false;
    string m_Zone_Faction = "Shamans";
    string m_Zone_Loadout = "HumanLoadout.json";
    string m_Zone_Name = "Survivor";
    int Spatial_MaxCount = 4;
    int Spatial_MinCount = 0;
    int Spatial_HuntMode = 3;
    int Spatial_Lootable = 1;
    float Spatial_Chance = 0.5;
    int Spatial_LocationHunt = 10;
    int m_Spatial_Birthday;
    bool Spatial_UnlimitedReload = false;
    ref Spatial_Groups m_Spatial_Groups;
    ref Spatial_Players m_Spatial_Players;

    void PlayerBase(){
        if (GetGame().IsServer()) {
            if (!IsAI()) {
                SpatialPlayerSettings().PullRef(m_Spatial_Players);   
                GetSpatialSettings().PullRef(m_Spatial_Groups);
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Spatial_SetBirthday, 500, false);
            }  
        }
    } //load file birthday

    override void EEKilled( Object killer ) {
        if (GetGame().IsServer())
        {   
            if (!IsAI()) {
                if (!m_Spatial_Players) SpatialPlayerSettings().PullRef(m_Spatial_Players);
                SpatialPlayerSettings().Update_Player(GetIdentity().GetPlainId(), 0); 
            }
        }
		super.EEKilled(killer);
	} //set file birthday to 0

    void Spatial_SetData(string fac, string lod, int c, int d, int e, string f, int g, float h, bool i){
        m_Zone_Faction = fac;
        m_Zone_Loadout = lod;
        Spatial_MinCount = c;
        Spatial_MaxCount = d;
        Spatial_HuntMode = e;
        m_Zone_Name = f;
        Spatial_Lootable = g;
        Spatial_Chance = h;
        Spatial_UnlimitedReload = i;
    }

    string Spatial_Faction() {
        return m_Zone_Faction;
    }
    string Spatial_Loadout() {
        return m_Zone_Loadout;
    }
    string Spatial_Name() {
        return m_Zone_Name;
    }
    void Spatial_SetInZone(bool in){
        Spatial_InZone = in;
    }
    int Spatial_MinCount(){
        return Spatial_MinCount;
    }
    int Spatial_MaxCount(){
        return Spatial_MaxCount;
    }
    int Spatial_HuntMode(){
        return Spatial_HuntMode;
    }
    int Spatial_Lootable(){
        return Spatial_Lootable;
    }
    float Spatial_Chance(){
        return Spatial_Chance;
    }
    bool Spatial_UnlimitedReload(){
        return Spatial_UnlimitedReload;
    }
    void Spatial_SetSafe(bool a){
        m_Zone_Safe = a;
    }
    void Spatial_SetLocationHunt(int a){
        Spatial_LocationHunt = a;
    }
    int Spatial_LocationHunt(){
        return Spatial_LocationHunt;
    }
    bool Spatial_CheckSafe(){
        return m_Zone_Safe;
    }
    bool Spatial_CheckZone(){
        return Spatial_InZone;
    }

    bool Spatial_HasGPSReceiver(){
        if (GetMapNavigationBehaviour()) return (GetMapNavigationBehaviour().GetNavigationType() & EMapNavigationType.GPS | EMapNavigationType.ALL == 0);
        return false;
    } //wardog's code
    
    void Spatial_SetBirthday(){
        if (!GetIdentity()) return;
        if (!m_Spatial_Players) SpatialPlayerSettings().PullRef(m_Spatial_Players);
        int birth = SpatialPlayerSettings().Check_Player(GetIdentity().GetPlainId());
        if (birth != 0){
            m_Spatial_Birthday = birth;
        } else {
            m_Spatial_Birthday = CF_Date.Now(true).GetTimestamp();
            SpatialPlayerSettings().Update_Player(GetIdentity().GetPlainId(), m_Spatial_Birthday); 
        }
    } //pull ref, check file and get/set birthday, otherwise new and set.

    string Spatial_GetBirthday(){
        if (!m_Spatial_Birthday) Spatial_SetBirthday();
        CF_Date date = CF_Date.Epoch(m_Spatial_Birthday);
        return string.Format("%1 %2 %3 @ %4:%5", date.GetDay(), date.GetFullMonthString(), date.GetYear(), date.GetHours().ToStringLen(2), date.GetMinutes().ToStringLen(2));
    } //print date

    int m_Spatial_BirthdayDate() {
        if (!m_Spatial_Birthday) Spatial_SetBirthday();
        return m_Spatial_Birthday;
    } //get raw timestamp
    
    bool Spatial_CheckAge(int a){
        a = a * 60;
        CF_Date date1 = CF_Date.Now(true);
        CF_Date date2 = CF_Date.Epoch(m_Spatial_Birthday + a);
        CF_Date date3 = CF_Date.Epoch(m_Spatial_Birthday);
        int hoursDiff, minutesDiff;
        Spatial_CompareDates(date2, date1, hoursDiff, minutesDiff);

        SpatialDebugPrint("player: " + GetIdentity().GetName());
        SpatialDebugPrint("now: " + string.Format("%1 %2 %3 @ %4:%5", date1.GetDay(), date1.GetFullMonthString(), date1.GetYear(), date1.GetHours().ToStringLen(2), date1.GetMinutes().ToStringLen(2)));
        SpatialDebugPrint("Valid Time: " + string.Format("%1 %2 %3 @ %4:%5", date2.GetDay(), date2.GetFullMonthString(), date2.GetYear(), date2.GetHours().ToStringLen(2), date2.GetMinutes().ToStringLen(2)));
        SpatialDebugPrint("birthday: " + string.Format("%1 %2 %3 @ %4:%5", date3.GetDay(), date3.GetFullMonthString(), date3.GetYear(), date3.GetHours().ToStringLen(2), date3.GetMinutes().ToStringLen(2)));
        SpatialDebugPrint("Difference: " + string.Format("hours:%1 - Minutes:%2", hoursDiff, minutesDiff));
        SpatialDebugPrint("Difference calc: " + (a / 60));

        if (hoursDiff > -1) {
            if (minutesDiff > -1) {
                return true;
            }
        }
        return false;	
    } //compare offset to birthday

    void Spatial_CompareDates(CF_Date date1, CF_Date date2, out int hours, out int minutes) {
        int timestamp1 = date1.GetTimestamp();
        int timestamp2 = date2.GetTimestamp();
        int timestampDiff = timestamp2 - timestamp1;
        hours = timestampDiff / 3600;
        minutes = (timestampDiff % 3600) / 60;
    }  //compare function

    void SpatialDebugPrint(string msg) {
        if (m_Spatial_Groups.Spatial_MinTimer == 60000)
            GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
    } //expansion debug print

};

modded class PlayerBase
{
    bool Spatial_InZone = false;
    bool m_Zone_Safe = false;
    int Spatial_LocationHunt = 10;
    int m_Spatial_Birthday;
    bool Spatial_UnlimitedReload = false;
    ref Spatial_Groups m_Spatial_Groups;
    ref Spatial_Players m_Spatial_Players;
    Spatial_Point m_spatial_point;
    Spatial_Notification spatial_notification;

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

    void SetSpatialPoint(Spatial_Point point){
        m_spatial_point = point;
    } //changed to class instead of individuals

    Spatial_Group GetSpatialGroup() {
      Spatial_Group group = new Spatial_Group(m_spatial_point.Spatial_MinCount, m_spatial_point.Spatial_MaxCount, 100, m_spatial_point.Spatial_ZoneLoadout, m_spatial_point.Spatial_Faction, m_spatial_point.Spatial_Name, m_spatial_point.Spatial_Lootable, m_spatial_point.Spatial_Chance, m_spatial_point.Spatial_MinAccuracy, m_spatial_point.Spatial_MaxAccuracy, m_spatial_point.Spatial_UnlimitedReload);
        return group;
        }
    void Spatial_SetInZone(bool in){
        Spatial_InZone = in;
        }

    void Spatial_SetSafe(bool a){
        m_Zone_Safe = a;
        }
    void Spatial_SetLocationHunt(int a){
        Spatial_LocationHunt = a;
        }

    void Spatial_SetNotification(Spatial_Notification a){
        spatial_notification = a;
        }

    Spatial_Notification Spatial_notification(){
        return spatial_notification;
        }
    
    int Spatial_HuntMode(){
        return m_spatial_point.Spatial_HuntMode;
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
        if (GetSpatialSettings().Spatial_Debug())
            GetExpansionSettings().GetLog().PrintLog("[Spatial Debug] " + msg);
    } //expansion debug print
};

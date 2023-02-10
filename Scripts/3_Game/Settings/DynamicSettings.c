/*
10/2/2022
dynamic settings
tldr - learning
*/
class DynamicSettings
{
//declares
  private const static string EXP_DYNAMIC_FOLDER = "$profile:ExpansionMod\\AI\\Dynamic\\";
  private const static string EXP_AI_DYNAMIC_SETTINGS = EXP_DYNAMIC_FOLDER + "DynamicSettings.json";
  const int SZ_IN_SAFEZONE = 0x0001;
  int m_cur = 0;
  int m_Dynamic_Total = -1;
  bool Dynamic_Version = true;
  bool m_Dynamic_InVehicle;
  bool m_Dynamic_IsBleeding;
  bool m_Dynamic_IsRestrained;
  bool m_Dynamic_IsUnconscious;
  bool m_Dynamic_IsInSafeZone;
  bool m_Dynamic_TPSafeZone;
  bool m_Dynamic_InOwnTerritory;

 //refs
  ref Dynamic_Groups m_Dynamic_Groups;

//init
bool Init(){
    Load();
    if (m_Dynamic_Total < 0) {
      Print("Dynamic AI Config Error - Disabled");
      loggerPrint("No Dynamic Groups valid."); //Sadface box
      return false;
    } else {
      if (Dynamic_Version == true) {
        return Dynamic_Version;
      }
    }
    return false;
}

//load ref to use location
void PullRef(out Dynamic_Groups Data){
    if (!m_Dynamic_Groups) Load();
    Data = m_Dynamic_Groups;
}

//load from file/data checks
void Load() 
{
    //loggerPrint("Dynamic Config Loader Start");
    if (!FileExist(EXP_AI_DYNAMIC_SETTINGS)) {
      if (!FileExist(EXP_DYNAMIC_FOLDER))
        MakeDirectory(EXP_DYNAMIC_FOLDER);
      loggerPrint("WARNING: Couldn't find config file !");
      loggerPrint("Dynamic config will be located in: " + EXP_DYNAMIC_FOLDER);
      DefaultDynamicSettings(m_Dynamic_Groups);
      JsonFileLoader < Dynamic_Groups > .JsonSaveFile(EXP_AI_DYNAMIC_SETTINGS, m_Dynamic_Groups);
    } else {
      m_Dynamic_Groups = new Dynamic_Groups();
      JsonFileLoader < Dynamic_Groups > .JsonLoadFile(EXP_AI_DYNAMIC_SETTINGS, m_Dynamic_Groups);
    }
    if (m_Dynamic_Groups.Version != 12) { // dont like this. change it.
      loggerPrint("Settings File Out of date. Please delete and restart server.");
      Dynamic_Version = false;
      return;
    }
    if (m_Dynamic_Groups.MaxAI < 1) {
      loggerPrint("MaxAI set under 1, disabling Dynamic AI.");
      Dynamic_Version = false;
      return;
    } 
    if (m_Dynamic_Groups.PlayerChecks < 0) {
      loggerPrint("PlayerChecks set under 0, disabling Dynamic AI.");
      Dynamic_Version = false;
      return;
    }

    if (m_Dynamic_Groups.Dynamic_MinTimer < 300000) { // global minimum time of 5 mins 
      loggerPrint("Timer Set too low. Defaulting to 5 minutes");
      m_Dynamic_Groups.Dynamic_MinTimer = 300000;
    }
    if (m_Dynamic_Groups.Dynamic_MaxTimer < m_Dynamic_Groups.Dynamic_MinTimer) { 
      loggerPrint("Max Timer set lower than min timer, setting to the same.");
      m_Dynamic_Groups.Dynamic_MaxTimer = m_Dynamic_Groups.Dynamic_MinTimer;
    }
    if (m_Dynamic_Groups.MinDistance < 120) {
      loggerPrint("Minimum Distance too low. setting to 120m");
      m_Dynamic_Groups.MinDistance = 120;
    }
    if (m_Dynamic_Groups.MaxDistance < m_Dynamic_Groups.MinDistance) {
      loggerPrint("Max distance under min distance. setting +20m");
      m_Dynamic_Groups.MaxDistance = m_Dynamic_Groups.MinDistance + 20;
    }
    if (m_Dynamic_Groups.HuntMode < 0 && m_Dynamic_Groups.HuntMode > 3) {
      loggerPrint("HuntMode setting wrong. setting to default.");
      m_Dynamic_Groups.HuntMode = 1;
    }
    if (m_Dynamic_Groups.Points_Enabled == 0) {
    } else if (m_Dynamic_Groups.Points_Enabled == 1) {
      foreach(Dynamic_Point point: m_Dynamic_Groups.Point) {
        if (point.Dynamic_Radius < 0) {
          loggerPrint("Radius on group incorrect, setting to 100m");
          point.Dynamic_Radius = 100;
        }
        if (!FileExist("$profile:ExpansionMod\\Loadouts\\" + point.Dynamic_ZoneLoadout)) {
          loggerPrint("Loadout Not Found: " + point.Dynamic_ZoneLoadout);
          point.Dynamic_ZoneLoadout = "HumanLoadout.json";
        }
        if (point.Dynamic_Safe != 1 && point.Dynamic_Safe != 0) {
          loggerPrint("Zone safe value incorrect. setting to safe");
          point.Dynamic_Safe = 1;
        }
        eAIFaction faction = eAIFaction.Create(point.Dynamic_Faction);
			  if (!faction)
			  {
        loggerPrint("Faction not correct: " + point.Dynamic_Faction);
				point.Dynamic_Faction = "Raiders";
			  }
      }
    } else {
      loggerPrint("error, disabling points");
      m_Dynamic_Groups.Points_Enabled = 0;
    }
    if (m_Dynamic_Groups.EngageTimer < 300000) { // global minimum time of 5 mins
      loggerPrint("Minimum engagement too low. setting to 5m");
      m_Dynamic_Groups.EngageTimer = 300000;
    }

    if (m_Dynamic_Groups.CleanupTimer < m_Dynamic_Groups.EngageTimer) {
      loggerPrint("Cleanup timer under engage timer. setting +1m");
      m_Dynamic_Groups.CleanupTimer = m_Dynamic_Groups.EngageTimer + 60000;
    }

    if (m_Dynamic_Groups.MessageType < 0 && m_Dynamic_Groups.MessageType > 4) {
      loggerPrint("Message type error. disabling.");
      m_Dynamic_Groups.MessageType = 0;
    }

    if (!m_Dynamic_Groups.MessageTitle) {
      loggerPrint("Notification title error. setting default.");
      m_Dynamic_Groups.MessageTitle = "Dynamic AI";
    } else if (m_Dynamic_Groups.MessageTitle == " ") {
      loggerPrint("Notification title error. setting default.");
      m_Dynamic_Groups.MessageTitle = "Dynamic AI";
    }
    if (!m_Dynamic_Groups.MessageText) {
      loggerPrint("Message text error. setting default.");
      m_Dynamic_Groups.MessageText = "AI Spotted in the Area. Be Careful.";
    } else if (m_Dynamic_Groups.MessageText == " ") {
      loggerPrint("Message text error. setting default.");
      m_Dynamic_Groups.MessageText = "AI Spotted in the Area. Be Careful.";
    }
    if (m_Dynamic_Groups.Lootable < 0 && m_Dynamic_Groups.Lootable > 4){
      m_Dynamic_Groups.Lootable = 0;
      loggerPrint("lootable check error. setting default to no.");
    }
    if (m_Dynamic_Groups.Dynamic_InVehicle != true && m_Dynamic_Groups.Dynamic_InVehicle != false) {
      loggerPrint("ignore vehicle check error - default off");
      m_Dynamic_Groups.Dynamic_InVehicle = false;
    }
    m_Dynamic_InVehicle = m_Dynamic_Groups.Dynamic_InVehicle;
    if (m_Dynamic_Groups.Dynamic_IsBleeding != true && m_Dynamic_Groups.Dynamic_IsBleeding != false) {
      loggerPrint("ignore bleeding check error - default off");
      m_Dynamic_Groups.Dynamic_IsBleeding = false;
    }
    m_Dynamic_IsBleeding = m_Dynamic_Groups.Dynamic_IsBleeding;
    if (m_Dynamic_Groups.Dynamic_IsRestrained != true && m_Dynamic_Groups.Dynamic_IsRestrained != false) {
      loggerPrint("ignore restrained check error - default off");
      m_Dynamic_Groups.Dynamic_IsRestrained = false;
    }
    m_Dynamic_IsRestrained = m_Dynamic_Groups.Dynamic_IsRestrained;
    if (m_Dynamic_Groups.Dynamic_IsUnconscious != true && m_Dynamic_Groups.Dynamic_IsUnconscious != false) {
      loggerPrint("ignore Unconscious check error - default off");
      m_Dynamic_Groups.Dynamic_IsUnconscious = false;
    }
    m_Dynamic_IsUnconscious = m_Dynamic_Groups.Dynamic_IsUnconscious;
    if (m_Dynamic_Groups.Dynamic_IsInSafeZone != true && m_Dynamic_Groups.Dynamic_IsInSafeZone != false) {
      loggerPrint("ignore expansion SafeZone check error - default off");
      m_Dynamic_Groups.Dynamic_IsInSafeZone = false;
    }
    m_Dynamic_IsInSafeZone = m_Dynamic_Groups.Dynamic_IsInSafeZone;
    if (m_Dynamic_Groups.Dynamic_TPSafeZone != true && m_Dynamic_Groups.Dynamic_TPSafeZone != false) {
      loggerPrint("ignore traderplus SafeZone check error - default off");
      m_Dynamic_Groups.Dynamic_TPSafeZone = false;
    }
    m_Dynamic_TPSafeZone = m_Dynamic_Groups.Dynamic_TPSafeZone;
    if (m_Dynamic_Groups.Dynamic_InOwnTerritory != true && m_Dynamic_Groups.Dynamic_InOwnTerritory != false) {
      loggerPrint("ignore expansion own territory check error - default off");
      m_Dynamic_Groups.Dynamic_InOwnTerritory = false;
    }
    m_Dynamic_InOwnTerritory = m_Dynamic_Groups.Dynamic_InOwnTerritory;
    m_Dynamic_Total = 0;
    foreach(Dynamic_Group group: m_Dynamic_Groups.Group) {

      if (!group.Dynamic_MinCount) {
        group.Dynamic_MinCount = 0;
        loggerPrint("Dynamic_MinCount error, setting to 0");
        continue;
      }

      if (group.Dynamic_MinCount > group.Dynamic_MaxCount) {
        loggerPrint("Minimum cant be more than maximum: " + group.Dynamic_MinCount);
        continue;
      }
      if (group.Dynamic_MaxCount < 1) {
        loggerPrint("Maximum count cant be less than 0: " + group.Dynamic_MaxCount);
        continue;
      }
      if (!FileExist("$profile:ExpansionMod\\Loadouts\\" + group.Dynamic_Loadout)) {
        loggerPrint("Loadout Not Found: " + group.Dynamic_Loadout);
        continue;
      }
        eAIFaction faction2 = eAIFaction.Create(group.Dynamic_Faction);
			  if (!faction2)
			  {
        loggerPrint("Faction not correct: " + group.Dynamic_Faction);
				group.Dynamic_Faction = "Raiders";
			  }
      m_Dynamic_Total += 1;
    }
    //loggerPrint("Dynamic Config Loader End");
  }


  //generate default array data
  void DefaultDynamicSettings(out Dynamic_Groups Data) {
    Data = new Dynamic_Groups();
    Data.Group.Insert(new Dynamic_Group(0, 3, "WestLoadout.json", "Shamans"));
    Data.Group.Insert(new Dynamic_Group(0, 3, "HumanLoadout.json", "Shamans"));
    Data.Group.Insert(new Dynamic_Group(0, 3, "EastLoadout.json", "Shamans"));

    Data.Point.Insert(new Dynamic_Point(1, 50, "HumanLoadout.json", 0, 4, "Shamans", "0.0 0.0 0.0"));
    Data.Point.Insert(new Dynamic_Point(0, 100, "HumanLoadout.json", 0, 5, "West", "0.0 0.0 0.0"));
    Data.Point.Insert(new Dynamic_Point(0, 150, "HumanLoadout.json", 3, 4, "East", "0.0 0.0 0.0"));
  }

  //expansion logging (Dynamic AI prefex)
  void loggerPrint(string msg) {
    if (GetExpansionSettings().GetLog().AIGeneral)
      GetExpansionSettings().GetLog().PrintLog("[Dynamic Settings] " + msg);
  }


//returns
    int Dynamic_Total(){
    return m_Dynamic_Total;
  }

  bool Dynamic_InVehicle(){
    return m_Dynamic_InVehicle;
  }

  bool Dynamic_IsBleeding(){
    return m_Dynamic_IsBleeding;
  }

  bool Dynamic_IsRestrained(){
    return m_Dynamic_IsRestrained;
  }

  bool Dynamic_IsUnconscious(){
    return m_Dynamic_IsUnconscious;
  }

  bool Dynamic_IsInSafeZone(){
    return m_Dynamic_IsInSafeZone;
  }

  bool Dynamic_TPSafeZone(){
    return m_Dynamic_TPSafeZone;
  }

  bool Dynamic_InOwnTerritory(){
    return m_Dynamic_InOwnTerritory;
  }

  TStringArray Dynamic_WhiteList(){
    if (!m_Dynamic_Groups) Load();
     return m_Dynamic_Groups.LootWhitelist;
  }

}
//json data
class Dynamic_Groups {
  int Version = 12;
  int Dynamic_MinTimer = 1200000;
  int Dynamic_MaxTimer = 1200000;
  int MinDistance = 140;
  int MaxDistance = 220;
  int HuntMode = 1;
  int Points_Enabled = 0;
  int EngageTimer = 300000;
  int CleanupTimer = 360000;
  int PlayerChecks = 0;
  int MaxAI = 20;
  int MessageType = 1;
  string MessageTitle = "Dynamic AI";
  string MessageText = "AI Spotted in the Area. Be Careful.";
  int Lootable = 0;
  ref TStringArray LootWhitelist = {"Expansion_AWM","AKM","M4A1"};
  bool Dynamic_InVehicle = false;
  bool Dynamic_IsBleeding = false;
  bool Dynamic_IsRestrained = false;
  bool Dynamic_IsUnconscious = false;
  bool Dynamic_IsInSafeZone = false;
  bool Dynamic_TPSafeZone = false;
  bool Dynamic_InOwnTerritory = false;
  ref array < ref Dynamic_Group > Group;
  ref array < ref Dynamic_Point > Point;
  void Dynamic_Groups() {
    Group = new array < ref Dynamic_Group > ;
    Point = new array < ref Dynamic_Point > ;
  }
}

class Dynamic_Group {
  int Dynamic_MinCount;
  int Dynamic_MaxCount;
  string Dynamic_Loadout;
  string Dynamic_Faction;
  void Dynamic_Group(int a, int b, string c, string d) {
    Dynamic_MinCount = a;
    Dynamic_MaxCount = b;
    Dynamic_Loadout = c;
    Dynamic_Faction = d;
  }
}

class Dynamic_Point {
  float Dynamic_Radius;
  string Dynamic_ZoneLoadout;
  string Dynamic_Faction;
  int Dynamic_MinCount;
  int Dynamic_MaxCount;
  bool Dynamic_Safe;
  vector Dynamic_Position;
  void Dynamic_Point(bool a, float b, string c, int d, int e, string f, vector g) {
    Dynamic_Safe = a;
    Dynamic_Radius = b;
    Dynamic_ZoneLoadout = c;
    Dynamic_MinCount = d;
    Dynamic_MaxCount = e;
    Dynamic_Faction = f;
    Dynamic_Position = g;
  }
};


static ref DynamicSettings g_DynamicSettings;

static DynamicSettings GetDynamicSettings()
{
	if ( g_DynamicSettings == NULL )
	{
		g_DynamicSettings = new DynamicSettings();
		g_DynamicSettings.Load();

	}

	return g_DynamicSettings;
}
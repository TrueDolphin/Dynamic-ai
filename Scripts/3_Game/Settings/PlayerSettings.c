class SpatialPlayerSettings {
  //Const
  private const static string EXP_SPATIAL_FOLDER = "$profile:ExpansionMod\\AI\\Spatial\\";
  private const static string EXP_AI_SPATIAL_SETTINGS = EXP_SPATIAL_FOLDER + "SpatialPlayerSettings.json";

  //refs
  ref Spatial_Players m_Spatial_Players;

  //load ref to use location
  void PullRef(out Spatial_Players Data) {
    if (!m_Spatial_Players) Load();
    Data = m_Spatial_Players;
    }

  //load from file/data checks
  void Load() {
    if (!FileExist(EXP_AI_SPATIAL_SETTINGS)) {
      if (!FileExist(EXP_SPATIAL_FOLDER))
        MakeDirectory(EXP_SPATIAL_FOLDER);
      loggerPrint("WARNING: Couldn't find config file !");
      loggerPrint("player config will be located in: " + EXP_SPATIAL_FOLDER);
      DefaultSpatialPlayerSettings(m_Spatial_Players);
      JsonFileLoader < Spatial_Players > .JsonSaveFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Players);
    } else {
      m_Spatial_Players = new Spatial_Players();
      JsonFileLoader < Spatial_Players > .JsonLoadFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Players);
    }
    }

  void Save() {
    if (m_Spatial_Players) {
      if (!FileExist(EXP_SPATIAL_FOLDER)) MakeDirectory(EXP_SPATIAL_FOLDER);
      JsonFileLoader < Spatial_Players > .JsonSaveFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Players);
    }
    }

  void New_Player(string UID, int date) {
    if (!m_Spatial_Players) Load();
    m_Spatial_Players.Group.Insert(new Spatial_Player(UID, date));
    Save();
    }

  void Update_Player(string UID, int date){
    bool found = false;
    if (!m_Spatial_Players) Load();
    foreach (Spatial_Player s_Player : m_Spatial_Players.Group) {
      if (s_Player.UID == UID){
        s_Player.Player_Birthday = date;
        found = true;
      }
    }
    if (!found){
      New_Player(UID, date);
    } else {
      Save();
    }

    }

  int Check_Player(string UID){
    if (!m_Spatial_Players) Load();
    foreach (Spatial_Player s_Player : m_Spatial_Players.Group) {
      if (s_Player.UID == UID){
        return s_Player.Player_Birthday;
      }
    }
    return 0;
    }


  //generate default array data
  void DefaultSpatialPlayerSettings(out Spatial_Players Data) {
    Data = new Spatial_Players();

    Data.Group.Insert(new Spatial_Player("76562158225858681", 0));
    Data.Group.Insert(new Spatial_Player("76562158225858682", 0));
    Data.Group.Insert(new Spatial_Player("76562158225858683", 0));
    Data.Group.Insert(new Spatial_Player("76561198019858686", 0));

    }

  void loggerPrint(string msg) {
      GetExpansionSettings().GetLog().PrintLog("[Spatial Player Settings] " + msg);
    }
}

//json data
class Spatial_Players {
  int Version = 1;
  ref array < ref Spatial_Player > Group;

  void Spatial_Players() {
    Group = new array < ref Spatial_Player > ;
  }
  }

class Spatial_Player {
  string UID;
  int Player_Birthday;

  void Spatial_Player(string bod, int date) {
    UID = bod;
    Player_Birthday = date;
  }
  }
  
static ref SpatialPlayerSettings g_SpatialPlayerSettings;
static SpatialPlayerSettings GetSpatialPlayerSettings() {
    g_SpatialPlayerSettings = new SpatialPlayerSettings();
    g_SpatialPlayerSettings.Load();
  return g_SpatialPlayerSettings;
  }
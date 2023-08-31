class Spatial_NotificationSettings {
  //declares
  private const static string EXP_SPATIAL_FOLDER = "$profile:ExpansionMod\\AI\\Spatial\\";
  private const static string EXP_AI_SPATIAL_SETTINGS = EXP_SPATIAL_FOLDER + "Notificationsettings.json";

  //refs
  ref Spatial_Notifications m_Spatial_Notifications;

  //load ref to use location
  void PullRef(out Spatial_Notifications Data) {
    if (!m_Spatial_Notifications) Load();
    Data = m_Spatial_Notifications;
    }

  //load from file/data checks
  void Load() {
    if (!FileExist(EXP_AI_SPATIAL_SETTINGS)) {
      if (!FileExist(EXP_SPATIAL_FOLDER))
        MakeDirectory(EXP_SPATIAL_FOLDER);
      loggerPrint("WARNING: Couldn't find Notification config");
      loggerPrint("generic Notification config will be located in: " + EXP_SPATIAL_FOLDER);
      DefaultSpatial_Notificationsettings(m_Spatial_Notifications);
      JsonFileLoader < Spatial_Notifications > .JsonSaveFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Notifications);
    } else {
      m_Spatial_Notifications = new Spatial_Notifications();
      JsonFileLoader < Spatial_Notifications > .JsonLoadFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Notifications);
    }
    bool datacheck = false;
    foreach(Spatial_Notification notification: m_Spatial_Notifications.notification) {
      if (!notification.Spatial_Name || notification.Spatial_Name == "") {
        notification.Spatial_Name = "null";
        datacheck = true;
        };
      if (!notification.MessageType) {
        notification.MessageType = 0;
        datacheck = true;
        };
      if (!notification.MessageTitle || notification.MessageTitle == "") {
        notification.MessageTitle = "null";
        datacheck = true;
        };
      if (!notification.MessageText) {
        notification.MessageText = {"null"};
        datacheck = true;
        };
    if (datacheck) JsonFileLoader < Spatial_Notifications > .JsonSaveFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Notifications);
    }
  }

  //generate default array data
  void DefaultSpatial_Notificationsettings(out Spatial_Notifications Data) {
    Data = new Spatial_Notifications();

    Data.notification.Insert(new Spatial_Notification( "East", 1, "East AI", {"Enemies nearby", "AI Detected: East", "Example 3"}));
    Data.notification.Insert(new Spatial_Notification( "West", 3, "West AI", {"Enemies nearby", "AI Detected: West"}));
    Data.notification.Insert(new Spatial_Notification( "Civilian", 5, "Civilian AI", {"Civilians detected", "Friendlies in the area"}));
    Data.notification.Insert(new Spatial_Notification( "Name", 0, "Disabled", {"Disabled"}));

    }

  void loggerPrint(string msg) {
      GetExpansionSettings().GetLog().PrintLog("[Spatial Notification Settings] " + msg);
    }
}

//json data
class Spatial_Notifications {
  int Version = 1;
  ref array < ref Spatial_Notification > notification;

  void Spatial_Notifications() {
    notification = new array < ref Spatial_Notification > ;
    }
  }

class Spatial_Notification {

  string Spatial_Name;
  int MessageType;
  string MessageTitle;
  ref TStringArray MessageText;

  void Spatial_Notification( string a, int b, string c, TStringArray d) {
  Spatial_Name = a;
  MessageType = b;
  MessageTitle = c;
  MessageText = d;
  }
  }


static ref Spatial_NotificationSettings g_Spatial_Notificationsettings;
static Spatial_NotificationSettings GetSpatial_Notificationsettings() {
    g_Spatial_Notificationsettings = new Spatial_NotificationSettings();
    g_Spatial_Notificationsettings.Load();
  return g_Spatial_Notificationsettings;
  }

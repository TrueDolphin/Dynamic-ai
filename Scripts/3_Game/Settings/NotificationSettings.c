class Spatial_NotificationSettings
{
  //declares
  private const static string EXP_SPATIAL_FOLDER = "$profile:ExpansionMod\\AI\\Spatial\\";
  private const static string EXP_AI_SPATIAL_SETTINGS = EXP_SPATIAL_FOLDER + "Notificationsettings.json";

  //refs
  ref Spatial_Notifications m_Spatial_Notifications;

  //load ref to use location
  void PullRef(out Spatial_Notifications Data)
  {
    if (!m_Spatial_Notifications) Load();
    Data = m_Spatial_Notifications;
  }

  //load from file/data checks
  void Load()
  {
    if (!FileExist(EXP_AI_SPATIAL_SETTINGS))
    {
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
    if (m_Spatial_Notifications.Version == 1)
    {
      m_Spatial_Notifications.Version = 2;
      datacheck = true;
    }
    foreach(Spatial_Notification notification: m_Spatial_Notifications.notification)
    {
      if (!notification.Spatial_Name || notification.Spatial_Name == "")
      {
        notification.Spatial_Name = "null";
        datacheck = true;
        };

      if (!notification.StartTime)
      {
        notification.StartTime = 0.0;
        datacheck = true;
      }

      if (!notification.StopTime)
      {
        notification.StopTime = 24.0;
        datacheck = true;
      }

      if (!notification.AgeTime)
      {
        notification.AgeTime = 1.30;
        datacheck = true;
      } 

      if (notification.StartTime > 24.0 || notification.StartTime < 0.0)
      {
        Math.Clamp(notification.StartTime, 0.0, 24.0);
        datacheck = true;
      }

      if (notification.StopTime > 24.0 || notification.StopTime < 0.0)
      {
        Math.Clamp(notification.StopTime, 0.0, 24.0);
        datacheck = true;
      }

      if (!notification.MessageType)
      {
        notification.MessageType = 0;
        datacheck = true;
        };
      if (!notification.MessageTitle || notification.MessageTitle == "")
      {
        notification.MessageTitle = "null";
        datacheck = true;
        };
      if (!notification.MessageText)
      {
        notification.MessageText = {"null"};
        datacheck = true;
        };
    if (datacheck) JsonFileLoader < Spatial_Notifications > .JsonSaveFile(EXP_AI_SPATIAL_SETTINGS, m_Spatial_Notifications);
    }
  }

  //generate default array data
  void DefaultSpatial_Notificationsettings(out Spatial_Notifications Data)
  {
    Data = new Spatial_Notifications();

    Data.notification.Insert(new Spatial_Notification( "East", 0.0, 24.0, 1.5, 1, "East AI", {"Enemies nearby", "AI Detected: East", "Example 3"}));
    Data.notification.Insert(new Spatial_Notification( "West", 0.0, 24.0, 2.5, 3, "West AI", {"Enemies nearby", "AI Detected: West"}));
    Data.notification.Insert(new Spatial_Notification( "Guard", 0.0, 24.0, 4.5, 5, "Guard AI", {"Guards detected", "Friendlies in the area, guns down."}));
    Data.notification.Insert(new Spatial_Notification( "Disabled", 0.0, 0.0, 10.0, 0, "Disabled", {"Disabled"}));

  }

  void loggerPrint(string msg)
  {
    GetExpansionSettings().GetLog().PrintLog("[Spatial Notification Settings] " + msg);
  }
}

//json data
class Spatial_Notifications
{
  int Version = 2;
  ref array < ref Spatial_Notification > notification;

  void Spatial_Notifications()
  {
    notification = new array < ref Spatial_Notification > ;
  }
}

class Spatial_Notification
{
  string Spatial_Name;
  float StartTime, StopTime, AgeTime;
  int MessageType;
  string MessageTitle;
  ref TStringArray MessageText;

  void Spatial_Notification( string a, float b, float c, float d, int e, string f, TStringArray g)
  {
    Spatial_Name = a;
    StartTime = b;
    StopTime = c;
    AgeTime = d;
    MessageType = e;
    MessageTitle = f;
    MessageText = g;
  }
}


static ref Spatial_NotificationSettings g_Spatial_Notificationsettings;
static Spatial_NotificationSettings GetSpatial_Notificationsettings()
{
  g_Spatial_Notificationsettings = new Spatial_NotificationSettings();
  g_Spatial_Notificationsettings.Load();
  return g_Spatial_Notificationsettings;
}

/*
name:TrueDolphin
date:4/10/2023
spatial ai spawns

added restricted time spawns.

*/
modded class MissionServer
{
  const ref SpatialAI Spatialai = new SpatialAI();
  ref Spatial_Groups m_Spatial_Groups;

  void MissionServer()
  {
    if (!m_Spatial_Groups)
    {
      if (GetSpatialSettings().Init() == true)
      {
        Spatialai.Spatial_Init();
        GetSpatialSettings().PullRef(m_Spatial_Groups);
        Spatialai.SpatialLoggerPrint("Spatial AI Enabled");
        if (GetSpatialSettings().Spatial_Debug()) Spatialai.SpatialLoggerPrint("Spatial Debug Mode on");
        Spatialai.InitSpatialTriggers();
        SpatialTimer();
      }
    }
  } //constructor
  void SpatialTimer()
  {
    int m_Spatial = Math.RandomIntInclusive(m_Spatial_Groups.Spatial_MinTimer, m_Spatial_Groups.Spatial_MaxTimer);
    if (GetCEApi())
    {
      if (m_Spatial_Groups.ActiveHoursEnabled)
      {
        float Spatial_daytime = GetGame().GetDayTime();  
        if (Spatial_daytime >= m_Spatial_Groups.ActiveStartTime && Spatial_daytime <= m_Spatial_Groups.ActiveStopTime)
        {
          Spatialai.SpatialDebugPrint("Spatial::Stack - Start");
          Spatialai.Spatial_Check(m_Players);
          Spatialai.SpatialDebugPrint("Spatial::Stack - End");
          Spatialai.SpatialLoggerPrint("Next valid check in: " + m_Spatial + "ms");
        }
        else
        {
          Spatialai.SpatialLoggerPrint("Next valid check in: " + m_Spatial + "ms");
        }
      } 
      else
      {
        Spatialai.SpatialDebugPrint("Spatial::Stack - Start");
        Spatialai.Spatial_Check(m_Players);
        Spatialai.SpatialDebugPrint("Spatial::Stack - End");
        Spatialai.SpatialLoggerPrint("Next valid check in: " + m_Spatial + "ms");
      }
    }
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpatialTimer, m_Spatial, false);
  } //timer call for varied check loops #refactored by LieutenantMaster
};
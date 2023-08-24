/*
name:TrueDolphin
date:7/8/2023
spatial ai spawns

Moved a bunch to its own class.

*/
modded class MissionServer {
  const ref SpatialAI Spatialai = new SpatialAI();
  ref Spatial_Groups m_Spatial_Groups;

  void MissionServer() {
    if (!m_Spatial_Groups) {
      if (GetSpatialSettings().Init() == true) {
        Spatialai.Spatial_Init();
        GetSpatialSettings().PullRef(m_Spatial_Groups);
        Spatialai.SpatialLoggerPrint("Spatial AI Enabled");
        if (GetSpatialSettings().Spatial_Debug()) Spatialai.SpatialLoggerPrint("Spatial Debug Mode on");
        Spatialai.InitSpatialTriggers();
        SpatialTimer();
      }
    }
  } //constructor
  void SpatialTimer() {
    Spatialai.SpatialDebugPrint("Spatial::Stack - Start");
    Spatialai.Spatial_Check(m_Players);
    Spatialai.SpatialDebugPrint("Spatial::Stack - End");
    int m_cor = Math.RandomIntInclusive(m_Spatial_Groups.Spatial_MinTimer, m_Spatial_Groups.Spatial_MaxTimer);
    Spatialai.SpatialLoggerPrint("Next valid check in: " + m_cor + "ms");
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpatialTimer, m_cor, false);
  } //timer call for varied check loops #refactored by LieutenantMaster
};
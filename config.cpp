#define _ARMA_

class CfgPatches
{
	class DayZ_Expansion_AI_Dynamic_scripts
	{
		requiredAddons[] = {"DayZExpansion_AI_Scripts", "EXP_AI_Dynamic_Define"};
	};
};
class CfgMods
{
	class DayZ_Expansion_AI_Dynamic
	{
		action = "";
		hideName = 0;
		hidePicture = 0;
		name = "AI Additional Scripts";
		credits = "DayZ Expansion and dolphin";
		author = "Dolphin";
		authorID = "";
		version = "0.1";
		extra = 0;
		type = "servermod";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"dolphin/DayZ-Expansion-AI-Dynamic/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"dolphin/DayZ-Expansion-AI-Dynamic/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"SafeZone/scripts/Common","dolphin/DayZ-Expansion-AI-Dynamic/Scripts/5_Mission"};
			};
		};
	};
};

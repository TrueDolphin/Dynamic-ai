#define _ARMA_

class CfgPatches
{
	class DayZ_Expansion_AI_Dynamic_scripts
	{
		//units[] = {"Storage_Band"};
		requiredAddons[] = {"DZ_Data","DZ_Scripts","DayZExpansion_AI_Scripts"};
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
		dependencies[] = {"World","Mission"};
		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"DayZExpansion/AI/Scripts/Common","DayZExpansion/AI/Scripts/4_World","dolphin/DayZ-Expansion-AI-Dynamic/World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"SafeZone/scripts/Common","DayZExpansion/AI/Scripts/Common","DayZExpansion/AI/Scripts/5_Mission","DayZExpansion/SpawnSelection/Scripts/5_Mission","dolphin/DayZ-Expansion-AI-Dynamic/Mission"};
			};
		};
	};
};
/*
class CfgVehicles
{
	class Armband_Black;
	class Dynamic_Admin_Band: Armband_Black
	{
		scope = 2;
		displayName = "Admin Band";
		descriptionShort = "Extra Storage.";
		weight = 1;
		storageCategory = 1;
		itemSize[] = {2,2};
		itemsCargoSize[] = {10,15};
		attachments[] = {"Truck_01_WoodenCrate1","Truck_01_WoodenCrate2","Truck_01_WoodenCrate3","Truck_01_WoodenCrate4","Shoulder"};
	};
};
*/
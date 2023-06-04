modded class PlayerBase
{
bool Spatial_InZone = false;
bool Zone_Safe = false;
string Zone_Faction = "Shamans";
string Zone_Loadout = "HumanLoadout.json";
string Zone_Name = "Survivor";
int Spatial_MaxCount = 4;
int Spatial_MinCount = 0;
int Spatial_HuntMode = 3;

void Spatial_SetData(string fac, string lod, int c, int d, int e, string f){
Zone_Faction = fac;
Zone_Loadout = lod;
Spatial_MinCount = c;
Spatial_MaxCount = d;
Spatial_HuntMode = e;
Zone_Name = f;
}

string Spatial_Faction() {
    return Zone_Faction;
}

string Spatial_Loadout() {
    return Zone_Loadout;
}

string Spatial_Name() {
    return Zone_Name;
}

void SetInZone(bool in){
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

void SetSafe(bool a){
    Zone_Safe = a;
}

bool CheckSafe(){
    return Zone_Safe;
}

bool CheckZone(){
    return Spatial_InZone;
}

};

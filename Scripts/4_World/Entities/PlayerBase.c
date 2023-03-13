modded class PlayerBase
{
bool Dynamic_InZone = false;
bool Zone_Safe = false;
string Zone_Faction = "Shamans";
string Zone_Loadout = "HumanLoadout.json";
int Dynamic_MaxCount = 4;
int Dynamic_MinCount = 0;
int Dynamic_HuntMode = 3;

void Dynamic_SetData(string fac, string lod, int c, int d, int e){
Zone_Faction = fac;
Zone_Loadout = lod;
Dynamic_MinCount = c;
Dynamic_MaxCount = d;
Dynamic_HuntMode = e;
}

string Dynamic_Faction() {
    return Zone_Faction;
}

string Dynamic_Loadout() {
    return Zone_Loadout;
}
void SetInZone(bool in){
    Dynamic_InZone = in;
}

int Dynamic_MinCount(){
    return Dynamic_MinCount;
}

int Dynamic_MaxCount(){
    return Dynamic_MaxCount;
}

int Dynamic_HuntMode(){
    return Dynamic_HuntMode;
}

void SetSafe(bool a){
    Zone_Safe = a;
}

bool CheckSafe(){
    return Zone_Safe;
}

bool CheckZone(){
    return Dynamic_InZone;
}

};

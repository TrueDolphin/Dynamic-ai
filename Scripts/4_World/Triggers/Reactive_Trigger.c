class Reactive_Trigger: CylinderTrigger {
  eAIBase c_ai;
  PlayerBase c_player;
  int c_TailDistance = 80;
  int c_Cooldown = 5000;
  bool c_allow = true;
  bool c_running = false;

  void Dynamic_SetData(eAIBase a, PlayerBase b, int c, int d) {
    c_ai = a;
    c_player = b;
    c_TailDistance = c;
    c_Cooldown = d;
  }

  override void Enter(TriggerInsider insider) {
    super.Enter(insider);
    PlayerBase player = PlayerBase.Cast(insider.GetObject());
  }

  override void Leave(TriggerInsider insider) {
    super.Leave(insider);
    PlayerBase player = PlayerBase.Cast(insider.GetObject());

    if (!c_ai || !c_player) {
      if (this) Delete();
      return;
    }
    if (player == c_player) {
      if (c_allow) {
        eAIGroup AiGroup = eAIGroup.Cast(c_ai.GetGroup());
        if (!AiGroup) AiGroup = eAIGroup.GetGroupByLeader(c_ai);
        c_ai.GetGroup().AddWaypoint(ExpansionMath.GetRandomPointInRing(player.GetPosition(), c_TailDistance, c_TailDistance + 20));
        c_allow = false;
      } else {
        if (c_running) return;
        c_running = true;
        thread AllowTail(this);
      }
    }
  }

  void AllowTail(Reactive_Trigger trigger) {
    Sleep(c_Cooldown);
    if (!trigger) return;
    c_allow = true;
    c_running = false;
  }

  override protected bool CanAddObjectAsInsider(Object object) {

    if (!super.CanAddObjectAsInsider(object)) {
      return false;
    }
    return PlayerBase.Cast(object) != null;
  }
}